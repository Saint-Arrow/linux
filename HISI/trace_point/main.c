/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/interrupt.h>

#if 0
#include <linux/sched.h>
#include <linux/proc_fs.h>
/*https://github.com/jasonactions/Linux-labs/tree/master/trace_point*/
#include "tp-samples-trace.h"
DEFINE_TRACE(subsys_event);

struct proc_dir_entry *pentry_sample;

void my_subsys_event(void *__data, struct inode *inode, struct file *file)
{
	printk("inode: 0x%lx, file: 0x%lx\n", (unsigned long)inode, (unsigned long)file);

}

static int my_open(struct inode *inode, struct file *file)
{
	trace_subsys_event(inode, file);
	
	return 0;
}

static const struct file_operations mark_ops = {
	.open = my_open,
	.llseek = noop_llseek,
};
int board_init(void)
{
    pentry_sample = proc_create("tracepoint-sample", 0, NULL,
		&mark_ops);
	if (!pentry_sample)
		return -EPERM;

	register_trace_subsys_event(my_subsys_event, NULL);
    pr_err("board_init\n");
    return 0;
}

void board_cleanup(void)
{
    unregister_trace_subsys_event(my_subsys_event,NULL);
    remove_proc_entry("tracepoint-sample", NULL);
    pr_err("board_cleanup\n");
}
#else
    
#include <linux/spi/spi.h>
#if 1
#include <trace/events/spi.h>
#else
#include <linux/ktime.h>
#include <linux/tracepoint.h>
DEFINE_EVENT(spi_master, spi_master_idle,

	TP_PROTO(struct spi_master *master),

	TP_ARGS(master)

);
#endif

/*注意默认的tracepoint是静态的，也就是static，如果需要在其他模块中使用需要EXPORT_TRACEPOINT_SYMBOL或_GPL来导出使用*/
//DEFINE_TRACE(spi_master_idle);
//EXPORT_TRACEPOINT_SYMBOL(spi_master_idle);


static void spi_master_idle_test(void *data,struct spi_master *master)
{
    pr_err("data:%p bus_num:%d\n",data,master->bus_num);
    return ;
}
int board_init(void)
{
	register_trace_spi_master_idle(spi_master_idle_test, (void*)7);//这个7就是回调函数中的void *data值
    pr_err("trace:%d\n",trace_spi_master_idle_enabled());
    
    pr_err("board_init\n");
    return 0;
}

void board_cleanup(void)
{
    unregister_trace_spi_master_idle(spi_master_idle_test,NULL);
    pr_err("board_cleanup\n");
}
#endif



module_init(board_init);
module_exit(board_cleanup);
MODULE_LICENSE("GPL");