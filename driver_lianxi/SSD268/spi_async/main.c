/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>

#define CS_PIN 46

struct spi_device *spi_device = NULL; 
struct spi_master*	 master= NULL;

#define MAX_TRANSFER 20
struct spi_message *test_async_msg=NULL;
unsigned char  tx_buf[MAX_TRANSFER][8] = {0};
unsigned char  rx_buf[MAX_TRANSFER][8] = {0};
int transfer_index=0;



//#define KEY_PIN 79






int ms41969_spi_read(unsigned int addr);
static DECLARE_COMPLETION(spi_completion);
struct task_struct *spi_completion_ctl_task_p = NULL; 
int spi_completion_ctl_thread(void *arg)
{
    int ret=0;
    do
    {
        ret=wait_for_completion_killable(&spi_completion);
        printk("spi transfer done or killed 0x%x 0x%x\n",ret,spi_completion.done);
        ms41969_spi_read(0x4034);

        if(kthread_should_stop())
        {    
            break;
        }   
    }while(1);
    return 0;
    
}
void test_complete(void *arg)
{
	printk("spi transfer done %ld\n",in_interrupt());
    
}

void ms41969_spi_init(void)
{

    int ret = 0;
    struct device *dev = NULL;

    dev = bus_find_device_by_name(&spi_bus_type, NULL, "spi0.0");
    if (dev == NULL) {
        pr_err("Failed to register  driver : %d \n", ret);
    }

    spi_device = to_spi_device(dev);
    if (spi_device == NULL) {
        pr_err("Failed to register  driver : %d \n", ret);
    }

    spi_device->max_speed_hz = 2000000; //2M
    spi_device->mode = SPI_MODE_3;//SPI_MODE_0 | SPI_CS_HIGH;
    spi_device->bits_per_word = 8;     //8位
    spi_device->cs_gpio = CS_PIN;  //CS

    ret = spi_setup(spi_device);
    
    master = spi_busnum_to_master(0);
    
    printk(KERN_ALERT"ms41969_spi driver enter 0x%x \n", ret);
    test_async_msg=spi_message_alloc(MAX_TRANSFER,0);
    
#if 1
    spi_completion_ctl_task_p = kthread_create(spi_completion_ctl_thread,NULL,"spi compele task");        
    if(!IS_ERR(spi_completion_ctl_task_p))
    {
           wake_up_process(spi_completion_ctl_task_p);//唤醒PTzero_task_p指向的线程
    } 
#endif

   
}
int ms41969_spi_read(unsigned int addr)
{	
    int ret = 0;
    unsigned char temp_tx_buf[8];
    unsigned char temp_rx_buf[8];
	struct spi_transfer t[] = {
        {
            .tx_buf = &temp_tx_buf[0],
            .rx_buf = &temp_rx_buf[0],
            .len = 5
        }, 
        
    };
    
    temp_tx_buf[0] = 0xA0;
	temp_tx_buf[1] = (addr >> 16) & 0xff;
	temp_tx_buf[2] = (addr >> 8) & 0xff;
	temp_tx_buf[3] = (addr >> 0) & 0xff;


#if 1
    ret = spi_sync_transfer(spi_device,t,1);
    if(0 != ret) 
    {
        pr_err("ms41969_spi_write error:0x%x",addr);
        return ret;
    }
#endif
    printk("read:0x%x 0x%x 0x%x 0x%x 0x%x\n",temp_rx_buf[0],temp_rx_buf[1],temp_rx_buf[2],temp_rx_buf[3],temp_rx_buf[4]);
	return ret;

}

int ms41969_spi_write(unsigned int addr, char data)
{	
    int ret = 0;
    struct spi_transfer *t_all = (struct spi_transfer *)(test_async_msg + 1);
	struct spi_transfer t[] = {
        {
            .tx_buf = &tx_buf[transfer_index],
            .rx_buf = &rx_buf[transfer_index],
            .len = 6
        }, 
        
    };
    
    tx_buf[transfer_index][0] = 0x20;
	tx_buf[transfer_index][1] = (addr >> 16) & 0xff;
	tx_buf[transfer_index][2] = (addr >> 8) & 0xff;
	tx_buf[transfer_index][3] = (addr >> 0) & 0xff;
	tx_buf[transfer_index][4] = data;
	tx_buf[transfer_index][5] = data;

    
    memcpy(&t_all[transfer_index],&t,sizeof(t));
    spi_message_add_tail(&t_all[transfer_index],test_async_msg);
    transfer_index++;
#if 0
    ret = spi_sync_transfer(spi_device,t,1);
    if(0 != ret) 
    {
        pr_err("ms41969_spi_write error:0x%x,w:0x%x",reg_ss,addr);
        return ret;
    }
#endif

	return ret;

}
void sync_test(void)
{
    transfer_index=0;
    spi_message_init(test_async_msg);
    ms41969_spi_write(0x4034,0x50);
    spi_sync(spi_device, test_async_msg);
}

