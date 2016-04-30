CFLAGS=-O3
#CFLAGS=-Og -g
CFLAGS+=-std=c99 -Wextra -pthread
LDFLAGS=-lglfw -lX11 -lXrandr -lXi -lXxf86vm -lm -lGL -lGLU -pthread
CC=gcc
CXX=g++
LD=gcc

all:		igraph

clean:		
		rm -vf igraph *.o

igraph:		igraph.o func.o quadtree.o
		${LD} -o $@ $^ ${LDFLAGS} 

%.o:		%.c
		${CC} ${CFLAGS} -c -o $@ $<

depend:
		${CC} ${CCFLAGS} -MM igraph.c func.c quadtree.c

.PHONY:		all depend clean

# output of `make depend`

igraph.o: igraph.c quadtree.h func.h
func.o: func.c func.h
quadtree.o: quadtree.c quadtree.h
