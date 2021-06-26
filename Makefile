CC=g++
PKGS=sdl2
CFLAGS=-Wall -ggdb -pedantic `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

all: glitchy

glitchy: src/main.cpp
	$(CC) $(CFLAGS) -o glitchy src/main.cpp $(LIBS)
