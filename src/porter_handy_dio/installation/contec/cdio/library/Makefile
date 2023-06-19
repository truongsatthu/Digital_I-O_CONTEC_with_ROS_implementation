#	
#
CC=gcc

SOLIB_OBJ := cdio.o
SOLIB_SRC := libcdio.so.1.0
SOLIB_DES := /usr/lib

ifeq ($(shell uname -r | cut -c-3), $(filter $(shell uname -r | cut -c-3), 2.2 2.4))
	PLATFORM = i386
else
	PLATFORM = $(shell uname -i)
endif

ifeq ($(strip $(PLATFORM)),x86_64)
        SOLIB_SRC_DIR := amd64
else
        SOLIB_SRC_DIR := i386
endif

SOLIB_NAME := libcdio.so

install:	
		cp $(SOLIB_SRC_DIR)/$(SOLIB_SRC) $(SOLIB_DES)
		ldconfig
		ln -sf $(SOLIB_DES)/$(SOLIB_SRC) $(SOLIB_DES)/$(SOLIB_NAME)

clean:
		rm -f $(SOLIB_DES)/$(SOLIB_SRC)
		rm -f $(SOLIB_DES)/$(SOLIB_NAME)
