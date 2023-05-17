#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");

static int hello_init(void)
{
	printk(KERN_EMERG "Hello world enter\n");
}
static int hello_exit(void)
{
	printk(KERN_EMERG "Hello world exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
