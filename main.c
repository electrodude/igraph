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
#include <unistd.h>
#include <pthread.h>

#include <math.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

GLFWwindow* gr_window;

#define WIDTH 1280
#define HEIGHT 712

#define DETAIL 1

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
//#define FUNC(x,y) (cos(x*x+y*y)/(pow(x*x-y*x - sin(x),1.0/3.0)) - 1)
//#define FUNC(x,y) ((x*x*x*y + y*y*y*x - 4*x*y - x - y - 0.2) * (sqrt(x*x + y*y*x) - 2) - 0.1)
//#define FUNC(x,y) ((x*y - x + y + sin(x*y*4)) * (x*x - y*y) - 2)
//#define FUNC(x,y) ((log(abs(- x*x + y + pow(x + y, exp(x*sin(x*y)*sin(x*y)))))/log(abs(x*y)) * pow(y, 1/3) + log(abs(pow(x, y)))/log(y*y)- 2) * (sin(x) - y))

//#define FUNC(x,y) ()

#define FUNC(x,y) (2*y + 3*x + x*x + y*y - x*y - x*x*y + y*y*y - 3)

#define XMIN -8
#define XMAX 8
#define YMIN -6
#define YMAX 6

#endif

#define MAXERROR 1e-10
//#define MAXERROR ((DX > DY ? DY : DX)/2)

#define STRIDE 1

#define WIDTH2 WIDTH*DETAIL
#define HEIGHT2 HEIGHT*DETAIL

#define DX (((double)(XMAX-XMIN))/(WIDTH-1))
#define DY (((double)(YMAX-YMIN))/(HEIGHT-1))
#define DX2 (((double)(XMAX-XMIN))/(WIDTH2-1))
#define DY2 (((double)(YMAX-YMIN))/(HEIGHT2-1))

int num_threads;

int changed;

char screen[WIDTH*HEIGHT*3];

int buf[WIDTH][HEIGHT];

pthread_t* threads;
pthread_mutex_t bufmutex;

void setpixel(int x, int y, int r, int g, int b)
{
	if (x<0 || x>=WIDTH || y<0 || y>=HEIGHT)
	{
		return;
	}
	screen[(y*WIDTH+x)*3+0] |= r;
	screen[(y*WIDTH+x)*3+1] |= g;
	screen[(y*WIDTH+x)*3+2] |= b;
}

void setpixeld(double x, double y, int r, int g, int b)
{
	int xi = (int)((x-XMIN)/DX+0.5); 
	int yi = (int)((y-YMIN)/DY+0.5);
	setpixel(xi, yi, r, g, b);
}

void binsearch_x(double leftx, double rightx, double y)
{
	double midx = (leftx + rightx)/2.0;

	int lefti = (int)((leftx-XMIN)/DX+0.5);
	int righti = (int)((rightx-XMIN)/DX+0.5);

	if (lefti == righti || fabs(leftx - rightx) <= MAXERROR)
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

	int lefti = (int)((lefty-YMIN)/DY+0.5);
	int righti = (int)((righty-YMIN)/DY+0.5);

	if (lefti == righti || fabs(FUNC(x, lefty) - FUNC(x, righty)) <= MAXERROR)
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

	printf("%d/%d: starting\n", id, num_threads);

	if ((id & 1) == 0 || num_threads == 1)
	{
		printf("%d: doing cols\n", id);
		for (x=id/2; x<WIDTH2; x+=(num_threads+1)/2)
		{
			double xd = ((double)x)*DX2 +XMIN;

			printf("%d: x=%d, %f done\n", id, x, (double)x/WIDTH2);

			for (y=0; y<HEIGHT2; y+=STRIDE)
			{
				double yd = ((double)y)*DY2 + YMIN;
				//setpixeld(xd, yd, 255, 0, 0);
				double lefty = yd;
				double righty = yd + DY*STRIDE;
				if ((FUNC(xd, lefty) > 0) != (FUNC(xd, righty) >= 0))
				{
					binsearch_y(xd, lefty, righty);
				}
			}

			changed=1;
		}
	}
	
	if ((id & 1) == 1 || num_threads == 1)
	{
		printf("%d: doing rows\n", id);
		for (y=id/2; y<HEIGHT2; y+=(num_threads+2)/2-1)
		{
			double yd = ((double)y)*DY2 +YMIN;

			printf("%d: y=%d, %f done\n", id, y, (double)y/HEIGHT2);

			for (x=0; x<WIDTH2; x+=STRIDE)
			{
				double xd = ((double)x)*DX2 + XMIN;
				//setpixeld(xd, yd, 255, 0, 0);
				double leftx = xd;
				double rightx = xd + DX*STRIDE;
				if ((FUNC(leftx, yd) > 0) != (FUNC(rightx, yd) >= 0))
				{
					binsearch_x(leftx, rightx, yd);
				}
			}

			changed=1;
		}
	}
	printf("%d: quitting\n", id);
	pthread_exit(NULL);
}

void gr_error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

int main(int argc, char** argv)
{
	int cores = sysconf(_SC_NPROCESSORS_ONLN);

	num_threads = cores + 1;

	printf("%d cores, %d threads\n", cores, num_threads);

	threads = malloc(num_threads*sizeof(pthread_t));

	glfwSetErrorCallback(gr_error_callback);

	if (!glfwInit())
	{
		glfwTerminate();
		puts("glfwInit failed");
		return -1;
	}


	gr_window = glfwCreateWindow(WIDTH, HEIGHT, "Graph", NULL, NULL);

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
	for (i=0; i<num_threads; i++)
	{
		printf("In main: creating thread %d\n", i);
		int rc = pthread_create(&threads[i], NULL, calc, (void*)i);
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
