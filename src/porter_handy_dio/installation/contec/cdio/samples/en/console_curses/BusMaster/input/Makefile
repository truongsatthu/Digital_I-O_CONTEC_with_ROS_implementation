#
INCLUDE_DIR = /usr/src/linux/include
CC = gcc
CFLAGS = -Wall

OBJS = input.o

all:$(OBJS)
	gcc ../common/TermLib.c $^ -o input -lcdio -lcurses

clean:
	rm -f *.o
	
