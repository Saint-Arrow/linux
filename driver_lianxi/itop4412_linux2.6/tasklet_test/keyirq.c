#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>

#define drive_name "cwj_gpio-keys"
#define dev_name "pollkey"
#define NR_TIMEVAL 512 /* length of the array of time values */

struct timeval tv_data[NR_TIMEVAL]; /* too lazy to allocate it*/ /*用于tasklet的另一个circular buffer，用于临时存储中间数据.*/
volatile struct timeval *tv_head = tv_data;
volatile struct timeval *tv_tail = tv_data;
int short_wq_count = 0;

unsigned long short_buffer = 0;
unsigned long volatile short_head;
volatile unsigned long short_tail;

/*
 * Atomicly increment an index into short_buffer
 */
static inline void short_incr_bp(volatile unsigned long *index, int delta)
{
    unsigned long new = *index + delta;

   barrier();  /* Don't optimize these two together */
   *index = (new >= (short_buffer + PAGE_SIZE)) ? short_buffer : new;
}
/*
 * Increment a circular buffer pointer in a way that nobody sees
 * an intermediate value.
 */
static inline void short_incr_tv(volatile struct timeval **tvp)
{
    if (*tvp == (tv_data + NR_TIMEVAL - 1))
    {
       *tvp = tv_data;
   }                   /* Wrap */
    else
    {
       (*tvp)++;
    }
}
void tasklet_test(unsigned long data)
{
  pr_err("%s[%d] data=%d\n",__func__, __LINE__,data);
  int savecount = short_wq_count, written;

   short_wq_count = 0; /* we have already been removed from the queue */
    /*
    * The bottom half reads the tv array, filled by the top half,
    * and prints it to the circular text buffer, which is then consumed
    * by reading processes
    */

    /* First write the number of interrupts that occurred before this bh */
    while(tv_tail!=tv_head)
    {
    pr_err("%u.%u,savecount=%d\n",(int)(tv_tail->tv_sec % 100000000),
                         (int)(tv_tail->tv_usec),savecount);
    short_incr_tv(&tv_tail);
    }
}
DECLARE_TASKLET(tasklet_test_s,tasklet_test,0);
static int led_gpios[]={
	EXYNOS4_GPX1(1),
	EXYNOS4_GPX1(2),
};
#define GPIO_NUM ARRAY_SIZE(led_gpios)

static struct platform_driver pollkey_driver;

static irqreturn_t interrupt9_fun(int irq,void *dev_id)
{
	pr_err("%s[%d] \n",__func__, __LINE__);
  do_gettimeofday((struct timeval *)tv_head);
  short_incr_tv(&tv_head);
  tasklet_schedule(&tasklet_test_s);
  short_wq_count++;
  return IRQ_HANDLED;
}
static int pollkey_probe(struct platform_device *pdev)
{
	int ret,i;
       pr_err("%s[%d] \n",__func__, __LINE__);
	for(i=0;i<GPIO_NUM;i++)
	{
		ret=gpio_request(led_gpios[i],"gpio");
		if(ret<0)
		{
			printk("gpio_request %d failed %x\n ",i,ret);
			return ret;
		}
		s3c_gpio_cfgpin(led_gpios[i],S3C_GPIO_INPUT);
		s3c_gpio_setpull(led_gpios[i],S3C_GPIO_PULL_NONE);
	}
  ret=request_irq(IRQ_EINT(9),interrupt9_fun,IRQF_TRIGGER_FALLING,"my_irq 9",pdev); 
	if(ret<0)
	{
		pr_err("%s[%d] \n",__func__, __LINE__);
	}
	ret=request_irq(IRQ_EINT(10),interrupt9_fun,IRQF_TRIGGER_FALLING,"my_irq 10",pdev); 
	if(ret<0)
	{
		pr_err("%s[%d] \n",__func__, __LINE__);
	}
	return ret;
}

static int pollkey_remove (struct platform_device *pdev)
{
	int i;
	for(i=0;i<GPIO_NUM;i++)
	{
		gpio_free(led_gpios[i]);
	}
	free_irq(IRQ_EINT(9),pdev);
	free_irq(IRQ_EINT(10),pdev);
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
	pr_err("%s[%d] \n",__func__, __LINE__);
	ret=platform_driver_register(&pollkey_driver);
	if(ret<0)
	{
		pr_err("%s[%d] \n",__func__, __LINE__);
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

