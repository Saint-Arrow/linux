ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     -include $(PARAM_FILE)
endif

NAME = clk_test
obj-m += $(NAME).o 
CURRENT_PATH := $(shell pwd)







all:
	touch $(NAME).c 
	echo "#include \"main.c\"" > $(NAME).c
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH)  modules  ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE)
	cp $(NAME).ko ~/nfs/
clean:
	rm -rf $(NAME).c
	rm -rf *.o *.ko .*.cmd *.mod.* modules.order Module.symvers .tmp_versions
