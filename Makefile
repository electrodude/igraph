CFLAGS=-std=c99 -O2 -g -I/usr/local/include/ -I/usr/include/
CXXFLAGS=-std=c++11 -O2 -g -fpermissive
LDFLAGS=-L/usr/local/lib/ -L/usr/lib/ -L/usr/lib/mesa/ -lglfw3 -lX11 -lXrandr -lXi -lXxf86vm -lm -lpthread -lGL -lGLU
CC=gcc
CXX=g++
LD=gcc

all:		main

clean:		
		rm -f main main.o

main:		main.o
		${LD} -o $@ $^ ${LDFLAGS} 

%.o:		%.c %.h
		${CC} ${CFLAGS} -c -o $@ $<

%.o:		%.cpp %.hpp
		${CXX} ${CXXFLAGS} -c -o $@ $<
		