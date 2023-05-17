#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>

#define drive_name "pollkeyirq"



static int led_gpios[]={
	EXYNOS4_GPX1(1),
	EXYNOS4_GPX1(2),
};
#define GPIO_NUM ARRAY_SIZE(led_gpios)

static struct platform_driver pollkey_driver;
struct fasync_struct *pollkey_fasync_p;
int key_count;

struct platform_device *pollkey_dev_p;

static irqreturn_t interrupt9_fun(int irq,void *dev_id)
{
	printk("interrupt9_fun...");
	key_count=9;
	//POLL_IN  == ready to read
	//POLL_OUT == ready to write
	kill_fasync (&pollkey_fasync_p, SIGIO, POLL_IN);
}
static irqreturn_t interrupt10_fun(int irq,void *dev_id)
{
	printk("interrupt10_fun...");
	key_count=10;
	//POLL_IN  == ready to read
	//POLL_OUT == ready to write
	kill_fasync (&pollkey_fasync_p, SIGIO, POLL_IN);
}
static int pollkey_open(struct inode *inode_p, struct file *file_p)
{
	int ret,i;
    printk("gpio_read_probe enter!\n");
	for(i=0;i<GPIO_NUM;i++)
	{
		ret=gpio_request(led_gpios[i],"gpio");
		if(ret<0)
		{
			printk("gpio_request %d failed %x\n ",i,ret);
			return ret;
		}
		s3c_gpio_cfgpin(led_gpios[i],S3C_GPIO_INPUT);
		s3c_gpio_setpull(led_gpios[i],S3C_GPIO_PULL_NONE);
	}
    ret=request_irq(IRQ_EINT(9),interrupt9_fun,IRQF_TRIGGER_FALLING,"my_irq 9",pollkey_dev_p); 
	if(ret<0)
	{
		printk("gpio_irq_request %d failed %x\n ",9,ret);
	}
	ret=request_irq(IRQ_EINT(10),interrupt10_fun,IRQF_TRIGGER_FALLING,"my_irq 10",pollkey_dev_p); 
	if(ret<0)
	{
		printk("gpio_irq_request %d failed %x\n ",10,ret);
	}
	
	return ret;
}
int pollkey_fasync(int fd, struct file *file_p, int on)
{
	printk("driver: fifth_drv_fasync\n");  
    //初始化/释放 fasync_struct 结构体 (fasync_struct->fa_file->f_owner->pid)  
    return fasync_helper(fd, file_p, on, &pollkey_fasync_p); 
}
ssize_t pollkey_read(struct file *file_p, char __user *user_p, size_t size, loff_t *num)
{
	copy_to_user(user_p, &key_count, 1);
	return 1;
}
static int pollkey_release(struct inode *inode_p, struct file *file_p)
{
	int i;
	for(i=0;i<GPIO_NUM;i++)
	{
		gpio_free(led_gpios[i]);
	}
	free_irq(IRQ_EINT(9),pollkey_dev_p);
	free_irq(IRQ_EINT(10),pollkey_dev_p);
	pollkey_fasync(-1,file_p,0);
	return 0;
}

struct file_operations pollkey_fops ={
	.owner=THIS_MODULE,
	.open=pollkey_open,
	.release=pollkey_release,
	.fasync=pollkey_fasync,
	.read=pollkey_read,
};
static struct miscdevice pollkey_dev={
	.minor=MISC_DYNAMIC_MINOR,//次设备号有系统分配，主设备号misc固定为10
	.name ="/dev/buttons", //板子上dev目录下显示的名字
	.fops=&pollkey_fops,
};
static int pollkey_probe(struct platform_device *pdev)
{
	pollkey_dev_p=pdev;
	return misc_register(&pollkey_dev);
}
static int pollkey_remove (struct platform_device *pdev)
{
	misc_deregister(&pollkey_dev);
}
static struct platform_driver pollkey_driver = {
                .driver = {
                .name = drive_name,
                .owner = THIS_MODULE,
        },
        .probe = pollkey_probe,
        .remove = pollkey_remove,

};
static int pollkey_init(void)
{
	int ret;
	printk(KERN_EMERG "pollkey driver enter\n");
	ret=platform_driver_register(&pollkey_driver);
	if(ret<0)
	{
		printk("read ko error");
	}	
	return ret;
}
static int pollkey_exit(void)
{
	platform_driver_unregister(&pollkey_driver);
	return 0;
}
module_init(pollkey_init);
module_exit(pollkey_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");

