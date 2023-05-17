#LINUX_ROOT := /home/cwj/hisi3519/linux-3.18.y_uac
LINUX_ROOT := /home/cwj/data/ssd268g_super_sound/kernel


ifneq ($(KERNELRELEASE),)
obj-m:=hello.o
else
PWD :=$(shell pwd)

modules:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-sigmastar-9.1.0- -C $(LINUX_ROOT) M=$(PWD) modules

endif
.PHONY:clean
clean:
	rm -rf *.o *~.depend.*.
