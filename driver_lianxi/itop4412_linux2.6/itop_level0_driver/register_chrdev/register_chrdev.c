#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>


#define drive_name "my_leds"
#define dev_name "my_leds"


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
module_param(major_dev,int,S_IRUSR);
module_param(minor_dev,int,S_IRUSR);

static struct class *my_led_class;

static struct file_operations leds_fops={
	.owner=THIS_MODULE,
};

#define REG_MAX_SIZE 100
struct reg_dev_s
{
	char *data;
	unsigned long size;
	struct cdev my_led;
};
static struct reg_dev_s *my_leds_dev;

/***************************************************/
static int leds_open(struct inode *pinode, struct file *files)
{
	printk("leds open..\n");
	//return nonseekable_open(pinode,files);
	return 0;
}
static int leds_release(struct inode *pinode, struct file *files)
{
	printk("leds close..\n");
	return 0;
}
static long leds_unlocked_ioctl(struct file *files, unsigned int cmd, unsigned long argc)
{
	printk("leds ioctl..\n");
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

static void reg_init_cdev(struct reg_dev_s *my_leds_dev_p,int index)
{
	int err;
	cdev_init(&my_leds_dev_p->my_led,&leds_fops);
	my_leds_dev_p->my_led.owner=THIS_MODULE;
	my_leds_dev_p->my_led.ops=&leds_fops;
	leds_fops.open=leds_open;
	leds_fops.release=leds_release;
	leds_fops.unlocked_ioctl=leds_unlocked_ioctl;
	err=cdev_add(&my_leds_dev_p->my_led,chrdev_t_num,1);
	if(err)
	{
		printk("cdev_add error %d,%x\n",index,err);
	}
}
//static int leds_probe(struct platform_device *pdev)
static int leds_probe(void)
{
	int ret,i;
	printk("leds:leds_probe enter!\n");
	for(i=0; i<LED_NUM; i++)
	{
			ret = gpio_request(led_gpios[i], "LED");
			if (ret) {
				printk("gpio_request failed %x\n",ret);
				return ret;
			}

			s3c_gpio_cfgpin(led_gpios[i], S3C_GPIO_OUTPUT);
			gpio_set_value(led_gpios[i], 1);
	}
	if((0==major_dev)&&(0==minor_dev))
	{
		ret =alloc_chrdev_region(&chrdev_t_num,minor_dev,Minor_num,dev_name);
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
	major_dev=MAJOR(chrdev_t_num);
	minor_dev=MINOR(chrdev_t_num);
	printk("chrdev_t_num %d,%d\n",major_dev,minor_dev);
	
	my_led_class=class_create(THIS_MODULE,dev_name);
	my_leds_dev=kmalloc(Minor_num*sizeof(struct reg_dev_s),GFP_KERNEL);
	if(IS_ERR(my_leds_dev))
	{
		ret=PTR_ERR(my_leds_dev);
		goto exit_2;
	}
	memset(my_leds_dev,0,Minor_num*sizeof(struct reg_dev_s));
	for(i=0;i<Minor_num;i++)
	{
		struct device *dev;
		my_leds_dev[i].data=kmalloc(REG_MAX_SIZE,GFP_KERNEL);
		if(IS_ERR(my_leds_dev[i].data))
		{
			goto process;
		}
		memset(my_leds_dev[i].data,0,REG_MAX_SIZE);
		reg_init_cdev(&my_leds_dev[i],i);
		dev=device_create(my_led_class,NULL,MKDEV(major_dev,minor_dev+i),NULL,dev_name"%d",i);
		if(IS_ERR(dev))
		{
			ret=PTR_ERR(dev);
			kfree(my_leds_dev[i].data);
			cdev_del(&my_leds_dev[i].my_led);
			goto process;
		}
		continue;
		process:
		{
			int j;
			for(j=0;j<i;j++)
			{
				kfree(my_leds_dev[i].data);
				kfree(my_leds_dev[j].data);
			}
			kfree(my_leds_dev);
			goto exit_2;
		}
	}
	printk("cdev init ok\n");
	return 0;
	
exit_2:
	printk("exit_2");
	class_destroy(my_led_class);
	unregister_chrdev_region(chrdev_t_num,Minor_num);
exit:
	printk("exit");
	for(i=0; i<LED_NUM; i++)
	{
		gpio_free(led_gpios[i]);
	}
    
	return ret;
}
//static int leds_remove (struct platform_device *pdev)    
static int leds_remove (void)                                                          
{      
	int i;                                                                                                             
	for(i=0;i<Minor_num;i++)
	{
		kfree(my_leds_dev[i].data);
		cdev_del(&my_leds_dev[i].my_led);
		device_destroy(my_led_class,MKDEV(major_dev,minor_dev+i));	
		//device_destroy(my_led_class,(dev_t)(chrdev_t_num+i));	//these can not work	
	}
	kfree(my_leds_dev);
	class_destroy(my_led_class);
    unregister_chrdev_region(chrdev_t_num,Minor_num);
	for(i=0; i<LED_NUM; i++)
	{
		gpio_free(led_gpios[i]);
	} 
	printk(KERN_EMERG "my leds driver remove and destroy misc dev\n");		
    return 0;                                                                                                     
}                                                                                                                     
 /*                                                                                                                     
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
*/
module_init(leds_probe);
module_exit(leds_remove);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");

