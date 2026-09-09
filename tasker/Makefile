### Automatic Makefile generation by 'genmake' script        ####
### Copyright, Jerry Cooperstein, coop@linuxfoundation.org 2003-2023 ####
### License: GPLv2 ###

obj-m +=  tasker.o

KDIR ?= /lib/modules/$(shell uname -r)/build

allofit:  modules
modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) modules
modules_install:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) modules_install
kernel_clean:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) clean

clean: kernel_clean
	rm -rf   Module.symvers modules.order
