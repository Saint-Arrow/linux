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

static struct platform_driver pollkey_driver;

static irqreturn_t interrupt9_fun(int irq,void *dev_id)
{
	printk("interrupt9_fun...");
}
static irqreturn_t interrupt10_fun(int irq,void *dev_id)
{
	printk("interrupt10_fun...");
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
    ret=request_irq(IRQ_EINT(9),interrupt9_fun,IRQF_TRIGGER_FALLING,"my_irq 9",pdev); 
	if(ret<0)
	{
		printk("gpio_irq_request %d failed %x\n ",9,ret);
	}
	ret=request_irq(IRQ_EINT(10),interrupt9_fun,IRQF_TRIGGER_FALLING,"my_irq 10",pdev); 
	if(ret<0)
	{
		printk("gpio_irq_request %d failed %x\n ",10,ret);
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
	free_irq(IRQ_EINT(9),pdev);
	free_irq(IRQ_EINT(10),pdev);
	return 0;
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

