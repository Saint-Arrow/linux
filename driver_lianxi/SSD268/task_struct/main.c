/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <linux/pid.h>
#include <linux/module.h>
#include <linux/sched.h>

#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/fdtable.h>

#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/smp.h>
#if 0
//TODO: enable CONFIG_STACKTRACE
#include <linux/stacktrace.h>
#define MAX_STACK_TRACE_DEPTH 64
static int _kprink_stack(struct task_struct *task)
{
    struct stack_trace trace;
    unsigned long *entries;
    int err;
 
    entries = kmalloc(MAX_STACK_TRACE_DEPTH * sizeof(*entries), GFP_KERNEL);
    if (!entries)
        return -ENOMEM;
 
    trace.nr_entries    = 0;/*调用返回后，为记录的调用栈中的有效符号的个数*/
    trace.max_entries    = MAX_STACK_TRACE_DEPTH;/*传入的trace.entries的大小，save_stack_trace_tsk最多保持该数量的调用栈符号*/
    trace.entries        = entries;/*返回的符号地址保存在这里*/
    trace.skip        = 0;/*从调用栈的顶开始，忽略的调用栈符号数量*/
 
    //err = lock_trace(task);
    if (!err) {
        unsigned int i;
        save_stack_trace_tsk(task, &trace);
        for (i = 0; i < trace.nr_entries; i++) {
            pr_notice("[<%pK>] %pS",
                   (void *)entries[i], (void *)entries[i]);
        }
        //unlock_trace(task);
    }
    kfree(entries);
 
    return err;
}

#endif


static unsigned int pid = 1;
module_param(pid, uint, S_IRUGO|S_IWUSR);

void print_signal(struct task_struct *t)
{
    int sig=0;
    for(sig=1;sig<=_NSIG;sig++)
    {// SIG_DFL:0 SIG_IGN:1 
        if(t->sighand->action[sig - 1].sa.sa_handler != SIG_DFL)
        {
            pr_info("sig[%d],%p 0x%lx\n",sig,t->sighand->action[sig - 1].sa.sa_handler,t->sighand->action[sig - 1].sa.sa_flags);
        }
        
    }
    for(sig=0;sig<_NSIG_WORDS;sig++)
    {
    pr_info("blocked[%d]:%ld",sig,t->blocked.sig[sig ]);
    pr_info("real_blocked[%d]:%ld",sig,t->real_blocked.sig[sig ]);
    pr_info("saved_sigmask[%d]:%ld",sig,t->saved_sigmask.sig[sig ]);
    }
}

