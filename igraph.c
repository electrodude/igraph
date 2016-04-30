#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <errno.h>

#include <math.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "quadtree.h"

#include "func.h"


GLFWwindow* gr_window;

int width = 640;
int height = 400;


double zoomspeed = 1.1;
double gridcurr = 1.0;
double gridsize = 1.0;
int gridmul = 10;
int gridmul2;
int gridmul3;

double xmid;
double ymid;
double zoom;

int panning = 0;
double panx;
double pany;

volatile double xmin;
volatile double xmax;
volatile double ymin;
volatile double ymax;

volatile double dp;

double mindetail = 4.0;
double maxdetail = 1.0;


quadtree_node* rootnode;

pthread_t thread;


void quadtree_search(quadtree_node** nodeptr, double xl, double xh, double yl, double yh)
{
	//printf("quadtree_search %g %g %g %g\n", xh, xl, yh, yl);

	// If this node is entirely off the screen
	if (xh < xmin || xl > xmax || yh < ymin || yl > ymax)
	{
		// then return now
		//printf("quit\n");
		return;
	}

	//printf("%d %d %d %d\n", xlp, xhp, ylp, yhp);

	double bls = func(xl, yl) > 0;
	double brs = func(xh, yl) > 0;
	double tls = func(xl, yh) > 0;
	double trs = func(xh, yh) > 0;

	int on = bls != brs || tls != trs || bls != tls || brs != trs;

	if (on)
	{
		// If this node is smaller than maxdetail pixels
		if (xh - xl < dp*maxdetail && yh - yl < dp*maxdetail)
		{
			// then make a leaf node
			//printf("quadtree_search %g %g %g %g\n", xh, xl, yh, yl);
			quadtree_node* node = quadtree_node_get(nodeptr);
			node->r = 1.0;
			node->g = 1.0;
			node->b = 1.0;
			node->a = 1.0;

			*nodeptr = node;

			return;
		}
	}

	int recurse = 0;

	// If not enough detail yet, then recurse
	if (xh - xl > dp*mindetail || yh - yl > dp*mindetail)
	{
		recurse = 1;
	}

	// If graph intersects this node, then recurse
	if (on)
	{
		recurse = 1;
	}

	if (recurse)
	{
		double xm = 0.5*xh + 0.5*xl;
		double ym = 0.5*yh + 0.5*yl;

		// get node
		quadtree_node* node = quadtree_node_get(nodeptr);

		node->a /= 2.0; // show what it's working on

		// recursively search each child
		quadtree_search(&node->children[0], xl, xm, yl, ym);

		quadtree_search(&node->children[1], xm, xh, yl, ym);

		quadtree_search(&node->children[2], xl, xm, ym, yh);

		quadtree_search(&node->children[3], xm, xh, ym, yh);

		// put node in tree, update it, or delete it as appropriate
		quadtree_node_update(nodeptr, node);
	}

}

void* calc(void* param)
{
	prctl(PR_SET_NAME, "graph");

	while (1)
	{
		quadtree_search(&rootnode, -pow(2, 20), pow(2, 20), -pow(2, 20), pow(2, 20));
		printf("nodes: %ld\n", rootnode != NULL ? rootnode->totalchildren + 1 : 0);

		glfwPostEmptyEvent();
	}

	glfwPostEmptyEvent();

	pthread_exit(NULL);
}



