#===================================================================
# API-DIO(LNX) USB Device Driver Makefile
# Copyright(C) 2017 CONTEC CO.,LTD.
#===================================================================

#------------------------------------------------
#   kernel 2.2 and 2.4
#------------------------------------------------
ifeq ($(shell uname -r | cut -c-3),($(filter $(shell uname -r | cut -c-3), 2.2 2.4)))

echo "This driver supports Kernel version 2.6 or later kernel version."

else
#------------------------------------------------
#   kernel 2.6 or later
#------------------------------------------------

OBJS = cgetres_usb.o
TARGET = cgetres_usb.ko

all :   $(TARGET)

$(TARGET):  cgetres_usb.c
	@if [ ! -d ../config/`uname -r` ]; \
		then mkdir ../config/`uname -r`; \
	fi
	@if [ -f ../config/`uname -r`/$(TARGET) ]; \
		then rm -f ../config/`uname -r`/$(TARGET); \
	fi
	make -C /lib/modules/`uname -r`/build M=`pwd` V=1 modules CFLAGS=MODULE_USB
	mv $(TARGET) ../config/`uname -r`

clean :
	make -C /lib/modules/`uname -r`/build M=`pwd` V=1 clean
	@if [ -f ../config/`uname -r`/$(TARGET) ]; \
		then rm -f ../config/`uname -r`/$(TARGET); \
	fi

obj-m :=  cgetres_usb.o

#cgetres-objs := cgetres_usb.o

clean-files := *.o *.ko *.mod.[co] *~ .*.cmd

endif
