/*
 *  main.h
 *  implicit equation grapher
 *
 *  Created by Albert Emanuel on 11/25/14.
 *
 */

void setpixel(int x, int y, int r, int g, int b);

void inline incpix(int x, int y);

void display(void);

void reshape(int width, int height);

void idle(void);

int main(int argc, char** argv);
