#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");


static void hello_release(struct device *pdev)
{
	printk(KERN_EMERG "hello release");
}

struct platform_device hello={
	.name ="hello",
	.id = -1,
	.dev={
		.release=hello_release,
	},
};


static int hello_init(void)
{
	int flag;
	printk(KERN_EMERG "Hello world enter\n");
	flag=platform_device_register(&hello);
	printk(KERN_EMERG "hello register %d\n",flag);
	return 0;
}
static int hello_exit(void)
{
	platform_device_unregister(&hello);
	printk(KERN_EMERG "Hello world exit\n");
	return 0;
}

module_init(hello_init);
module_exit(hello_exit);
