#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>

#define drive_name "read_gpio_ctl"
#define dev_name "read_gpioo_ctl"


static int led_gpios[]={
	EXYNOS4_GPC0(3),
	EXYNOS4_GPX0(6),
};
#define GPIO_NUM ARRAY_SIZE(led_gpios)
static struct file_operations gpio_read_fops;
static struct miscdevice gpio_read;
static struct platform_driver gpio_read_driver;

static int gpio_read_open(struct inode *pinode, struct file *files)
{
        printk("gpio_read open..\n");
        return 0;
}
static int gpio_read_close(struct inode *pinode, struct file *files)
{
        printk("gpio_read close..\n");
        return 0;
}
static long gpio_read_ioctl(struct file *files, unsigned int cmd, unsigned long argc)
{
	printk("gpio_read ioctl..\n");
	if(cmd > 1)
	{
		printk("cmd is 0,1");
		return -1;
	}
	if(argc > 1)
	{
		printk("argc is 0,1");
		return -1;
	}
	
	if(0==cmd)
	{
		return gpio_get_value(led_gpios[0]);
	}
	else
	{
		return gpio_get_value(led_gpios[1]);
	}
	
}


static int gpio_read_probe(struct platform_device *pdev)
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
	ret = misc_register(&gpio_read);
	if(ret<0)
	{
		printk("misc_register failed!\n");
		goto exit;
	}
	return 0;
exit:
	for(i=0;i<GPIO_NUM;i++)
	{
		gpio_free(led_gpios[i]);
	}
	return ret;
}

static int gpio_read_remove (struct platform_device *pdev)
{
	int i;
	for(i=0;i<GPIO_NUM;i++)
	{
		gpio_free(led_gpios[i]);
	}
	misc_deregister(&gpio_read);
	return 0;
}








static struct file_operations gpio_read_fops={
	.owner=THIS_MODULE,
	.open=gpio_read_open,
	.release=gpio_read_close,
	.unlocked_ioctl=gpio_read_ioctl,
};
static struct miscdevice gpio_read={
        .minor=MISC_DYNAMIC_MINOR,//次设备号有系统分配，主设备号misc固定为10
        .name=dev_name , //板子上dev目录下显示的名字
        .fops=&gpio_read_fops,
       };
static struct platform_driver gpio_read_driver = {
		.driver = {
                .name = drive_name,
                .owner = THIS_MODULE,
        },
        .probe = gpio_read_probe,
        .remove = gpio_read_remove,
		//.shutdown=gpio_read_shutdown,
        //.suspend=gpio_read_suspend,
        //.resume=gpio_read_resume,
        
};

static int gpio_read_init(void)
{
	int ret;
	printk(KERN_EMERG "my_leds driver enter\n");
	ret=platform_driver_register(&gpio_read_driver);
	if(ret<0)
	{
		printk("read ko error");
	}	
	return ret;
}
static int gpio_read_exit(void)
{
	platform_driver_unregister(&gpio_read_driver);
	return 0;
}
module_init(gpio_read_init);
module_exit(gpio_read_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");

