#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>


#define drive_name "cwj_leds"
#define dev_name "leds"

static int led_gpios[]={
	EXYNOS4_GPL2(0),
	EXYNOS4_GPK1(1),
};
#define LED_NUM ARRAY_SIZE(led_gpios)


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

static struct miscdevice my_leds_dev={
	.minor=MISC_DYNAMIC_MINOR,	
	.name =dev_name,
	.fops=&leds_fops,
};
static int leds_probe(struct platform_device *pdev)
{
	int ret,i;
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

	ret = misc_register(&my_leds_dev);
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
	misc_deregister(&my_leds_dev);
	return ret;
}
static int leds_remove (struct platform_device *pdev)                                                                 
{      
	int i;
	for(i=0; i<LED_NUM; i++)
	{
		gpio_free(led_gpios[i]);
	}                                                                                                               
    misc_deregister(&my_leds_dev);
    pr_err("%s[%d] \n",__func__, __LINE__);	
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
	pr_err("%s[%d] \n",__func__, __LINE__);
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

