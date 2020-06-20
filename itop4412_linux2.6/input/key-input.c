/*
 * Samsung keypad driver
 *
 * Copyright (C) 2010 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <plat/keypad.h>

#include <asm/irq.h>
#include <mach/gpio.h>
#include <mach/regs-pmu.h>

#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#define drive_name "cwj_gpio-keys"
#define dev_name "gpio_keys" 

struct samsung_keypad {
	struct input_dev *input_dev;
	struct clk *clk;
	void __iomem *base;
	wait_queue_head_t wait;
	bool stopped;
	int irq;
	unsigned int row_shift;
	unsigned int rows;
	unsigned int cols;
	unsigned int row_state[SAMSUNG_MAX_COLS];
	unsigned short keycodes[];
};
struct input_dev *g_input_dev;
int g_row_shift = 0;


static int led_gpios[]={
	EXYNOS4_GPX1(1),
	EXYNOS4_GPX1(2),
};
char input_name_t[32]="input_test";










static irqreturn_t interrupt9_fun(int irq,void *dev_id)
{
	int ret;
	struct samsung_keypad *keypad=(struct samsung_keypad *)dev_id;
	printk("interrupt9_fun...\n");
	if(keypad->stopped)
	{
		keypad->stopped=0;
		//input_report_key(keypad->input_dev,KEY_HOME,1);
		//input_sync(keypad->input_dev);
		input_event(keypad->input_dev,EV_KEY,KEY_HOME,1);
		input_event(keypad->input_dev,EV_SYN,0,0);
	}
	else
	{
		keypad->stopped=1;
		//input_report_key(keypad->input_dev,KEY_HOME,0);
		//input_sync(keypad->input_dev);
		input_event(keypad->input_dev,EV_KEY,KEY_HOME,0);
		input_event(keypad->input_dev,EV_SYN,0,0);
	}
	printk("interrupt9_leave...\n");
	return IRQ_HANDLED;


}


static int __devinit key_input_probe(struct platform_device *pdev)
{
	const struct samsung_keypad_platdata *pdata;
	const struct matrix_keymap_data *keymap_data;
	struct samsung_keypad *keypad;
	struct resource *res;
	struct input_dev *input_dev;
	unsigned int row_shift;
	unsigned int keymap_size;
	int error;
       int i=0,ret;
       
	error=gpio_request(led_gpios[0],"gpio");
	if(error<0)
	{
		printk("gpio_request %d failed %x\n ",i,ret);
		goto err_free_mem;
	}
	s3c_gpio_cfgpin(led_gpios[0],S3C_GPIO_INPUT);
	s3c_gpio_setpull(led_gpios[0],S3C_GPIO_PULL_NONE);




	
	keypad = kzalloc(sizeof(*keypad), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!keypad || !input_dev) {
		printk("input_allocate_device   failed %x\n ");
		error = -ENOMEM;
		goto err_free_mem;
	}
	keypad->input_dev=input_dev;
	


	
       error=request_irq(IRQ_EINT(9),interrupt9_fun,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"my_irq 9",keypad); 
	if(error<0)
	{
		printk("gpio_irq_request %d failed %x\n ",9,ret);
		goto err_free_mem;
	}
	keypad->irq=IRQ_EINT(9);
	
	set_bit(EV_SYN,keypad->input_dev->evbit);
	set_bit(EV_KEY,keypad->input_dev->evbit);
	
	set_bit(KEY_HOME,keypad->input_dev->keybit); 
	set_bit(KEY_UP,keypad->input_dev->keybit); 
	keypad->input_dev->name=input_name_t;

	error = input_register_device(keypad->input_dev);
	if (error)
	{
		printk(KERN_EMERG "input_register_device error\n");
		goto err_free_irq;
	}


	platform_set_drvdata(pdev, keypad);
	printk(KERN_EMERG "key input probe ok\n");
	return 0;

err_free_irq:
	free_irq(keypad->irq, keypad);
	

err_free_mem:
	input_free_device(input_dev);
	gpio_free(led_gpios[0]);
	kfree(keypad);
	return error;
}

static int __devexit key_input_remove(struct platform_device *pdev)
{
	struct samsung_keypad *keypad = platform_get_drvdata(pdev);


	platform_set_drvdata(pdev, NULL);

	input_unregister_device(keypad->input_dev);

	/*
	 * It is safe to free IRQ after unregistering device because
	 * samsung_keypad_close will shut off interrupts.
	 */
	free_irq(keypad->irq, keypad);
	gpio_free(led_gpios[0]);
	input_free_device(keypad->input_dev);
	kfree(keypad);

	return 0;
}







static struct platform_device_id keypad_driver_ids[] = {
	{
		.name		= "input-keypad",
		.driver_data	= 0,
	}, 
	{ },
};

static struct platform_driver key_input_driver = {
	.probe		= key_input_probe,
	.remove		= key_input_remove,
	.driver = {
                .name = drive_name,
                .owner = THIS_MODULE,
        },
	//.id_table	= keypad_driver_ids,
};

static int pollkey_init(void)
{
	int ret;
	printk(KERN_EMERG "key input driver enter\n");
	ret=platform_driver_register(&key_input_driver);
	if(ret<0)
	{
		printk("read ko error");
	}	
	return ret;
}
static int pollkey_exit(void)
{
	platform_driver_unregister(&key_input_driver);
	return 0;
}
module_init(pollkey_init);
module_exit(pollkey_exit);

MODULE_DESCRIPTION("Samsung keypad driver");
MODULE_AUTHOR("Joonyoung Shim <jy0922.shim@samsung.com>");
MODULE_AUTHOR("Donghwa Lee <dh09.lee@samsung.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:samsung-keypad");
