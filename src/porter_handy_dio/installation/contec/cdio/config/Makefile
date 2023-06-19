#===================================================================
# API-DIO(LNX) Configuration Makefile
# Copyright(C) 2002 CONTEC CO.,LTD.
#===================================================================

I386_DIR = i386
AMD64_DIR = amd64
OBJS = cgetres.o
TARGET = cgetres.ko

all :   $(TARGET)

$(TARGET):  cgetres.c
	rm -fr `uname -r`
	mkdir `uname -r`
	make -C /lib/modules/`uname -r`/build M=`pwd` V=1 modules
	mv $(TARGET) `uname -r`

clean :
	make -C /lib/modules/`uname -r`/build M=`pwd` V=1 clean
	rm -fr `uname -r`

obj-m :=  cgetres.o

clean-files := *.o *.ko *.mod.[co] *~ .*.cmd

install:
ifeq ($(shell arch),x86_64)
	cp $(AMD64_DIR)/config ./
else
	cp $(I386_DIR)/config ./
endif

