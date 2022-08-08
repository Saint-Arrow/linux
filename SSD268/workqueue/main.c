/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

static struct kthread_worker *kworker;
void kthread_work_f(struct kthread_work *work)
{
    printk("kthread_work_f enter\n");
}
DEFINE_KTHREAD_WORK(kwork,kthread_work_f);



static struct workqueue_struct *queue = NULL;


static struct work_struct mywork;
void work_handler(struct work_struct *work)  
{
    int ret=1;
    printk("work_handler enter\n");
    WARN_ONCE(ret,"tets");
}

int board_init(void)
{
    kworker=kthread_create_worker_on_cpu(0,0,"kworker_test");
    kthread_queue_work(kworker,&kwork);
    
    
    /*创建工作队列，create_singlethread_workqueue 创建单个cpu执行的队列，而create_workqueue创建每个cpu都执行的队列*/
    //queue=create_workqueue("helloworld"); 
    queue=create_singlethread_workqueue("helloworld"); 
    
    INIT_WORK(&mywork, work_handler);
    #if 0
    schedule_work(&mywork); //system_wq 默认队列中调度，这个队列不建议运行太长时间，如果需要长时间的任务，请使用 system_long_wq
    #else
    queue_work(queue,&mywork);
    #endif
    
    pr_err("board_init\n");
    return 0;
}

void board_cleanup(void)
{
    flush_workqueue(queue);
    destroy_workqueue(queue);
    
    kthread_flush_worker(kworker);
    kthread_destroy_worker(kworker);
    pr_err("board_cleanup\n");
}




module_init(board_init);
module_exit(board_cleanup);
MODULE_LICENSE("GPL");