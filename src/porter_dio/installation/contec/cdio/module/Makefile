#===================================================================
# API-DIO(LNX) Device Driver Makefile
# Copyright(C) 2002 CONTEC CO.,LTD.
#===================================================================

TARGET = cdio.ko

all	:	$(TARGET)

$(TARGET): Ccom_module.c Cdio_dispatch.c Cdio_dm_dispatch.c Cdio_bios.c BusMaster.c Cdio_user_irq.c
	make -C /lib/modules/`uname -r`/build M=`pwd` V=1 modules

obj-m :=  cdio.o

cdio-objs := Ccom_module.o Cdio_dispatch.o Cdio_dm_dispatch.o Cdio_bios.o BusMaster.o Cdio_user_irq.o

install:
	cp $(TARGET) ../config

clean :
	rm -f *.o *.ko *.mod.[co] *~ .*.cmd *.symvers *.ko.unsigned *.order