static void sb_trace_test(struct super_block *sb, void *arg)
{
    if(sb!=NULL)
    {
        pr_err("sb:%p ",sb);
        pr_err("%s s_blocksize_bits:%d s_blocksize:%ld s_maxbytes:%lld",sb->s_type->name,sb->s_blocksize_bits,sb->s_blocksize,sb->s_maxbytes);
        pr_err("s_dev:%d",sb->s_dev);
    }
    else
    {
        pr_err("sb is null");
    }
}
void print_fd(struct task_struct *t)
{//https://www.freesion.com/article/34301296410/
    //pr_info("open files:%d",t->files);
    char buf[256]="123";
    char* path=NULL;
    int i=0;
    struct file **p_file=NULL;

    /*file_path 填充的文件路径是倒过来的填充的，从后往前，实际返回的地址是在buf中的*/

    #if 0
    /**读取默认 file 数组,这种办法 在fd close掉之后 会出现NULL指针的问题 **/
    rcu_read_lock();
    p_file=t->files->fd_array;
    pr_info("try to open fd_array %p",p_file);
    for (i=0;i<NR_OPEN_DEFAULT;i++)
    {
        if(p_file[i]==NULL)
        {
            //break;
            continue;
        }
        else
        {
            path=file_path(p_file[i],buf,sizeof(buf));
            if(path!=NULL)
            {
                pr_info("open file:%s,%d",path,path-buf);
            }
            else
            {
                pr_info("file_path:fail");
            }
        }
    }
    rcu_read_unlock();
    #endif
    
    /* 真实文件是存在于fdt中，不同内核版本的存放位置有所不同 */
    /* 单打开文件个数少于 NR_OPEN_DEFAULT 时, fdt其实是指向fd_array，如果大了，会重新指向另外的alloc出来的 buf */
    /* NR_OPEN_DEFAULT 与 long 的长度等同  */
    /* 后面即便close掉file    ,这个fdt的地址也不会更换回来了              */
  
    //rcu_read_lock();
    p_file=t->files->fdt->fd;
    pr_info("try to open fdt,max:%d %p fd_array:%p",t->files->fdt->max_fds,p_file,t->files->fd_array);
    for(i=0;i<t->files->fdt->max_fds;i++)
    {
        if(p_file[i]==NULL)
        {
            //break;
            continue;
        }
        else
        {
            path=file_path(p_file[i],buf,sizeof(buf));
            if(path!=NULL)
            {
                pr_info("open file[%d]:%s",i,path);
                sb_trace_test(p_file[i]->f_path.dentry->d_sb,NULL);
            }
            else
            {
                pr_info("file_path:fail");
            }
        }
    }
    //rcu_read_unlock();
}
struct task_struct *thread_test = NULL;
int RAMPSTAT_READ_thread(void *arg)
{
    int process_id=-1;
    int temp=-1;
    while(1)
    {
        if(kthread_should_park())
        {
    		kthread_parkme();
        }
        //kthread_should_stop判断线程是否应该结束，返回true表示结束，false表示不结束
        if(kthread_should_stop())
        {
            break;
        }
        msleep(200);
        temp=smp_processor_id();//获取单前CPU ID
        if(process_id != temp)
        {
            printk("process_id:%d->%d",process_id,temp);
            process_id = temp;
        }
    }
    return 0;
}
int board_init(void)
{
    struct pid *current_pid = find_get_pid(pid);
    struct task_struct *current_task = pid_task(current_pid, PIDTYPE_PID);
    struct pid* parent_pid = get_task_pid(current_task->parent, PIDTYPE_PID);
    struct task_struct *parent = pid_task(parent_pid, PIDTYPE_PID);
    struct timeval uval;
    struct timeval sval;
    unsigned long mm;
	unsigned ss;
    
    pr_info("parent pid: %d,name:%s\n", pid_nr(parent_pid),parent->comm);
    pr_info("current pid:%d %s,state:%ld\n", pid,current_task->comm,current_task->state);
    
    cputime_to_timeval(current_task->utime,&uval);
    cputime_to_timeval(current_task->stime,&sval);
    /*注意进程主线程 如果一直在sleep的话，utime 几乎为0,另外这个时间和proc stat中的信息并不一致！*/
    pr_info("utime:%ld %ld,stime:%ld s\n", uval.tv_sec,cputime_to_secs(current_task->utimescaled),sval.tv_sec);
    
    
    pr_info("start_time :%lld real_start_time:%lld us\n",current_task->start_time,current_task->real_start_time);
    mm=(ktime_get_ns()-current_task->start_time)/NSEC_PER_SEC;
    ss = mm % 60;
	mm /= 60;
    pr_info("run time:%3lu:%02u\n",mm,ss);
    
    
    pr_info("timer_slack_ns :%lld default_timer_slack_ns:%lld ns\n", current_task->timer_slack_ns,current_task->default_timer_slack_ns);
    
    /*打印指定进程堆栈*/
    //dump_backtrace(NULL,current_task);
    //_kprink_stack(current_task);
    
    print_signal(current_task);
    print_fd(current_task);
    
    
    thread_test = kthread_create(RAMPSTAT_READ_thread,NULL,"thread_test");    
    
    if(!IS_ERR(thread_test))
           wake_up_process(thread_test);
       
    pr_err("board_init\n");
    return 0;
}

void board_cleanup(void)
{
  if(thread_test)
    { 
        kthread_stop(thread_test);
        thread_test = NULL;
        printk(KERN_ALERT"thread_test exit");
    }
    pr_err("board_cleanup\n");
}




module_init(board_init);
module_exit(board_cleanup);
MODULE_LICENSE("GPL");