#
INCLUDE_DIR = /usr/src/linux/include
CC = gcc
CFLAGS = -Wall

OBJS = output.o

all:$(OBJS)
	gcc ../common/TermLib.c $^ -o output -lcdio

clean:
	rm -f *.o
	
