#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");



static int hello_open(struct inode *pinode, struct file *files)
{
printk("hello open..\n");
return 0;
}
static int hello_release(struct inode *pinode, struct file *files)
{
printk("hello close..\n");
return 0;
}
static long hello_unlocked_ioctl(struct file *files, unsigned int cmd, unsigned long argc)
{
printk("hello ioctl..\n");
return 0;
}

static struct file_operations hello_fops={
	.owner=THIS_MODULE,
	.open=hello_open,
	.release=hello_release,
	.unlocked_ioctl=hello_unlocked_ioctl,
};

static struct miscdevice hello={
	.minor=MISC_DYNAMIC_MINOR,	
	.name ="hello_dev",
	.fops=&hello_fops,

};


static int hello_init(void)
{
	int flag;
	printk(KERN_EMERG "Hello world driver enter\n");
	flag=misc_register(&hello);
	printk(KERN_EMERG "hello driver register %d\n",flag);
	return 0;
}
static int hello_exit(void)
{
	misc_deregister(&hello);
	printk(KERN_EMERG "Hello world driver exit\n");
	return 0;
}

module_init(hello_init);
module_exit(hello_exit);


