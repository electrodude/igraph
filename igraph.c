/*
 *  igraph.c
 *  implicit equation grapher
 *
 *  Created by Albert Emanuel on 11/25/14.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>

#include <math.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "igraph.h"

GLFWwindow* gr_window;

#define M_PI 3.1415926535897932384626

#define FPART(x) (fabs((x) - (long)((x)+0.5)))

#if 0
#define FUNC(x,y) (x*y*(1-y))

#define XMIN 2.99
#define XMAX 4
#define YMIN 0
#define YMAX 1

#endif

#if 0
#define FUNC(x,y) (x*y*(1-y))

#define XMIN 0.9
#define XMAX 3.9
#define YMIN 0
#define YMAX 1

#endif

#if 0
//#define FUNC(x,y) (sin(x*y) - x*x - y*y + 1)
//#define FUNC(x,y) (x*x+y*y+sin(8*x)+sin(8*y)-2)
//#define FUNC(x,y) (cos(x*x+y*y)/(pow(x*x-y*x - sin(x),1.0/3.0)) - 1)
//#define FUNC(x,y) ((x*x*x*y + y*y*y*x - 4*x*y - x - y - 0.2) * (sqrt(x*x + y*y*x) - 2) - 0.1)
//#define FUNC(x,y) ((x*y - x + y + sin(x*y*4)) * (x*x - y*y) - 2)
//#define FUNC(x,y) ((log(abs(- x*x + y + pow(x + y, exp(x*sin(x*y)*sin(x*y)))))/log(abs(x*y)) * pow(y, 1/3) + log(abs(pow(x, y)))/log(y*y)- 2) * (sin(x) - y))

//#define FUNC(x,y) ()

#define _T 300
#define _n 1
#define _a 5.46
#define _b 0.0305
#define _R 0.08206


#define FUNC(V,P) (((P+(_n*_n*_a/(V*V)))*(V-(_n*_b))-(_n*_R*_T))*V*P*(sin(V*M_PI)+((P > 0.0 && P < 0.5) ? 0.0 : 2.0))*(sin(P*M_PI)+((V > 0.0 && V < 0.5) ? 0.0 : 2.0)))
//#define FUNC(V,P) (V*P*(sin(P*M_PI) + ((V > 0.0 && V < 1.0) ? 0.0 : 1.1)))

#define XMIN -2
#define XMAX 5
#define YMIN -30
#define YMAX 40

#endif

#if 0
//#define FUNC(x,y) (pow(x, sqrt(y)) - 4/(x-2))
//#define FUNC(x,y) (sin(M_PI*x)+sin(M_PI*y)+sin(8*x)+sin(8*y)-1)
//#define FUNC(x,y) ((13-x)*(12-y)*(11-x)*(10-y)*(9-x)*(8-y)*(7-x)*(6-y)*(5-x)*(4-y)*(3-x)*(2-y)*(1-x) - 5)
//#define FUNC(x,y) (pow(y, x/2) - pow(3, (7 - pow(x,cos(y)))))
#define FUNC(x,y) (x*x+y*y+sin(8*x)+sin(8*y)-1)

#define XMIN -16
#define XMAX 16
#define YMIN -9
#define YMAX 9
#endif

#if 1
#define FUNC(x,y) ((x*x) - (-2+y+5*cos(y)))

#define XMIN -32
#define XMAX 32
#define YMIN -12
#define YMAX 24
#endif


int width = 1280;
int height = 740;

int detail = 1;

double xmin = XMIN;
double xmax = XMAX;
double ymin = YMIN;
double ymax = YMAX;


int width2;
int height2;

double dx;
double dy;
double dx2;
double dy2;


int stride = 1;

int maxerror = 5e2;
int minerror = 1e-10;


int num_threads;

int changed;

char* screen;

int* rows;
int* cols;

//int buf[width][height];

pthread_t* threads;
//pthread_mutex_t bufmutex;

void setpixel(int x, int y, int r, int g, int b)
{
	if (x<0 || x>=width || y<0 || y>=height)
	{
		return;
	}
	screen[(y*width+x)*3+0] |= r;
	screen[(y*width+x)*3+1] |= g;
	screen[(y*width+x)*3+2] |= b;
}

void setpixeld(double x, double y, int r, int g, int b)
{
	int xi = (int)((x-xmin)/dx+0.5); 
	int yi = (int)((y-ymin)/dy+0.5);
	setpixel(xi, yi, r, g, b);
}

static inline void addpixel(int x, int y, int rd, int gd, int bd)
{
	int r, g, b;

	//getpixel(x, y, &r, &g, &b);

	r = 0;
	g = 0;
	b = 0;

	r += rd;
	g += gd;
	b += bd;

	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;

	setpixel(x, y, r, g, b);
}


void binsearch_x(double leftx, double rightx, double y)
{
	double midx = (leftx + rightx)/2.0;

	int lefti = (int)((leftx-xmin)/dx+0.5);
	int righti = (int)((rightx-xmin)/dx+0.5);

	double err = fabs(FUNC(leftx, y) - FUNC(rightx, y));

	if (lefti == righti || err <= minerror)
	{
		if (err <= maxerror)
		{
			setpixeld(midx, y, 255, 255, 255);
		}
		return;
	}

	if ((FUNC(leftx, y) > 0) != (FUNC(midx, y) >= 0))
	{
		binsearch_x(leftx, midx, y);
	}

	if ((FUNC(midx, y) > 0) != (FUNC(rightx, y) >= 0))
	{
		binsearch_x(midx, rightx, y);
	}
}

void binsearch_y(double x, double lefty, double righty)
{
	double midy = (lefty + righty)/2.0;

	int lefti = (int)((lefty-ymin)/dy+0.5);
	int righti = (int)((righty-ymin)/dy+0.5);

	double err = fabs(FUNC(x, lefty) - FUNC(x, righty));

	if (lefti == righti || err <= minerror)
	{
		if (err <= maxerror)
		{
			setpixeld(x, midy, 255, 255, 255);
		}
		return;
	}

	if ((FUNC(x, lefty) > 0) != (FUNC(x, midy) >= 0))
	{
		binsearch_y(x, lefty, midy);
	}

	if ((FUNC(x, midy) > 0) != (FUNC(x, righty) >= 0))
	{
		binsearch_y(x, midy, righty);
	}
}

void* calc(void* threadid)
{
	int id = *(int*)threadid;
	free(threadid);

	char name[20];
	snprintf(name, 20, "graph%d", id);
	prctl(PR_SET_NAME, name);

	int x;
	int y;

	printf("%d/%d: starting\n", id, num_threads);

	if ((id & 1) == 0 || num_threads == 1)
	{
		printf("%d: doing cols\n", id);
		for (x=id/2; x<width2; x+=(num_threads+1)/2)
		{
			if (!cols[x])
			{
				cols[x] = 1;

				double xd = ((double)x)*dx2 + xmin;

				printf("%d: x=%d, %f done\n", id, x, (double)x/width2);

				for (y=0; y<height2; y+=stride)
				{
					double yd = ((double)y)*dy2 + ymin;
					//setpixeld(xd, yd, 255, 0, 0);
					double lefty = yd;
					double righty = yd + dy*stride;
					if ((FUNC(xd, lefty) > 0) != (FUNC(xd, righty) >= 0))
					{
						binsearch_y(xd, lefty, righty);
					}
				}

				cols[x] = 2;

				changed=1;
			}
		}
	}
	
	if ((id & 1) == 1 || num_threads == 1)
	{
		printf("%d: doing rows\n", id);
		for (y=id/2; y<height2; y+=(num_threads+2)/2-1)
		{
			if (!rows[y])
			{
				rows[y] = 1;

				double yd = ((double)y)*dy2 + ymin;

				printf("%d: y=%d, %f done\n", id, y, (double)y/height2);

				for (x=0; x<width2; x+=stride)
				{
					double xd = ((double)x)*dx2 + xmin;
					//setpixeld(xd, yd, 255, 0, 0);
					double leftx = xd;
					double rightx = xd + dx*stride;
					if ((FUNC(leftx, yd) > 0) != (FUNC(rightx, yd) >= 0))
					{
						binsearch_x(leftx, rightx, yd);
					}
				}

				rows[y] = 2;

				changed=1;
			}
		}
	}

	printf("%d: quitting\n", id);
	pthread_exit(NULL);
}

void gr_error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

void viewupdate()
{
	width2 = width*detail;
	height2 = height*detail;

	dx = (xmax-xmin)/(width-1);
	dy = (ymax-ymin)/(height-1);
	dx2 = (xmax-xmin)/(width2-1);
	dy2 = (ymax-ymin)/(height2-1);
}

int main(int argc, char** argv)
{

	int cores = sysconf(_SC_NPROCESSORS_ONLN);

	num_threads = 2;

	printf("%d cores, %d threads\n", cores, num_threads);

	threads = malloc(num_threads*sizeof(pthread_t));

	glfwSetErrorCallback(gr_error_callback);

	if (!glfwInit())
	{
		glfwTerminate();
		puts("glfwInit failed");
		return -1;
	}

	gr_window = glfwCreateWindow(width, height, "Graph", NULL, NULL);

	if (!gr_window)
	{
		puts("glfwCreateWindow failed");
		return -1;
	}

	glfwMakeContextCurrent(gr_window);

	glScaled(1/detail, 1/detail, 1/detail);
	

	viewupdate();

	screen = calloc(width*height*3, sizeof(char));	// 3 because r,g,b

	rows = calloc(height2, sizeof(int));
	cols = calloc(width2, sizeof(int));

	for (double xf = ((long)xmin); xf < ((long)xmax) + 1; xf += 1)
	{
		int x = (int)((xf-xmin)/dx);
		for (int y = 0; y < height; y++)
		{
			//int s = (fabs(xf - (long)(xf+0.5)) <= 0.05) ? 128 : 64;
			int s = (FPART(fabs(xf)/10) <= 0.05) ? 127 : 64;

			int r = (fabs(xf) <= 0.05) ? 255 : s;
			int g = (fabs(xf) <= 0.05) ?   0 : s;
			int b = (fabs(xf) <= 0.05) ?   0 : s;
			addpixel(x, y, r, g, b);
		}
	}

	for (double yf = ((long)ymin); yf < ((long)ymax) + 1; yf += 1)
	{
		int y = (int)((yf-ymin)/dy);
		for (int x = 0; x < width; x++)
		{
			//int s = (fabs(yf - (long)(yf+0.5)) <= 0.05) ? 128 : 64;
			int s = (FPART(fabs(yf)/10) <= 0.05) ? 127 : 64;

			int r = (fabs(yf) <= 0.05) ?   0 : s;
			int g = (fabs(yf) <= 0.05) ? 255 : s;
			int b = (fabs(yf) <= 0.05) ?   0 : s;
			addpixel(x, y, r, g, b);
		}
	}


	//pthread_mutex_init(&bufmutex, NULL);
	
	pthread_attr_t threadattr;
	pthread_attr_init(&threadattr);
	//pthread_attr_setstacksize(&threadattr, 2097152);
	
	int i;
	for (i=0; i<num_threads; i++)
	{
		printf("In main: creating thread %d\n", i);
		int* threadid = malloc(sizeof(int));
		*threadid = i;
		int rc = pthread_create(&threads[i], NULL, calc, threadid);
		if (rc)
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	
	while (!glfwWindowShouldClose(gr_window))
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		//setpixel(width/2, height/2, 255, 255, 255);
		//glutSwapBuffers();
		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, screen);
		
		glfwSwapBuffers(gr_window);

		glfwPollEvents();

		usleep(100000);
	}

	glfwDestroyWindow(gr_window);

	return EXIT_SUCCESS;
}
