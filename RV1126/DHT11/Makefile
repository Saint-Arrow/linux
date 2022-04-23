LINUX_ROOT := /home/cwj/data/rv1126_rv1109_linux_V1.5.0_20201107/kernel
DEBUG_DIR=~/nfs
CC=arm-linux-gnueabihf-gcc-8.3.0

PRODUCT=V40UB



obj-m := dht11.o



default:	
	@$(MAKE) ARCH=arm  -C $(LINUX_ROOT) M=$(PWD) modules 
	-cp *.ko  $(DEBUG_DIR) 
	@rm -f *.o modules.* *.symvers *.mod.c

clean:
	@make -C $(LINUX_ROOT) M=$(PWD) clean 
	@rm -f nfc_test *.bak *.o 