void quadtree_render(const quadtree_node* node, double xl, double xh, double yl, double yh)
{
	//printf("quadtree_render %g %g %g %g\n", xh, xl, yh, yl);
	if (node == NULL) return;

	// If this node is entirely off the screen
	if (xh < xmin || xl > xmax || yh < ymin || yl > ymax)
	{
		// then return now
		return;
	}

	double xm = 0.5*xh + 0.5*xl;
	double ym = 0.5*yh + 0.5*yl;

#if 0
	glEnd();

	glColor4d(0.0, 0.0, 1.0, 0.5);

	glBegin(GL_LINES);

	glVertex2d(xl, ym);
	glVertex2d(xh, ym);

	glVertex2d(xm, yl);
	glVertex2d(xm, yh);

	glEnd();

	glBegin(GL_POINTS);
#endif

	// If the node fits entirely on the screen
	if (xl >= xmin && xh <= xmax && yl >= ymin && yh <= ymax)
	{
		// figure out where on the screen it is
		int xhp = (xh-xmin) / dp;
		int xlp = (xl-xmin) / dp;

		int yhp = (yh-ymin) / dp;
		int ylp = (yl-ymin) / dp;

		// If this node is entirely inside a single pixel, or smaller than one pixel
		if ((xhp == xlp && yhp == ylp) || (xh - xl <= dp && yh - yl <= dp))
		{
			// then we're done; draw this node
			glColor4d(node->r, node->g, node->b, node->a);
			glVertex2d(xm, ym);
			return;
		}
	}

	// If this node has any children
	if (node->totalchildren)
	{
		// then recurse for all of its children
		quadtree_node* bl = node->children[0];
		quadtree_render(bl, xl, xm, yl, ym);

		quadtree_node* br = node->children[1];
		quadtree_render(br, xm, xh, yl, ym);

		quadtree_node* tl = node->children[2];
		quadtree_render(tl, xl, xm, ym, yh);

		quadtree_node* tr = node->children[3];
		quadtree_render(tr, xm, xh, ym, yh);
	}
	else
	{
		// otherwise, just draw this node
		glEnd();

		glColor4d(node->r, node->g, node->b, node->a);

		glBegin(GL_QUADS);

		glVertex2d(xl, yl);
		glVertex2d(xh, yl);
		glVertex2d(xh, yh);
		glVertex2d(xl, yh);

		glEnd();

		glBegin(GL_POINTS);

		glVertex2d(xm, ym);
	}
}



void setview(double _xmid, double _ymid, double _zoom)
{
	//printf("setview(%g, %g, %g)\n", _xmid, _ymid, _zoom);

	xmid = _xmid;
	ymid = _ymid;
	zoom = _zoom;

	gridmul2 = gridmul  * gridmul;
	gridmul3 = gridmul2 * gridmul;

	double halfheight = height * 0.5;
	double halfwidth  = width  * 0.5;

	dp = zoom;

	xmin = xmid - halfwidth *dp;
	xmax = xmid + halfwidth *dp;

	ymin = ymid - halfheight*dp;
	ymax = ymid + halfheight*dp;

	//printf("dp = %g\n", dp);

	// adjust gridlines to appropriate size
	if (gridcurr/zoom > gridsize * gridmul)
	{
		gridcurr /= gridmul;
	}
	else if (gridcurr/zoom <= gridsize)
	{
		gridcurr *= gridmul;
	}

	//printf("gridcurr: %f\n", gridcurr);
}


static void gr_error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static void gr_on_mousebutton(GLFWwindow* window, int button, int action, int mods)
{
	//printf("mousebutton %d %d %d\n", button, action, mods);

	//if (button == 1)
	if (1) // any button pans now, this might change to only RMB or MMB
	{
		if (action == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			// set pan origin
			panx = xmid + xpos*dp;
			pany = ymid - ypos*dp;

			panning = 1;
		}
		else
		{
			panning = 0;
		}
	}

}

static void gr_on_mousemove(GLFWwindow* window, double xpos, double ypos)
{
	//printf("mouse %f %f\n", xpos, ypos);

	if (panning)
	{
		setview(-xpos*dp + panx, ypos*dp + pany, zoom);
	}
}

static void gr_on_mousewheel(GLFWwindow* window, double xoffset, double yoffset)
{
	//printf("mousescroll %f %f\n", xoffset, yoffset);

	double a = (yoffset < 0) ? zoomspeed : 1.0/zoomspeed;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// If we're panning
	if (panning)
	{
		// then update pan origin
		panx = xmid + xpos*dp;
		pany = ymid - ypos*dp;
	}

	// Calculate view position relative to cursor in world coordinates
	double xposc = xpos*dp + xmin;
	double yposc = ymax - ypos*dp;

	// Zoom relative to cursor
	setview(a*(xmid - xposc) + xposc, a*(ymid - yposc) + yposc, zoom*a);
}

