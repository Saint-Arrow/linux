/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/interrupt.h>

#include <linux/notifier.h>

#include <linux/netdevice.h>
//https://blog.csdn.net/iampisfan/article/details/53470188?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-1-53470188-blog-112234507.pc_relevant_vip_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-1-53470188-blog-112234507.pc_relevant_vip_default&utm_relevant_index=2

static int test_event1(struct notifier_block *this, unsigned long event, void *ptr)
{
    	struct netdev_notifier_info * info=(struct netdev_notifier_info *)ptr;
        printk("Event Number is %x,%s,%s", event,info->dev->name,info->dev->perm_addr);
        /*
        Event Number is 5,eth0,
        Event Number is 1,eth0,
        register test_notifier1 completed 
        board_init
        Event Number is 9,lo,
        Event Number is 2,lo,
        Event Number is d,lo,
        Event Number is 1,lo,
        */
        printk("test dev_name:%s\n",dev_name(& (info->dev->dev)));
        return 0; 
}
static struct notifier_block test_notifier1 =
{
        .notifier_call = test_event1,
};

#include <linux/reboot.h>
static int test_event2(struct notifier_block *this, unsigned long event, void *ptr)
{
    int ret=1;
    printk("reboot notifier 0x%x\n",event);
    /*
    /mnt # insmod notifier.ko 
    /mnt # echo "7 4 1 7" > /proc/sys/kernel/printk 
    /mnt # reboot
    /mnt # umount: 192.168.1.191:/home/cwj/nfs busy - remounted read-only
    umount: can't remount devtmpfs read-only
    The system is going down NOW!
    Sent SIGTERM to all processes
    Sent SIGKILL to all processes
    Requesting system reboot
    [   24.362900] reboot notifier
    [   24.366046] reboot: Restarting system
    */
    WARN_ONCE(ret,"reboot WARN_ONCE");
    /*可以查看谁调用了reboot函数接口*/
    /*
    [  510.840998] ------------[ cut here ]------------
    [  510.845659] WARNING: CPU: 0 PID: 869 at /home/cwj/driver_lianxi/HISI/notifier/main.c:58 test_event2+0x44/0x58 [notifier]
    [  510.856526] reboot WARN_ONCE[  510.859225] Modules linked in:
     notifier(O)
    [  510.863513] CPU: 0 PID: 869 Comm: hello_world Tainted: G           O    4.9.37 #40
    */
    
}

static struct notifier_block test_notifier2 =
{
        .notifier_call = test_event2,
};
int board_init(void)
{
    int err=0;
    err = register_netdevice_notifier(&test_notifier1);
    if (err)
    {
            printk("register test_notifier1 error ");
            return -1; 
    }
    printk("register test_notifier1 completed ");
    
    err = register_reboot_notifier(&test_notifier2);
    if (err)
    {
            printk("register test_notifier1 error ");
            return -1; 
    }
    printk("register test_notifier1 completed ");
    
    
    pr_err("board_init\n");
    return 0;
}

void board_cleanup(void)
{
    unregister_netdevice_notifier(&test_notifier1);
    unregister_reboot_notifier(&test_notifier2);
    pr_err("board_cleanup\n");
}




module_init(board_init);
module_exit(board_cleanup);
MODULE_LICENSE("GPL");