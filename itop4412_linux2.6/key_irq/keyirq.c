#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>

#define drive_name "cwj_gpio-keys"
#define dev_name "gpio_keys" 


static int led_gpios[]={
	EXYNOS4_GPX1(1),
	EXYNOS4_GPX1(2),
};
#define GPIO_NUM ARRAY_SIZE(led_gpios)
static struct file_operations pollkey_fops;
static struct miscdevice pollkey_dev;
static struct platform_driver pollkey_driver;
#define BUF_LEN 10
typedef struct 
{
	char name[32];
	//struct cdev cdev;
	struct miscdevice cdev;
	char mem[BUF_LEN];
	int read_pos;
	int write_pos;
	int current_len;
	struct semaphore sem; /*并发控制用的信号量*/           
	wait_queue_head_t r_wait; /*阻塞读用的等待队列头*/     
	wait_queue_head_t w_wait; /*阻塞写用的等待队列头*/ 
	spinlock_t key_lock;
	int open_count;
}keyirq_s;
static int pollkey_open(struct inode *filp, struct file *files)
{
	keyirq_s *dev=container_of(files->private_data, keyirq_s, cdev);
	printk("%s.open..\n",dev->name);
	if(dev->open_count) 
	{
		printk(".pollkey_open.error.\n");
		return -1;
	}

	dev->open_count++;
	files->private_data=dev;
	return 0;
}
static int pollkey_close(struct inode *filp, struct file *files)
{
 	keyirq_s *dev = (keyirq_s *)(files->private_data);
 	dev->open_count=0;
	files->private_data=NULL;
	return 0;
}
static ssize_t pollkey_read (struct file *filp, char __user *buf, size_t count, loff_t *p_offset)
{
	int ret;
	 keyirq_s *dev = (keyirq_s *)(filp->private_data); //获得设备结构体指针
	DECLARE_WAITQUEUE(wait, current); //定义等待队列

	down(&dev->sem); //获得信号量
	add_wait_queue(&dev->r_wait, &wait); //进入读等待队列头

	/* 等待FIFO非空 */
	while (dev->current_len == 0)
	{
		if (filp->f_flags &O_NONBLOCK)
		{
		  ret =  - EAGAIN;
		  goto out;
		} 
		__set_current_state(TASK_INTERRUPTIBLE); //改变进程状态为睡眠
		up(&dev->sem);

		schedule(); //调度其他进程执行
		if (signal_pending(current))
		//如果是因为信号唤醒
		{
		  ret =  - ERESTARTSYS;
		  goto out2;
		}

		down(&dev->sem);
	}

	/* 拷贝到用户空间 */
	if (count > dev->current_len)
	count = dev->current_len;

	if (copy_to_user(buf, dev->mem, count))
	{
		ret =  - EFAULT;
		goto out;
	}
	else
	{
		memcpy(dev->mem, dev->mem + count, dev->current_len - count); //fifo数据前移
		dev->current_len -= count; //有效数据长度减少
		printk(KERN_INFO "read %d bytes(s),current_len:%d\n", count, dev->current_len);
		 
		wake_up_interruptible(&dev->w_wait); //唤醒写等待队列

		ret = count;
	}
	out: up(&dev->sem); //释放信号量
	out2:remove_wait_queue(&dev->w_wait, &wait); //从附属的等待队列头移除
	set_current_state(TASK_RUNNING);
	return ret;
}
static ssize_t pollkey_write (struct file *files, const char __user *buf, size_t len, loff_t *p_offset)
{
	return 0;
}
static unsigned int pollkey_poll(struct file *files, struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	 keyirq_s *dev =(keyirq_s *) (files->private_data); /*获得设备结构体指针*/

	down(&dev->sem);

	poll_wait(files, &dev->r_wait, wait);
	//poll_wait(files, &dev->w_wait, wait);  
	/*fifo非空*/
	if (dev->current_len != 0)
	{
		mask |= POLLIN | POLLRDNORM; /*标示数据可获得*/
	}
	/*fifo非满*/
	if (dev->current_len != BUF_LEN)
	{
		mask |= POLLOUT | POLLWRNORM; /*标示数据可写入*/
	}
	 
	up(&dev->sem);
	return mask;
}












static struct file_operations pollkey_fops={
	.owner=THIS_MODULE,
	.open=pollkey_open,
	.release=pollkey_close,
	.poll=pollkey_poll,
	.read=pollkey_read,
	.write=pollkey_write,
};
/*
static struct miscdevice pollkey_dev={
        .minor=MISC_DYNAMIC_MINOR,//次设备号有系统分配，主设备号misc固定为10
        .name=dev_name , //板子上dev目录下显示的名字
        .fops=&pollkey_fops,
       };
       */
