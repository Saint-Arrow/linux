NAME = dts_test
obj-m += dts_test.o
CURRENT_PATH := $(shell pwd)

#LINUX_KERNEL_PATH := /data/lym/Hi3516EV200_SDK_V1.0.1.3/osdrv/opensource/kernel/linux-4.9.y
#LINUX_KERNEL_PATH := /data/lxc/V10C/linux-4.9.y
LINUX_KERNEL_PATH := /home/cwj/data_ext/temp/hi3516ev300_kernel_1.3.0/kernel/linux-4.9.y

EXTRA_CFLAGS += -DUSE_LINUX_GPIOLIB
#EXTRA_CFLAGS += -DUSE_LINUX_SPI
all:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH)  modules  ARCH=arm CROSS_COMPILE=arm-himix100-linux-
	rm -rf *.o  .*.cmd *.mod.* modules.order Module.symvers .tmp_versions
	cp $(NAME).ko ~/nfs/
clean:
	    rm -rf *.o *.ko .*.cmd *.mod.* modules.order Module.symvers .tmp_versions
