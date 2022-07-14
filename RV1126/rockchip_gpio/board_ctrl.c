#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/interrupt.h>
#include "board_ctrl.h"
//#include <mach/io.h>
#include <linux/gpio.h>


int board_open(struct inode *inode, struct file *filp);
int board_release(struct inode *inode, struct file *filp);
long  board_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);


int gpio=5;
module_param(gpio, int , 0644);
MODULE_PARM_DESC(gpio, "gpio_id");
//struct gpio_desc *gpio=NULL;

#if 0
int wait_val_msecs(int value,int msec)
{
	int ret = 0;
	unsigned long int cur;
	cur = jiffies;
	while(gpio_get_value(gpio) != value)
	{
		
		if(time_after_eq(jiffies,cur+msecs_to_jiffies(msec)))
		{
			ret = -1;
			break;
		}
	}
	return ret;
}
int wait_val_usecs(int value,int usec)
{
	int ret = 0;
	unsigned long int cur;
	cur = jiffies;
	while(gpio_get_value(gpio) != value)
	{
		
		if(time_after_eq(jiffies,cur+usecs_to_jiffies(usec)))
		{
			ret = -1;
			break;
		}
	}
	return ret;
}
#endif
struct file_operations board_fops = {
	.owner =    THIS_MODULE,
	.unlocked_ioctl =    board_ioctl,
	.open =     board_open,
	.release =  board_release,
};

static struct miscdevice boardctrl_dev =
{
    .minor   = MISC_DYNAMIC_MINOR,
    .name    = "encrypt_detect",
    .fops    = &board_fops,
};



int board_open(struct inode *inode, struct file *filp)
{
	return 0;          /* success */
}

int board_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * The ioctl() implementation
 */
long  board_ioctl(struct file *filp,  unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	int i=0;
	unsigned char reg_val=0;
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	//printk("cmd  %x\n", cmd);
	if (_IOC_TYPE(cmd) != BOARD_IOC_MAGIC) {
		return -ENOTTY;
	}
	
	switch(cmd) {
		case GPIO_DOWN:
			retval=gpio_direction_output(gpio,0);
			break;
		case GPIO_UP:
			retval=gpio_direction_output(gpio,1);
			break;
		case GPIO_GET_DATA:
			*((__user uint *)arg)=gpio_get_value(gpio);
			break;
		case GPIO_SET_INPUT:
			retval =gpio_direction_input(gpio);
			break;
		case GPIO_RESET:
			gpio_direction_output(gpio,0);
			udelay(500);
			gpio_direction_input(gpio);
			udelay(70);
			reg_val=gpio_get_value(gpio);	
			udelay(500);
			*((__user uint *)arg)=reg_val;
			break;
		case GPIO_W:
			reg_val=*((__user uint *)arg);
			//printk("reg  0x%x\n", reg_val);
			gpio_direction_output(gpio,1);
			udelay(1);
			for(i=0;i<8;i++)
			{
				gpio_set_value(gpio,0);
				ndelay(1800);
				if((reg_val&0x01)==0)
				{
					udelay(60);
				}	
				gpio_set_value(gpio,1);
				udelay(60);
				reg_val>>=1;
			}
			gpio_direction_input(gpio);
			udelay(1);
			break;
		case GPIO_R:
			gpio_direction_output(gpio,1);
			udelay(1);
			for(i=0;i<8;i++)
			{
				gpio_direction_output(gpio,0);
				ndelay(1800);
				gpio_direction_input(gpio);
				udelay(10);
				reg_val>>=1;
				if(gpio_get_value(gpio))
				{
					reg_val|=0x80;
				}
				udelay(60);
				/*连续读写需要1us的recovery时间*/
				gpio_direction_output(gpio,1);
				udelay(1);
			}
			gpio_direction_input(gpio);
			udelay(1);
			//printk("reg  0x%x\n", reg_val);
			*((__user uint *)arg)=reg_val;
		default:  
			retval = -ENOSYS;
	}
	return retval;
}

#define IRQ_TEST

#ifdef IRQ_TEST
static int motor_pt_dev_id = 10000;
volatile unsigned int irq_flag;
static irqreturn_t motor_pt_irq_handler(int irq, void *dev_id)
{
	int ret = 0;
	
    printk("[%s %d]\n", __func__, __LINE__);
	
	
    return 0;
}
#endif

 int board_init(void)
{
	int     ret;
    
	printk("gpio_index:%d\n",gpio);
	if (!gpio_is_valid(gpio)){
        printk("invalid gpio_index: %d\n",gpio);
        return -1;
    }
    if (gpio_request(gpio, "gpio_index")) {
        printk("gpio %d request failed!\n",gpio);
        return ret;
    }
    
    #if 1
    gpio_direction_input(gpio);
    udelay(70);
    int reg_val=gpio_get_value(gpio);	
    printk("gpio %d value %d \n",gpio,reg_val);
    #endif
    
    #ifdef IRQ_TEST
	int vd_gpio_irq;
	vd_gpio_irq = gpio_to_irq(gpio);
    printk("gpio %d ,vd_gpio_irq %d\n",gpio,gpio);
    ret = request_irq(vd_gpio_irq,motor_pt_irq_handler,IRQF_TRIGGER_RISING|IRQF_SHARED,"gpio_index",(void *)&motor_pt_dev_id);
	if(ret != 0)
	{
	 	printk("request_irq failure\n");
		return -1;
	}
    #endif
    
	ret = misc_register(&boardctrl_dev);
    if (ret != 0) {
        printk("register i2c device failed with %#x!\n", ret);
        return -1;
    }
	return 0;
}

void board_cleanup(void)
{
    #ifdef IRQ_TEST
    int vd_gpio_irq;
	vd_gpio_irq = gpio_to_irq(gpio);
    free_irq(vd_gpio_irq, &motor_pt_dev_id);
    #endif
	gpio_free(gpio);
	misc_deregister(&boardctrl_dev);
}

module_init(board_init);
module_exit(board_cleanup);
MODULE_LICENSE("GPL");
