#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>


#define drive_name "leds"
#define dev_name "leds"


static int led_gpios[]={
	EXYNOS4_GPL2(0),
	EXYNOS4_GPK1(1),
};
#define LED_NUM ARRAY_SIZE(led_gpios)

#define Major_Dev 9
#define Minor_Dev 9
#define Minor_num 2
int major_dev=0;
int minor_dev=0;
dev_t chrdev_t_num;
module_param(major_dev,int,0644);
module_param(minor_dev,int,0644);

static int leds_open(struct inode *pinode, struct file *files)
{
	printk("leds open..\n");
	return nonseekable_open(pinode,files);
}
static int leds_release(struct inode *pinode, struct file *files)
{
	printk("leds close..\n");
	return 0;
}
static long leds_unlocked_ioctl(struct file *files, unsigned int cmd, unsigned long argc)
{
	printk("leds ioctl.%d.%d\n",cmd,argc);
	switch(cmd)
	{
		case 0:
		case 1:
		if(argc>LED_NUM)
		{
			return -EINVAL;
		}
		gpio_set_value(led_gpios[argc],cmd);
        break;
	}
	return 0;
}

static struct file_operations leds_fops={
	.owner=THIS_MODULE,
	.open=leds_open,
	.release=leds_release,
	.unlocked_ioctl=leds_unlocked_ioctl,
};
static struct cdev my_leds_dev={
.owner=THIS_MODULE,
.ops=&leds_fops,

};
static int leds_probe(struct platform_device *pdev)
{
	int ret,i;
	printk("leds:leds_probe enter!\n");
	for(i=0; i<LED_NUM; i++)
	{
			ret = gpio_request(led_gpios[i], "LED");
			if (ret) {
					printk("%s: request GPIO %d for LED failed, ret = %d\n", drive_name,
									led_gpios[i], ret);
					return ret;
			}

			s3c_gpio_cfgpin(led_gpios[i], S3C_GPIO_OUTPUT);
			gpio_set_value(led_gpios[i], 1);
	}
	if((0==major_dev)&&(0==minor_dev))
	{
		/*5代表次设备号开始分配的基数*/
		ret =alloc_chrdev_region(&chrdev_t_num,5,Minor_num,dev_name);
	}
	else
	{
		major_dev=Major_Dev;
		minor_dev=Minor_Dev;
		chrdev_t_num=MKDEV(major_dev,minor_dev);
		ret = register_chrdev_region(chrdev_t_num,Minor_num,dev_name);
	}
	if(ret<0)
	{
			printk("leds:register device failed!\n");
			goto exit;
	}

	return 0;

exit:
	for(i=0; i<LED_NUM; i++)
	{
		gpio_free(led_gpios[i]);
	}
    unregister_chrdev_region(chrdev_t_num,Minor_num);
	return ret;
}
static int leds_remove (struct platform_device *pdev)                                                                 
{      
	int i;
	for(i=0; i<LED_NUM; i++)
	{
		gpio_free(led_gpios[i]);
	}                                                                                                               
    unregister_chrdev_region(chrdev_t_num,Minor_num);
	printk(KERN_EMERG "my leds driver remove and destroy misc dev\n");		
    return 0;                                                                                                     
}                                                                                                                     
                                                                                                                      
static int leds_suspend (struct platform_device *pdev, pm_message_t state)                                            
{                                                                                                                     
        printk("leds suspend:power off!\n");                                                                         
        return 0;                                                                                                     
}
        
static int leds_resume (struct platform_device *pdev)                                                                 
{               
        printk("leds resume:power on!\n");                                                                           
        return 0;
}
static struct platform_driver leds_driver = {                                                                         
        .probe = leds_probe,
        .remove = leds_remove,                                                                                        
        .suspend = leds_suspend,                                                                                      
        .resume = leds_resume,                                                                                        
        .driver = {                                                                                                   
                .name = drive_name,                                                                                  
                .owner = THIS_MODULE,                                                                                 
        },
}; 
static int leds_init(void)
{
	printk(KERN_EMERG "my_leds driver enter\n");
	return platform_driver_register(&leds_driver);
}
static int leds_exit(void)
{
	platform_driver_unregister(&leds_driver);
	return 0;
}

module_init(leds_init);
module_exit(leds_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");

