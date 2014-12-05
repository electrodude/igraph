/*
 *  main.h
 *  implicit equation grapher
 *
 *  Created by Albert Emanuel on 11/25/14.
 *
 */

void setpixel(int x, int y, int r, int g, int b);

void setpixeld(double x, double y, int r, int g, int b);

void binsearch_x(double leftx, double rightx, double y);

void binsearch_y(double x, double lefty, double righty);

void* calc(void* threadid);

void gr_error_callback(int error, const char* description);

int main(int argc, char** argv);
