#
INCLUDE_DIR = /usr/src/linux/include
CC = gcc
CFLAGS = -Wall

OBJS = infinite.o

all:$(OBJS)
	gcc ../common/TermLib.c $^ -o infinite -lcdio -lcurses

clean:
	rm -f *.o
	
