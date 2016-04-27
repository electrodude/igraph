CFLAGS=-std=c99 -O3 -Wextra -I/usr/local/include/ -I/usr/include/
LDFLAGS=-L/usr/local/lib/ -L/usr/lib/ -L/usr/lib/mesa/ -lglfw -lX11 -lXrandr -lXi -lXxf86vm -lm -lpthread -lGL -lGLU
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
