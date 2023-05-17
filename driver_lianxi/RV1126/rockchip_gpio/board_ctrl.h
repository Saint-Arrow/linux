/************************************************************************
* 版权所有 (C)2009, 深圳市中兴通讯股份有限公司。
* 
* 文件名称： board_ctrl.h
* 文件标识： 
* 内容摘要： Gtting board information and board setting
* 其它说明： 
* 当前版本： 1.0
* 作    者：        songyonglai
* 完成日期： 2010 0501
* 
************************************************************************/

#ifndef BOARD_CTRL_H
#define BOARD_CTRL_H

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/errno.h>
#include <linux/platform_device.h>
//#include <mach/clkdev.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>

/**************************************************************************
 *                        常量                                           							    *
 **************************************************************************/
 /*
 * Ioctl definitions
 */
/* Use 'B' as magic number */
#define BOARD_IOC_MAGIC  'B'
#define GPIO_GET_DATA	 _IOR(BOARD_IOC_MAGIC,100,int)
#define GPIO_DOWN 	_IOR(BOARD_IOC_MAGIC,101,int)
#define GPIO_UP 	 	_IOR(BOARD_IOC_MAGIC,102,int)
#define GPIO_SET_INPUT 	 	_IOR(BOARD_IOC_MAGIC,103,int)

#define GPIO_RESET _IOR(BOARD_IOC_MAGIC,104,int)
#define GPIO_W     _IOW(BOARD_IOC_MAGIC,105,int)
#define GPIO_R     _IOR(BOARD_IOC_MAGIC,106,int)
#endif /*  BOARD_CTRL_H */

