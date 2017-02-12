obj-m += pass_data_test.o

CROSS_COMPILE = arm-linux-gnueabihf-
ARCH = arm
KERNELDIR = ~/Desktop/kernel_source_full

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=`pwd` modules

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=`pwd` clean
