#include "../my_driver.h"



#define drive_name "leds"
#define dev_name "atomic_int"

//#define atomic_type_int
#ifdef atomic_type_int
#define int_atomic
#else
#define bit_atomic
#endif
static struct file_operations leds_fops;
static struct miscdevice my_leds_dev;
static struct platform_driver leds_driver;
#ifdef int_atomic
static atomic_t atomic_num=ATOMIC_INIT(0);
#else
unsigned long int value_bit  = 0;	
#endif
static int leds_open(struct inode *pinode, struct file *files)
{
	#ifdef int_atomic
	if(atomic_read(&atomic_num)>0)
	{
		return -EBUSY;
	}
	atomic_inc(&atomic_num);
	#else
	if(test_bit(0,&value_bit)!=0){
		return -EBUSY; 
	}
	set_bit(0,&value_bit);
	#endif
	printk("leds open..\n");
	return 0;
}
static int leds_release(struct inode *pinode, struct file *files)
{
	printk("leds close..\n");
	#ifdef int_atomic
	atomic_dec(&atomic_num);
	#else
	clear_bit(0,&value_bit);
	#endif
	return 0;
}
static long leds_unlocked_ioctl(struct file *files, unsigned int cmd, unsigned long argc)
{
	printk("leds ioctl.%d.%d\n",cmd,argc);
	return 0;
}

static int leds_probe(struct platform_device *pdev)
{
	int ret,i;
	printk("leds:leds_probe enter!\n");
	ret = misc_register(&my_leds_dev);
	if(ret<0)
	{
		printk("leds:register device failed!\n");
	}
	return 0;


}
static int leds_remove (struct platform_device *pdev)                                                                 
{      
	int i;   
    misc_deregister(&my_leds_dev);
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

