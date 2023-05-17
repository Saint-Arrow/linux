/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/interrupt.h>
#include <linux/mm.h>

#include <linux/notifier.h>

#include <linux/netdevice.h>
//https://blog.csdn.net/iampisfan/article/details/53470188?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-1-53470188-blog-112234507.pc_relevant_vip_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-1-53470188-blog-112234507.pc_relevant_vip_default&utm_relevant_index=2

#include <linux/kallsyms.h>
unsigned long msys_call_table=0;

#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>



#define COLOR_NONE          "\033[0m"
#define COLOR_RED           "\033[0;31m"


static unsigned int pid = 0;
module_param(pid, uint, S_IRUGO|S_IWUSR);


typedef int (*access_process_vm_p)(struct task_struct *tsk, unsigned long addr, void *buf, int len,
		unsigned int gup_flags);
static access_process_vm_p call=NULL;

typedef int (*get_cmdline_p)(struct task_struct *task, char *buffer, int buflen);
static get_cmdline_p get_cmdline_m=NULL;
static void deal_raw_cmdline(char *buffer, unsigned int length)
{
        int i = 0;
        for (i = 0; i < length; i ++) {
                if (buffer[i] == '\0') {
                        buffer[i] = ' ';
                }
        }
}

static void print_process_arg(struct task_struct *task)
{
    char arg[256]="";
    int ret=0;
    if(get_cmdline_m!=NULL)
    {
        ret=get_cmdline_m(task,  arg, 256);
    }
    

    deal_raw_cmdline(arg, ret);
    printk(KERN_EMERG "process[%p %d] %s,arg:%s\n",task,task->pid,task->comm,arg);
}

static void loop_process_parent(struct task_struct *task)
{
    printk(KERN_EMERG "loop begin\n");
    while(task && task->parent)
    {
        print_process_arg(task->parent);
        task=task->parent;
        if(1 == task->pid )
        {
            break;
        }
    }
    printk(KERN_EMERG "loop end\n");
}




static int test_event1(struct notifier_block *this, unsigned long event, void *ptr)
{
    	struct netdev_notifier_info * info=(struct netdev_notifier_info *)ptr;
        printk("Event Number is %lx,%s,%s", event,info->dev->name,info->dev->perm_addr);
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
    
    printk("reboot notifier 0x%lx\n",event);
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
    WARN_ONCE(ret,COLOR_RED"reboot called by %s[pid:%d] \n"COLOR_NONE,current->comm,current->pid);
    print_process_arg(current);
    loop_process_parent(current);

    if (1){
        struct mm_struct *mm=current->mm;
        unsigned long arg_start, arg_end;
        int _count;
        char arg[256];
        
        down_read(&mm->mmap_sem);
        arg_start = mm->arg_start;
        arg_end = mm->arg_end;
        up_read(&mm->mmap_sem);
        
        _count=arg_end-arg_start;
        
        _count = min3(_count, 256, 256);
        #if 0
        nr_read = access_process_vm(current, arg_start, arg, _count, FOLL_ANON);
        #endif

        msys_call_table = kallsyms_lookup_name("access_process_vm");
        printk("sys_call_table:%lx\n",msys_call_table);
        if(msys_call_table!=0)
        {
            msys_call_table++;
            call=(access_process_vm_p)msys_call_table;
            ret=call(current, arg_start, arg, _count, 0);
            deal_raw_cmdline(arg, ret);
        }
        
        printk("arg:%s \n",arg);
    }
    
                            
    /*可以查看谁调用了reboot函数接口
	*但是如果是system("reboot"),则看不出来,reboot called by linuxrc[pid:1271] ,parent:linuxrc[pid:1] ,gparent:swapper/0[pid:0]
	*而进程参数则为init
	*阅读busybox的代码，发现确实是init进程收到信号后的reboot处理，而发送信号的是c库实现的
	*实际上信号的监控可以使用strace，如   ./strace_268  -f  -e trace=process -p 1026 -p 1024   
	* execve("/bin/sh", ["sh", "-c", "sleep 101 && reboot"], 0xbe9b3e5c ) = 0
	*所以本文 介绍的 notify 的方式适合于 直接调用 _reboot 的方法，甚至可能这个也可以用strace跟踪系统调用出来
	*/
    /*
    [  510.840998] ------------[ cut here ]------------
    [  510.845659] WARNING: CPU: 0 PID: 869 at /home/cwj/driver_lianxi/HISI/notifier/main.c:58 test_event2+0x44/0x58 [notifier]
    [  510.856526] reboot WARN_ONCE[  510.859225] Modules linked in:
     notifier(O)
    [  510.863513] CPU: 0 PID: 869 Comm: hello_world Tainted: G           O    4.9.37 #40
    */
	
	return 0; 
    
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

    {//kallsyms_lookup_name test..
        msys_call_table = kallsyms_lookup_name("spi_sync");
        printk("get_cmdline_p:%lx %p\n",msys_call_table,spi_sync);
        msys_call_table = kallsyms_lookup_name("devm_gpio_request");
        printk("get_cmdline_p:%lx %p\n",msys_call_table,devm_gpio_request);
        msys_call_table = kallsyms_lookup_name("i2c_new_device");
        printk("get_cmdline_p:%lx %p\n",msys_call_table,i2c_new_device);
    }
    {
        msys_call_table = kallsyms_lookup_name("get_cmdline");
        printk("get_cmdline_p:%lx %p\n",msys_call_table,get_cmdline);
        if(msys_call_table!=0)
        {
            msys_call_table++;
            get_cmdline_m=(get_cmdline_p)msys_call_table;
        }
    }
    

    if(pid){
       struct pid *current_pid = find_get_pid(pid);
       struct task_struct *tsk = pid_task(current_pid, PIDTYPE_PID);
       print_process_arg(tsk);
       loop_process_parent(tsk);
   }

    
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