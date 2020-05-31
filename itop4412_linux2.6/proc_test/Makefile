ifneq ($(KERNELRELEASE),)
CONFIG_USB_G_WEBCAM=m
obj-$(CONFIG_USB_G_WEBCAM)  += my_proc.o 
g_my_proc-y           := my_proc.o 
else
#LINUX_ROOT:=/home/cwj/Hi3516A_SDK_V1.0.6.0/osdrv/opensource/kernel/linux-3.4.y
LINUX_ROOT:=/home/topeet/zImage/iTop4412_Kernel_3.0
PWD ?=$(shell pwd)
all:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -C $(LINUX_ROOT) M=$(PWD) modules
clean:
	@rm -rf *.o *.ko
	@rm -f *.o modules.* *.symvers *.mod.c
endif





