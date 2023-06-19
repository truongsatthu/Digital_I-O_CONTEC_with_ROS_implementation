#===================================================================
# API-DIO(LNX) USB Device Driver Makefile
# Copyright(C) 2010 CONTEC CO.,LTD.
#===================================================================

#------------------------------------------------
#   kernel 2.6 or later
#------------------------------------------------

OBJS = Cusb_module.o Cusb_firm.o
TARGET = cdiousb.ko

all	:	$(TARGET)

$(TARGET):  Cusb_module.c Cusb_firm.c
	rm -fr `uname -r`
	mkdir `uname -r`
	make -C /lib/modules/`uname -r`/build M=`pwd` V=1 modules
	cp $(TARGET) `uname -r`/

clean :
	make -C /lib/modules/`uname -r`/build M=`pwd` V=1 clean
	rm -fr `uname -r`

obj-m := cdiousb.o

cdiousb-objs := $(OBJS)

clean-files := *.o *.ko *.mod.[co] *~ .*.cmd

install:
	cp `uname -r`/cdiousb.ko ../config
	cp `uname -r`/cdiousb.ko /lib/modules/`uname -r`/kernel/drivers/usb
	depmod