static irqreturn_t interrupt9_fun(int irq,void *dev_id)
{
	int ret;
	keyirq_s *key_dev_p=(keyirq_s *)dev_id;
	printk("interrupt9_fun...\n");
	ret=down_trylock(&key_dev_p->sem);
	if(ret)
	{
		return IRQ_NONE;
	}

	if(key_dev_p->current_len>=BUF_LEN)
	{
		up(&key_dev_p->sem);
		return IRQ_NONE;
	}
	key_dev_p->mem[key_dev_p->current_len%BUF_LEN]='9';
	key_dev_p->current_len+=1;
	wake_up_interruptible(&key_dev_p->r_wait); //唤醒读等待队列
	up(&key_dev_p->sem);
	printk("interrupt9_leave...\n");
	return IRQ_HANDLED;


}
static irqreturn_t interrupt10_fun(int irq,void *dev_id)
{
	int ret;
	keyirq_s *key_dev_p=(keyirq_s *)dev_id;
	printk("interrupt10_fun...\n");
	ret=down_trylock(&key_dev_p->sem);
	if(ret)
	{
		return IRQ_NONE;
	}

	if(key_dev_p->current_len>=BUF_LEN)
	{
		up(&key_dev_p->sem);
		return IRQ_NONE;
	}
	key_dev_p->mem[key_dev_p->current_len%BUF_LEN]='a';
	key_dev_p->current_len+=1;
	wake_up_interruptible(&key_dev_p->r_wait); //唤醒读等待队列
	up(&key_dev_p->sem);
	printk("interrupt10_leave...\n");
	return IRQ_HANDLED;
}
static int pollkey_probe(struct platform_device *pdev)
{
	int ret,i;
       printk("gpio_read_probe enter!\n");
	keyirq_s *key_dev_p;
       key_dev_p=kmalloc(sizeof(*key_dev_p),GFP_KERNEL);
       if(IS_ERR(key_dev_p))
       {
       	return -1;
       }
       memset(key_dev_p,0,sizeof(*key_dev_p));
	for(i=0;i<GPIO_NUM;i++)
	{
		ret=gpio_request(led_gpios[i],"gpio");
		if(ret<0)
		{
			printk("gpio_request %d failed %x\n ",i,ret);
			goto exit0;
		}
		s3c_gpio_cfgpin(led_gpios[i],S3C_GPIO_INPUT);
		s3c_gpio_setpull(led_gpios[i],S3C_GPIO_PULL_NONE);
	}
       ret=request_irq(IRQ_EINT(9),interrupt9_fun,IRQF_TRIGGER_FALLING,"my_irq 9",key_dev_p); 
	if(ret<0)
	{
		printk("gpio_irq_request %d failed %x\n ",9,ret);
		goto exit0;
	}
	ret=request_irq(IRQ_EINT(10),interrupt10_fun,IRQF_TRIGGER_FALLING,"my_irq 10",key_dev_p); 
	if(ret<0)
	{
		printk("gpio_irq_request %d failed %x\n ",10,ret);
		goto exit1;
	}
	key_dev_p->cdev.minor=MISC_DYNAMIC_MINOR;
	key_dev_p->cdev.name=dev_name;
	key_dev_p->cdev.fops=&pollkey_fops;
	//ret = misc_register(&pollkey_dev);
	ret = misc_register(&key_dev_p->cdev);
	if(ret<0)
	{
		printk("misc_register failed!\n");
		goto exit2;
	}
	strncpy(key_dev_p->name,dev_name,strlen(dev_name));
	init_waitqueue_head(&key_dev_p->r_wait);
	init_waitqueue_head(&key_dev_p->w_wait);
	sema_init(&key_dev_p->sem,1);
	key_dev_p->current_len=0;
	key_dev_p->open_count=0;
	platform_set_drvdata(pdev, key_dev_p);
	printk("%s probe ok\n",key_dev_p->name);
	return ret;
exit2:
	free_irq(IRQ_EINT(10),pdev);
exit1:
	free_irq(IRQ_EINT(9),pdev);
exit0:
	for(i=0;i<GPIO_NUM;i++)
	{
		gpio_free(led_gpios[i]);
	}
exit00:
	kfree(key_dev_p);
	return ret;
}

static int pollkey_remove (struct platform_device *pdev)
{
	int i;
	keyirq_s *p_data=platform_get_drvdata(pdev);
	printk("%s.remove..\n",p_data->name);
	misc_deregister(&p_data->cdev);
	
	free_irq(IRQ_EINT(10),p_data);
	free_irq(IRQ_EINT(9),p_data);
	for(i=0;i<GPIO_NUM;i++)
	{
		gpio_free(led_gpios[i]);
	}
	kfree(p_data);
	platform_set_drvdata(pdev, NULL);
	return 0;
}









static struct platform_driver pollkey_driver = {
	.driver = {
                .name = drive_name,
                .owner = THIS_MODULE,
        },
        .probe = pollkey_probe,
        .remove = pollkey_remove, 
};

static int pollkey_init(void)
{
	int ret;
	printk(KERN_EMERG "pollkey driver enter\n");
	ret=platform_driver_register(&pollkey_driver);
	if(ret<0)
	{
		printk("read ko error");
	}	
	return ret;
}
static int pollkey_exit(void)
{
	platform_driver_unregister(&pollkey_driver);
	return 0;
}
module_init(pollkey_init);
module_exit(pollkey_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");

