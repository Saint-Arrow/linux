#
# DMA Demo Makefile
# 支持编译驱动模块和应用层测试程序
#

# 内核源码路径 - 可通过命令行覆盖
KDIR ?= /data1/cwj/vhd_gitlab/HI3559_SDK/vx90s_sdk/open_source/linux/linux-4.19.y_multi-core

# 交叉编译支持
CROSS_COMPILE ?= aarch64-himix210-linux-
ARCH ?= arm64

# 编译器设置
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

# 驱动模块名称
DRV_NAME = dma_demo_driver

# 应用程序名称
APP_NAME = dma_demo_app

# 目标文件
obj-m := $(DRV_NAME).o

# 默认目标
.PHONY: all clean driver app install uninstall test

all: driver app

# 编译驱动模块
driver:
	@echo "========================================"
	@echo "Building DMA Demo Driver..."
	@echo "Kernel: $(KDIR)"
	@echo "Architecture: $(ARCH)"
	@echo "========================================"
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

# 编译应用程序
app: $(APP_NAME)

$(APP_NAME): $(APP_NAME).c
	@echo "========================================"
	@echo "Building DMA Demo Application..."
	@echo "========================================"
	$(CC) -o $@ $< -Wall -O2

# 清理
clean:
	@echo "Cleaning build files..."
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) clean
	- rm -f $(APP_NAME)
	- rm -f *.o *.ko *.mod.c *.mod.o *.order *.symvers
	- rm -rf .tmp_versions .$(DRV_NAME)*.cmd
	- rm -rf *.o.rc *.o.ur-safe
