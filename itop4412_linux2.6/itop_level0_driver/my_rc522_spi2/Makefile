obj-m += my_rc522.o
KDIR :=/home/topeet/zImage/iTop4412_Kernel_3.0
PWD ?= $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -rf *.o *.mod.o *.mod.c  *.symvers *.order

