/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/interrupt.h>

#include <linux/io.h>

#define SSP_SIZE                  0x1000  // 64KB
#define MOTOR_DRV_IO_REG_BASE0    		   (0x120B5400)  //gpio5
static void __iomem *motor_drv_io_reg_base0;
static void __iomem *motor_drv_io_reg_base1;
static void __iomem *motor_drv_io_reg_base2;
static void __iomem *motor_drv_io_reg_base3;
int board_init(void)
{
	int     ret;
    motor_drv_io_reg_base2 = ioremap((unsigned long)MOTOR_DRV_IO_REG_BASE0, (unsigned long)SSP_SIZE);
	if (!motor_drv_io_reg_base2)
	{
		printk("Kernel: ioremap mux base failed!\n");
		return -ENOMEM;
	}
    motor_drv_io_reg_base0 = ioremap_nocache((unsigned long)MOTOR_DRV_IO_REG_BASE0, (unsigned long)SSP_SIZE);
	if (!motor_drv_io_reg_base0)
	{
		printk("Kernel: ioremap mux base failed!\n");
		return -ENOMEM;
	}
    motor_drv_io_reg_base1 = ioremap_nocache((unsigned long)MOTOR_DRV_IO_REG_BASE0, (unsigned long)SSP_SIZE);
	if (!motor_drv_io_reg_base1)
	{
		printk("Kernel: ioremap mux base failed!\n");
		return -ENOMEM;
	}
    motor_drv_io_reg_base3 = ioremap((unsigned long)MOTOR_DRV_IO_REG_BASE0, (unsigned long)SSP_SIZE);
	if (!motor_drv_io_reg_base2)
	{
		printk("Kernel: ioremap mux base failed!\n");
		return -ENOMEM;
	}
    pr_err("data:0x%x,nocache:%p nocache:%p cache:%p cache:%p\n",\
    readl(motor_drv_io_reg_base0),motor_drv_io_reg_base0,motor_drv_io_reg_base1,motor_drv_io_reg_base2,motor_drv_io_reg_base3);
    writel(0x5a,motor_drv_io_reg_base0);
    pr_err("motor_drv_io_reg_base0:0x%x\n",readl(motor_drv_io_reg_base0));
    pr_err("motor_drv_io_reg_base1:0x%x\n",readl(motor_drv_io_reg_base1));
    pr_err("motor_drv_io_reg_base2:0x%x\n",readl(motor_drv_io_reg_base2));
    
    
    writel(0xa5,motor_drv_io_reg_base2);
    pr_err("motor_drv_io_reg_base0:0x%x\n",readl(motor_drv_io_reg_base0));
    pr_err("motor_drv_io_reg_base1:0x%x\n",readl(motor_drv_io_reg_base1));
    pr_err("motor_drv_io_reg_base2:0x%x\n",readl(motor_drv_io_reg_base2));
    pr_err("board_init\n");
    return 0;
}

void board_cleanup(void)
{
    if(motor_drv_io_reg_base0) iounmap((void *)motor_drv_io_reg_base0);
    if(motor_drv_io_reg_base1) iounmap((void *)motor_drv_io_reg_base1);
    if(motor_drv_io_reg_base2) iounmap((void *)motor_drv_io_reg_base2);
    if(motor_drv_io_reg_base3) iounmap((void *)motor_drv_io_reg_base3);
    pr_err("board_cleanup\n");
}

module_init(board_init);
module_exit(board_cleanup);
MODULE_LICENSE("GPL");