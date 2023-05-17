ifneq ($(KERNELRELEASE),)
CONFIG_USB_FILE_STORAGE=m
obj-$(CONFIG_USB_FILE_STORAGE)  += key-input.o  
#这一行代表输出模块的名字
#g_file_storage-y           := file_storage.o  file2.o  
#这一行代表模块由多个文件组成
#obj-$(CONFIG_USB_GADGET)        += udc/ function/ legacy/   
#这一行代表进入子文件夹编译模块
else
#LINUX_ROOT:=/home/cwj/Hi3516A_SDK_V1.0.6.0/osdrv/opensource/kernel/linux-3.4.y
LINUX_ROOT:=/home/topeet/zImage/iTop4412_Kernel_3.0
PWD ?=$(shell pwd)
all:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -C $(LINUX_ROOT) M=$(PWD) modules
	cp ./*.ko ../../home_link/

clean:
	@rm -rf *.o 
	@rm -f *.o modules.* *.symvers *.mod.c
endif