int ms41969_spi_async_write(unsigned int addr, char data)
{
    int ret = 0;
    unsigned long		        flags;
    struct spi_transfer *t = (struct spi_transfer *)(test_async_msg + 1);
    /* check spi_message is or no finish */
   spin_lock_irqsave(&master->queue_lock, flags);
   if (test_async_msg->state != NULL)
   {
       spin_unlock_irqrestore(&master->queue_lock, flags);
       printk("error %s, %s, %d line: spi_message no finish!\n", __FILE__, __func__, __LINE__);
       return -EFAULT;
   }
   spin_unlock_irqrestore(&master->queue_lock, flags);


   
   transfer_index=0;
   spi_message_init(test_async_msg);

    tx_buf[transfer_index][0] = 0x20;
	tx_buf[transfer_index][1] = (addr >> 16) & 0xff;
	tx_buf[transfer_index][2] = (addr >> 8) & 0xff;
	tx_buf[transfer_index][3] = (addr >> 0) & 0xff;
	tx_buf[transfer_index][4] = data;
	tx_buf[transfer_index][5] = data;
    t[transfer_index].tx_buf=&tx_buf[transfer_index];
    t[transfer_index].rx_buf=&rx_buf[transfer_index];
    t[transfer_index].len = 6;

    
    spi_message_add_tail(&t[transfer_index],test_async_msg);
    test_async_msg->complete=test_complete;
    ret=spi_async(spi_device,test_async_msg);
    return 0;
}





#ifdef KEY_PIN
int num_irq_pls1 = 0; 
int cnt=0X50;
irqreturn_t pls1_gpio_irq_fun(int irq, void *dev_instance)
{

    printk("pls1_gpio_irq_fun enter,0x%x %ld\n",++cnt,in_interrupt());
    ms41969_spi_async_write(0X4034,cnt);

 
    
    complete(&spi_completion);
    return IRQ_HANDLED;
}
#else
struct hrtimer tilt_timer ;                              //高精度定时器，用于产生AB电机驱动同步信号
struct hrtimer pan_timer;                                //水平
#define NEW_COMMAND_EFFECTIVE_TIME_NS   2000000
static unsigned int pan_timeout = 0;
static int p_cnt=0;
enum hrtimer_restart Hrtimer_Pan_Timeout_Handler(struct hrtimer *t)
{
    enum hrtimer_restart hrt_returned;
	ktime_t kt_enter;
    p_cnt++;
    if(100==p_cnt)
    {
        p_cnt=0;
        //printk("do p \n");
        mdelay(2);
    }
    kt_enter = pan_timer.base->get_time(); 
    pan_timeout=NEW_COMMAND_EFFECTIVE_TIME_NS;
    hrtimer_forward(&pan_timer, kt_enter, ktime_set(0, pan_timeout));
    hrt_returned = HRTIMER_RESTART;
    return hrt_returned;
}
static unsigned int tilt_timeout = 0;
static int t_cnt=0;
enum hrtimer_restart Hrtimer_Tilt_Timeout_Handler(struct hrtimer *t)
{
    enum hrtimer_restart hrt_returned;
	ktime_t kt_enter;
    t_cnt++;
    if(101==t_cnt)
    {
        t_cnt=0;
        //printk("do p \n");
        mdelay(2);
    }
    kt_enter = tilt_timer.base->get_time();
    tilt_timeout=NEW_COMMAND_EFFECTIVE_TIME_NS;
    hrtimer_forward(&tilt_timer, kt_enter, ktime_set(0, tilt_timeout));
    hrt_returned = HRTIMER_RESTART;
    return hrt_returned;
}
#endif

int board_init(void)
{
    ms41969_spi_init();
    //sync_test();

    

    #ifdef KEY_PIN
    {
        int ret=0;
        //电机1监控信号
        ret = gpio_request(KEY_PIN, "KEY_PIN");
        //设置输入
        ret = gpio_direction_input(KEY_PIN);
      
        //获取 GPIO 的中断号
        num_irq_pls1 = gpio_to_irq(KEY_PIN);
        printk("gpio_to_irq(PLS1) =  %d\n", num_irq_pls1);
        //申请中断
        if(num_irq_pls1 > 0)
        {
            ret = request_irq(num_irq_pls1, pls1_gpio_irq_fun, IRQF_TRIGGER_FALLING, "inputkey_irq", NULL);//上升沿触发
            printk("request_irq(num_irq_pls1, pls1_gpio_irq_fun, IRQF_TRIGGER_RISING, DEV_MOTOR_PT_PATH, NULL) =  %d\n\n", ret);
        }
    }
    #else
    hrtimer_init(&pan_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pan_timer.function = Hrtimer_Pan_Timeout_Handler;
    hrtimer_init(&tilt_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tilt_timer.function = Hrtimer_Tilt_Timeout_Handler;

    hrtimer_start(&pan_timer, ktime_set(0, 1000), HRTIMER_MODE_REL);
    hrtimer_start(&tilt_timer, ktime_set(0, 1000), HRTIMER_MODE_REL);
    #endif
 
    



    
    pr_err("board_init\n");
    return 0;
}

void board_cleanup(void)
{
    #ifdef KEY_PIN
    free_irq(num_irq_pls1, NULL);
    gpio_free(KEY_PIN);

    
    spi_completion.done=-1;
    completion_done(&spi_completion);
    if(spi_completion_ctl_task_p)
    { 
        kthread_stop(spi_completion_ctl_task_p);
        spi_completion_ctl_task_p = NULL;
        printk(KERN_ALERT"pt_horizontal_ctl_task_p exit");
    }
    #else
    hrtimer_cancel(&tilt_timer);
	hrtimer_cancel(&pan_timer);
    #endif

    
    pr_err("board_cleanup\n");
}




module_init(board_init);
module_exit(board_cleanup);
MODULE_LICENSE("GPL");