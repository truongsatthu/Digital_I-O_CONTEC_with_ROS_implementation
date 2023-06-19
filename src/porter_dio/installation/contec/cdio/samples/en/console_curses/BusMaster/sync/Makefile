#
INCLUDE_DIR = /usr/src/linux/include
CC = gcc
CFLAGS = -Wall

OBJS = sync.o

all:$(OBJS)
	gcc ../common/TermLib.c $^ -o sync -lcdio -lcurses

clean:
	rm -f *.o
	
