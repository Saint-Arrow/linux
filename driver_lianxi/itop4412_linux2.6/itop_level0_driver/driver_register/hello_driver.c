#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");


static int hello_probe(struct platform_device *pdev)
{
	printk(KERN_EMERG "hello probe init");
	return 0;
}
static int hello_remove(struct platform_device *pdev)
{
	printk(KERN_EMERG "hello remove");
	return 0;
}
static void hello_shutdown(struct platform_device *pdev)
{
}
static int hello_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int hello_resume(struct platform_device *pdev)
{
	return 0;
}
struct platform_driver hello={
	.driver={
		.name ="hello",
		.owner=THIS_MODULE,
	},
	.probe=hello_probe,
	.remove=hello_remove,
	.shutdown=hello_shutdown,
	.suspend=hello_suspend,
	.resume=hello_resume,

};


static int hello_init(void)
{
	int flag;
	printk(KERN_EMERG "Hello world driver enter\n");
	flag=platform_driver_register(&hello);
	printk(KERN_EMERG "hello driver register %d\n",flag);
	return 0;
}
static int hello_exit(void)
{
	platform_driver_unregister(&hello);
	printk(KERN_EMERG "Hello world driver exit\n");
	return 0;
}

module_init(hello_init);
module_exit(hello_exit);
