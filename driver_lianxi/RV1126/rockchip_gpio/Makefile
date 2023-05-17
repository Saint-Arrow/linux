DEBUG_DIR=~/nfs

#LINUX_ROOT := /home/cwj/data/rv1126_rv1109_linux_V1.5.0_20201107/kernel
#CROSS_COMPILE=arm-linux-gnueabihf-gcc-8.3.0

LINUX_ROOT := /home/cwj/data_ext/temp/hi3516ev300_kernel_1.3.0/kernel/linux-4.9.y
CROSS_COMPILE = arm-himix100-linux-

PRODUCT=V40UB



obj-m := board_ctrl.o



default:	
	@$(MAKE) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) -C $(LINUX_ROOT) M=$(PWD) modules 
	-cp *.ko  $(DEBUG_DIR) 
	@rm -f *.o modules.* *.symvers *.mod.c

clean:
	@make -C $(LINUX_ROOT) M=$(PWD) clean 
	@rm -f nfc_test *.bak *.o 

