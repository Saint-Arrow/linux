#ifndef MY_DRIVER
#define MY_DRIVER


/*驱动所需基本头文件*/
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
/*模块获取参数头文件*/
#include <linux/moduleparam.h>
/*MKDEV头文件*/
#include <linux/kdev_t.h>
/*file operation头文件*/
#include <linux/fs.h>
#include <linux/poll.h>
/*class头文件*/
#include <linux/device.h>
/*misc设备头文件*/
#include <linux/miscdevice.h>
/*字符设备头文件*/
#include <linux/cdev.h>
/*hid*/
#include <linux/usb/input.h>
#include <linux/hid.h>


/*linux gpio头文件*/
#include <linux/gpio.h>
/*i2c头文件*/
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
/*SPI 头文件*/
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
/*延时头文件*/
#include <linux/delay.h>
/*定时器头文件*/
#include <linux/timer.h>
#include <linux/jiffies.h>
/*中断头文件*/
#include <linux/irq.h>
#include <linux/interrupt.h>



/*分配内存函数头文件*/
#include <linux/slab.h>


/*iomap头文件*/
#include <asm/io.h>
/*原子操作的函数头文件*/
#include <asm/atomic.h>
/*copy to user头文件*/
#include <asm/uaccess.h>


#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>
#define  trace  {pr_err("%s[%d] \n",__func__, __LINE__);}
#endif