obj-m += tasker.o

tasker-objs := mytasker.o tracepoints.o ghosttask.o

export KROOT=/lib/modules/6.19.14+kali-amd64/build

all:
	@$(MAKE) -C $(KROOT) M=$(shell pwd) modules

clean: kernel_clean
	rm -rf  tracepoints  Module.symvers modules.order
