all:
	$(MAKE) -C module_usb
	$(MAKE) -C module
	$(MAKE) -C config 
	$(MAKE) -C cgetres_usb

install:
	$(MAKE) -C module_usb install
	$(MAKE) -C module install
	$(MAKE) -C library install
	$(MAKE) -C config install

clean:
	$(MAKE) -C module_usb clean
	$(MAKE) -C module clean
	$(MAKE) -C cgetres_usb clean
	$(MAKE) -C library clean
	$(MAKE) -C config clean
