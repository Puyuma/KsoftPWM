obj-m += gpio_access.o

CROSS_COMPILE = arm-linux-gnueabihf-
ARCH = arm
KERNELDIR = ~/Desktop/kernel_source_full #/lib/modules/$(shell uname -r)/build/

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=`pwd` modules

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=`pwd` clean