static void gr_on_resize(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;

	//printf("resize %d %d\n", width, height);

	glViewport(0, 0, width, height);

	setview(xmid, ymid, zoom);
}


int main(int argc, char** argv)
{
	glfwSetErrorCallback(gr_error_callback);

	if (!glfwInit())
	{
		glfwTerminate();
		puts("glfwInit failed");
		return -1;
	}

	gr_window = glfwCreateWindow(width, height, "igraph", NULL, NULL);

	if (!gr_window)
	{
		puts("glfwCreateWindow failed");
		return -1;
	}

	glfwMakeContextCurrent(gr_window);

	glfwSetMouseButtonCallback(gr_window, gr_on_mousebutton);

	glfwSetCursorPosCallback(gr_window, gr_on_mousemove);

	glfwSetScrollCallback(gr_window, gr_on_mousewheel);

	glfwSetFramebufferSizeCallback(gr_window, gr_on_resize);


	GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

	glfwSetCursor(gr_window, cursor);


	glfwGetFramebufferSize(gr_window, &width, &height);

	glViewport(0, 0, width, height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	setview(0.0, 0.0, 0.05);

	rootnode = NULL;



	pthread_attr_t threadattr;
	pthread_attr_init(&threadattr);

	int en = pthread_create(&thread, NULL, calc, NULL);
	if (en)
	{
		errno = en;
		perror("pthread_create");
		exit(-1);
	}

	while (!glfwWindowShouldClose(gr_window))
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		double aspect = width/(double)height;

		double xscl = 1;
		double yscl = 1;

		if (aspect > 1)
		{
			xscl *= aspect;
		}
		else if (aspect < 1)
		{
			yscl /= aspect;
		}

		glOrtho(xmin, xmax, ymin, ymax, -1.0, 1.0);

		glClear(GL_COLOR_BUFFER_BIT);

		//glScaled(1.0/width, 1.0/height, 1.0);

		//glScaled(dp, dp, 1.0);

		//glTranslated(-xmin, -ymin, 0.0);

		double gridstress = (gridcurr / zoom / gridsize - 1) / (gridmul - 1) * 3 + 1;

		//printf("gridstress = %g\n", gridstress);


		glBegin(GL_LINES);

		for (int x = xmin/gridcurr; x < xmax/gridcurr + 1; x++)
		{
			double xf = x*gridcurr;

			double s = (x % gridmul3 == 0) ? 1.0           :
			           (x % gridmul2 == 0) ? 1/3.0         :
			           (x % gridmul  == 0) ? 1/3.0/3.0     :
			                                 1/3.0/3.0/3.0 ;

			double r = 1.0;
			double g = 1.0;
			double b = 1.0;
			double a = 0.5 * s * gridstress;

			glColor4d(r, g, b, a);

			glVertex2d(xf, ymin);
			glVertex2d(xf, ymax);
		}

		for (int y = ymin/gridcurr; y < ymax/gridcurr + 1; y++)
		{
			double yf = y*gridcurr;

			double s = (y % gridmul3 == 0) ? 1.0           :
			           (y % gridmul2 == 0) ? 1/3.0         :
			           (y % gridmul  == 0) ? 1/3.0/3.0     :
			                                 1/3.0/3.0/3.0 ;

			double r = 1.0;
			double g = 1.0;
			double b = 1.0;
			double a = 0.5 * s * gridstress;

			glColor4d(r, g, b, a);

			glVertex2d(xmin, yf);
			glVertex2d(xmax, yf);
		}

		glColor4ub(255,   0,   0, 255);

		glVertex2d(0.0, ymin);
		glVertex2d(0.0, ymax);

		glColor4ub(  0, 255,   0, 255);

		glVertex2d(xmin, 0.0);
		glVertex2d(xmax, 0.0);

		glEnd();

		glBegin(GL_POINTS);

		quadtree_render(rootnode, -pow(2, 20), pow(2, 20), -pow(2, 20), pow(2, 20));

		glEnd();

		int err = glGetError();

		if (err)
		{
			printf("GL error: %d\n", err);
		}

		glfwSwapBuffers(gr_window);

		glfwWaitEvents();
	}

	glfwDestroyWindow(gr_window);

	return EXIT_SUCCESS;
}
