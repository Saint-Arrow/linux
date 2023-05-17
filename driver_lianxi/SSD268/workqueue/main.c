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

#include <linux/delay.h>

#define CNT 32

#define HRIMER_TIMEOUT_NSECOND 1000000000
//static struct hrtimer g_hrtimer;

#define DELAY_TEST 5

static int g_get_time_index = 0;
static struct timespec64 curr_time[CNT];

static struct kthread_worker *kworker;
void kthread_work_f(struct kthread_work *work);
//DEFINE_KTHREAD_WORK(kwork,kthread_work_f);
DEFINE_KTHREAD_DELAYED_WORK(kwork,kthread_work_f);



void kthread_work_f(struct kthread_work *work)
{
    getnstimeofday64(&curr_time[g_get_time_index++]);
    if(g_get_time_index <  CNT)
    {
        kthread_mod_delayed_work(kworker,&kwork,msecs_to_jiffies(DELAY_TEST));
    }
    else
    {
        int i=0;
	    long int time_error = 0;
        for(i=0;i<CNT;i++)
        {
            time_error = curr_time[i].tv_sec*HRIMER_TIMEOUT_NSECOND+curr_time[i].tv_nsec -
        					curr_time[i-1].tv_sec*HRIMER_TIMEOUT_NSECOND-curr_time[i-1].tv_nsec;
        	printk(KERN_ERR"%d : use time %ld error %ld\n",i,time_error,time_error-DELAY_TEST*1000000);
        }
        printk("kthread_work_f enter\n");
    }
    #if 0
    printk(KERN_EMERG "%s[%d] in_interrupt:%lx in_atomic_preempt_off:%x\n",__func__,__LINE__,\
        in_interrupt(),in_atomic_preempt_off());
    msleep(1);
    #endif
}




static struct workqueue_struct *queue = NULL;


static struct work_struct mywork;
void work_handler(struct work_struct *work)  
{
    //int ret=1;

    printk("work_handler enter\n");
    //WARN_ONCE(ret,"tets");

    printk(KERN_EMERG "%s[%d] in_interrupt:%lx in_atomic_preempt_off:%x\n",__func__,__LINE__,\
        in_interrupt(),in_atomic_preempt_off());
    msleep(1);
}

int board_init(void)
{
    kworker=kthread_create_worker_on_cpu(0,0,"kworker_test");
    //kthread_queue_work(kworker,&kwork);
    getnstimeofday64(&curr_time[g_get_time_index++]);
    kthread_mod_delayed_work(kworker,&kwork,msecs_to_jiffies(DELAY_TEST));
    
    printk(KERN_EMERG "%s[%d] in_interrupt:%lx in_atomic_preempt_off:%x\n",__func__,__LINE__,\
        in_interrupt(),in_atomic_preempt_off());
    msleep(1);
    
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