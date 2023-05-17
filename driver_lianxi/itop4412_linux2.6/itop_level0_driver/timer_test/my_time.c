#include"../my_driver.h"


struct timer_list demo_timer;

static void time_func(unsigned long data)
{
	printk("%s ,secs = %ld!\n",(char *)data,jiffies/HZ);
	
	mod_timer(&demo_timer,jiffies + 5*HZ);
}


static int timer_init(void)
{
	int ret;
	printk(KERN_EMERG "timer_init enter\n");
	setup_timer(&demo_timer,time_func,(unsigned long) "demo_timer!");
	demo_timer.expires = jiffies + 1*HZ;
	add_timer(&demo_timer);
}
static int timer_exit(void)
{
	printk(KERN_EMERG "demo_timer exit\n");
	del_timer(&demo_timer);
}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");
