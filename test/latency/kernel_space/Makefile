obj-m := kernel_latency.o

CROSS_COMPILE = arm-linux-gnueabihf-
ARCH = arm
KERNELDIR = /home/hua-arch/Development/xenomai_kernel_source/linux

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=`pwd` modules

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=`pwd` clean
