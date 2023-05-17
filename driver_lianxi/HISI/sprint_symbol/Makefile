NAME = sprint_symbol
obj-m += $(NAME).o 
CURRENT_PATH := $(shell pwd)

#LINUX_KERNEL_PATH := /data/lym/Hi3516EV200_SDK_V1.0.1.3/osdrv/opensource/kernel/linux-4.9.y
#LINUX_KERNEL_PATH := /data/lxc/V10C/linux-4.9.y

LINUX_KERNEL_PATH := /home/cwj/data_ext/hi3516ev300_kernel_1.3.0/kernel/linux-4.9.y
CROSS_COMPILE=arm-himix100-linux-

#LINUX_KERNEL_PATH := /data_ext/cwj/GK7205V300/GKIPCLinuxV100R001C00SPC020/out/gk7205v300/linux-4.9.y


all:
	touch $(NAME).c 
	echo "#include \"main.c\"" > $(NAME).c
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH)  modules  ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE)
	cp $(NAME).ko ~/nfs/
clean:
	rm -rf $(NAME).c
	rm -rf *.o *.ko .*.cmd *.mod.* modules.order Module.symvers .tmp_versions
