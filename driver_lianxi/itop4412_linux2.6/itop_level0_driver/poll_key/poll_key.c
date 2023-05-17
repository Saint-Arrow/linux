#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>

#define drive_name "pollkeyirq"
#define dev_name "pollkey"


static int led_gpios[]={
	EXYNOS4_GPX1(1),
	EXYNOS4_GPX1(2),
};
#define GPIO_NUM ARRAY_SIZE(led_gpios)
static struct file_operations pollkey_fops;
static struct miscdevice pollkey_misc;
static struct platform_driver pollkey_driver;

static int key_open(struct inode *pinode, struct file *files)
{
        printk("gpio_read open..\n");
        return 0;
}
static int key_close(struct inode *pinode, struct file *files)
{
        printk("gpio_read close..\n");
        return 0;
}
ssize_t key_read(struct file *pfile, char __user *puser, size_t num, loff_t *num_2)
{
		char buffer[2]={0,0};
		if(gpio_get_value(led_gpios[0])) buffer[0]=1;
		if(gpio_get_value(led_gpios[1])) buffer[1]=1;
		copy_to_user(puser,buffer,sizeof(buffer));
        return 0;
}



static int pollkey_probe(struct platform_device *pdev)
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
	ret = misc_register(&pollkey_misc);
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

static int pollkey_remove (struct platform_device *pdev)
{
	int i;
	for(i=0;i<GPIO_NUM;i++)
	{
		gpio_free(led_gpios[i]);
	}
	misc_deregister(&pollkey_misc);
	return 0;
}








static struct file_operations pollkey_fops={
	.owner=THIS_MODULE,
	.open=key_open,
	.release=key_close,
	//.unlocked_ioctl=gpio_read_ioctl,
	.read=key_read,
	
};
static struct miscdevice pollkey_misc={
        .minor=MISC_DYNAMIC_MINOR,//次设备号有系统分配，主设备号misc固定为10
        .name=dev_name , //板子上dev目录下显示的名字
        .fops=&pollkey_fops,
       };
static struct platform_driver pollkey_driver = {
		.driver = {
                .name = drive_name,
                .owner = THIS_MODULE,
        },
        .probe = pollkey_probe,
        .remove = pollkey_remove,
		//.shutdown=gpio_read_shutdown,
        //.suspend=gpio_read_suspend,
        //.resume=gpio_read_resume,
        
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

