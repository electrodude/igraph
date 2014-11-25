/*
 *  main.c
 *  implicit equation grapher
 *
 *  Created by Albert Emanuel on 11/25/14.
 *
 */

#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <math.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

 GLFWwindow* gr_window;
/*
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
*/

#define WIDTH 1280
#define HEIGHT 712

#define DETAIL 1

#define NUM_THREADS 2

//#define ANTIBUDDHABROT

//#define OVERFLOWCOLOR
//#define TRICOLOR

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
#define FUNC(x,y) (x*sin(y))

#define XMIN 2
#define XMAX 3.14169265358979
#define YMIN 0
#define YMAX 3.14169265358979

#endif

#if 1
//#define FUNC(x,y) (sin(x*y) - x*x - y*y + 1)
#define FUNC(x,y) (x*x+y*y+sin(8*x)+sin(8*y)-2)

#define XMIN -4
#define XMAX 4
#define YMIN -3
#define YMAX 3

#endif

#define MAXERROR 1e-10

#define STRIDE 1

#define WIDTH2 WIDTH*DETAIL
#define HEIGHT2 HEIGHT*DETAIL

#define DX (((double)(XMAX-XMIN))/(WIDTH-1))
#define DY (((double)(YMAX-YMIN))/(HEIGHT-1))
#define DX2 (((double)(XMAX-XMIN))/(WIDTH2-1))
#define DY2 (((double)(YMAX-YMIN))/(HEIGHT2-1))

char screen[WIDTH*HEIGHT*3];

int buf[WIDTH][HEIGHT];

//int cols[WIDTH];

pthread_t threads[NUM_THREADS];
pthread_mutex_t bufmutex;

void setpixel(int x, int y, int r, int g, int b)
{
	if (x<0 || x>=WIDTH || y<0 || y>=HEIGHT)
	{
		return;
	}
	screen[(y*WIDTH+x)*3+0] = r;
	screen[(y*WIDTH+x)*3+1] = g;
	screen[(y*WIDTH+x)*3+2] = b;
}

void setpixeld(double x, double y, int r, int g, int b)
{
	int xi = (int)((x-XMIN)/DX+0.5); 
	int yi = (int)((y-YMIN)/DY+0.5);
	setpixel(xi, yi, r, g, b);
}

void inline incpix(int x, int y)
{
	buf[x][y]++;
}

void binsearch_x(double leftx, double rightx, double y)
{
	double midx = (leftx + rightx)/2.0;

	if (fabs(leftx - rightx) <= MAXERROR)
	{
		setpixeld(midx, y, 255, 255, 255);
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

	if (fabs(FUNC(x, lefty) - FUNC(x, righty)) <= MAXERROR)
	{
		setpixeld(x, midy, 255, 255, 255);
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
	int id = (int)threadid;
	int x;
	int y;
	int i;
	double cy=0.00001;
	//double *pr = malloc(ITERATIONS*sizeof(double));
	//double *pi = malloc(ITERATIONS*sizeof(double));
	for (x=id; x<WIDTH2; x+=NUM_THREADS)
	{
		double xd = ((double)x)*DX2 +XMIN;

		printf("%d: x=%d/%f, %f done\n", id, x, (xd-XMIN)/DX2, (double)x/WIDTH2);
		//screen[(y/DETAIL*2*WIDTH)*3+id]=255;
		//printf("x=%d\n", x);

		for (y=0; y<HEIGHT2; y+=STRIDE)
		{
			double yd = ((double)y)*DY2 + YMIN;
			double lefty = yd;
			double righty = yd + DY*STRIDE;
			if ((FUNC(xd, lefty) > 0) != (FUNC(xd, righty) >= 0))
			{
				binsearch_y(xd, lefty, righty);
			}
		}
		
	}

	for (y=id; y<WIDTH2; y+=NUM_THREADS)
	{
		double yd = ((double)y)*DY2 +YMIN;

		printf("%d: y=%d/%f, %f done\n", id, y, (yd-YMIN)/DY2, (double)y/HEIGHT2);
		//screen[(y/DETAIL*2*WIDTH)*3+id]=255;
		//printf("x=%d\n", x);

		for (x=0; x<WIDTH2; x+=STRIDE)
		{
			double xd = ((double)x)*DX2 + XMIN;
			double leftx = xd;
			double rightx = xd + DX*STRIDE;
			if ((FUNC(leftx, yd) > 0) != (FUNC(rightx, yd) >= 0))
			{
				binsearch_x(leftx, rightx, yd);
			}
		}
		
	}
	printf("Thread %d quitting!\n", id);
	pthread_exit(NULL);
}

void gr_error_callback(int error, const char* description)
{
	fputs(description, stderr);
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


	gr_window = glfwCreateWindow(WIDTH, HEIGHT, "Fractal", NULL, NULL);

	if (!gr_window)
	{
		puts("glfwCreateWindow failed");
		return -1;
	}

	glfwMakeContextCurrent(gr_window);

	glScaled(1/DETAIL, 1/DETAIL, 1/DETAIL);
	
	pthread_mutex_init(&bufmutex, NULL);
	
	pthread_attr_t threadattr;
	pthread_attr_init(&threadattr);
	//pthread_attr_setstacksize(&threadattr, 2097152);
	
	int i;
	for (i=0; i<NUM_THREADS; i++)
	{
		printf("In main: creating thread %d\n", i);
		int rc = pthread_create(&threads[i], NULL, calc, (void *)i);
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
		
		//setpixel(WIDTH/2, HEIGHT/2, 255, 255, 255);
		//glutSwapBuffers();
		glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, &screen);
		
		glfwSwapBuffers(gr_window);

		glfwPollEvents();

		usleep(100000);
	}

	glfwDestroyWindow(gr_window);

	return EXIT_SUCCESS;
}
