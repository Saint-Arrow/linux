ifneq ($(KERNELRELEASE),)
CONFIG_USB_FILE_STORAGE=m
obj-$(CONFIG_USB_FILE_STORAGE)  += keyirq.o  
else
#LINUX_ROOT:=/home/cwj/Hi3516A_SDK_V1.0.6.0/osdrv/opensource/kernel/linux-3.4.y
LINUX_ROOT:=/home/topeet/zImage/iTop4412_Kernel_3.0
PWD ?=$(shell pwd)
all:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -C $(LINUX_ROOT) M=$(PWD) modules
	#cp ./*.ko ../../home_link/

clean:
	@rm -rf *.o *.ko 
	@rm -rf  .tmp_versions
	@rm -f *.o modules.* *.symvers *.mod.c
	find . -name "*.cmd"|xargs -i rm -rf {}
endif
