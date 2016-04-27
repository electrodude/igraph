CFLAGS=-std=c99 -O3 -Wextra
#CFLAGS=-std=c99 -Og -g -Wextra
LDFLAGS=-L/usr/lib/mesa/ -lglfw -lX11 -lXrandr -lXi -lXxf86vm -lm -pthread -lGL -lGLU
CC=gcc
CXX=g++
LD=gcc

all:		igraph

clean:		
		rm -vf igraph *.o

igraph:		igraph.o
		${LD} -o $@ $^ ${LDFLAGS} 

%.o:		%.c %.h
		${CC} ${CFLAGS} -c -o $@ $<
