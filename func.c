#include <math.h>
#include <complex.h>

#include "func.h"

#if 1

double func(double x, double y)
{
	return (x*x*sin(y+x)) - (-2*y*sin(x)+y+5*cos(x*y));
}

#endif


#if 0
// circle of radius 1

double func(double x, double y)
{
	return x*x + y*y - 1;
}
#endif


#if 0
// Mandelbrot set
// It's actually fairly efficient, since it uses a boundary trace

#define MAXITER 1000

double func(double x, double y)
{
	complex double c = x + y*I;

	complex double z = 0.0;

	for (unsigned int iter = 1; iter < MAXITER; iter++)
	{
		complex double z2 = z*z + c;
		if (z2 == z)
		{
			return -1;
		}

		z = z2;

		if (cabs(z) > 2.0)
		{
			return 1;
		}
	}

	return -1;
}
#endif
