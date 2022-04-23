#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>


#include <linux/time.h>
//struct timeval volatile  tstart,tsend;
struct timespec tstart,tsend;
unsigned long long del_us;
#define DEVICE_NAME  "dht11"

#if 0
/*三星平台的GPIO配置函数头文件*/
/*三星平台EXYNOS系列平台，GPIO配置参数宏定义头文件*/
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
/*三星平台4412平台，GPIO宏定义头文件*/
#include <mach/gpio-exynos4.h>

#define DATE_IO EXYNOS4_GPC1(1) 
#define DATE_IO_OUT  s3c_gpio_cfgpin(DATE_IO,S3C_GPIO_OUTPUT)
#define DATE_IO_IN  s3c_gpio_cfgpin(DATE_IO,S3C_GPIO_INPUT)
#endif

#if 1
/*RV1126 平台 GPIO1_B6*/
#define DATE_IO 46
#define DATE_IO_OUT gpio_direction_output(DATE_IO,1);
#define DATE_IO_IN gpio_direction_input(DATE_IO);
#endif


#define DATE_IO_SET    gpio_set_value(DATE_IO,1)
#define DATE_IO_CLR    gpio_set_value(DATE_IO,0)
#define DATE_IO_VALUE  gpio_get_value(DATE_IO)

//typedef unsigned short int u16;
//typedef unsigned char u8;
#define delay_us(x)		udelay(x)
#define  READ_DATE  _IOR('D', 1,struct dht11_date)

#if 1
#define IRQ_DISABLE     
#define IRQ_ENABLE	
//#define IRQ_DISABLE     local_irq_disable()
//#define IRQ_ENABLE		local_irq_enable()
#else
unsigned long flags;
#define IRQ_DISABLE     local_irq_save(flags)
#define IRQ_ENABLE		local_irq_restore(flags)
#endif
struct dht11_date{
	u16 temp;  //温度
	u16 hum;   //湿度
};

int wait_val_msecs(int value,int msec)
{
	int ret = 0;
	unsigned long int cur;
	cur = jiffies;
	while(DATE_IO_VALUE != value)
	{
		
		if(time_after_eq(jiffies,cur+msecs_to_jiffies(msec)))
		{
			ret = -1;
			DATE_IO_OUT
			break;
		}
	}
	return ret;
}
int double_read(int value )
{
	int ret=-1;
	if(DATE_IO_VALUE == value)
	{
		udelay(1);
		if(DATE_IO_VALUE == value)
		{
			ret=0;
		}
		else
		{
			//printk("ddouble_read error");
		}
	}
	return ret;
}	
unsigned long long time_data[100];
char count=0;
u8 Read_Dat(void)
{ 
		u8 ret;
		u8 i,j,retry; 
		u8 Dat = 0x00; 
		
		
		for( j = 0 ; j < 8 ; j++ ) 
		{  
			retry=0;
			while((double_read(1)==0)&&retry<150)
			{
				retry++;
				udelay(1);
			}
			#if 0
		    //tstart=current_kernel_time();
			ktime_get_ts(&tstart);
			#endif
			retry=0;
			while((double_read(0)==0)&&retry<100)
			{
				retry++;
				udelay(1);
			}
			Dat <<= 1; 
			#if 0
			//tsend=current_kernel_time();
			//time_data[count]=1000*1000*(tsend.tv_sec-tstart.tv_sec)+(tsend.tv_usec-tstart.tv_usec);
			ktime_get_ts(&tsend);
			time_data[count]=1000*1000*(tsend.tv_sec-tstart.tv_sec)+(tsend.tv_nsec-tstart.tv_nsec)/1000;
			count++;
			#else
			udelay(28);			
			if( DATE_IO_VALUE ) 
			{   
				Dat |= 1;  
			}  
			#endif

		
		} 
		
		
		return( Dat );   
}

int fail_mark=0;
int read_form_dht11(struct dht11_date *date)
{
	
	u16 Dat;
	int ret,i,retry;
	
	
	//湿度整数，湿度小数，温度整数，温度小数 , 校验值
	u8 SD_z , SD_x , WD_z , WD_x , JY;  
	DATE_IO_OUT;
	DATE_IO_CLR;          

	msleep(23); 

	DATE_IO_SET;   
	udelay(30);
	DATE_IO_IN;
	fail_mark=0;
	retry=0;
	while(((double_read(1)==0))&&retry<100)
	{
		retry++;
		delay_us(3);
	}
	if(retry>=100)
	{
		printk(KERN_EMERG"humidity_read_data %d err!\n",__LINE__);
		fail_mark=1;
		return -1;
	}
	
	retry=0;
	while(((double_read(0)==0))&&retry<100)
	{
		retry++;
		delay_us(3);
	}
	if(retry>=100)
	{
		printk(KERN_EMERG"humidity_read_data %d err!\n",__LINE__);
		return -1;
	}
	
	retry=0;
	while(((double_read(1)==0))&&retry<100)
	{
		retry++;
		delay_us(3);
	}
	if(retry>=100)
	{
		printk(KERN_EMERG"humidity_read_data %d err!\n",__LINE__);
		return -1;
	}
	
	count=0;
	for(i=0;i<40;i++)
	{
		time_data[i]=0;
	}
	SD_z = Read_Dat(); 
	SD_x = Read_Dat(); 
	WD_z = Read_Dat(); 
	WD_x = Read_Dat(); 
	JY = Read_Dat(); //校验  
	printk(KERN_EMERG"DATA:0x%x %x %x %x,jy:%x\n",SD_z,SD_x,WD_z,WD_x,JY);
	if( JY == ( SD_z + SD_x + WD_z + WD_x )  ) 
	{
		 date->hum  = SD_z<<8 | (SD_x&&0xff);
		 date->temp = WD_z<<8 | (WD_x&&0xff);
		ret = 0;
	}
	else
	{
		Dat = 0; 
		printk(KERN_EMERG"JY IS FAIL\n");
		ret = -1;
	}
	#if  0
	for(i=0;i<40;i+=8)
	{
		printk("%lld %lld %lld %lld %lld %lld %lld %lld \n",time_data[i+0],time_data[i+1],time_data[i+2],time_data[i+3],time_data[i+4],time_data[i+5],time_data[i+6],time_data[i+7]);
	}
	#endif
	DATE_IO_OUT;
	DATE_IO_SET;
	return ret;
}



static long dht11_ioctl (struct file *filp, unsigned int cmd, unsigned long args)
{
	int ret;
	struct dht11_date cur_date = {0x0,0x0};
	
	switch(cmd)
	{
		case READ_DATE:
			IRQ_DISABLE;
			ret = read_form_dht11(&cur_date);
			IRQ_ENABLE;
			if(ret == 0 ) 
			{
				ret = copy_to_user((void *)args,&cur_date,sizeof(cur_date));
				if(ret) printk(KERN_EMERG"address error !!\n");
			}	
			break;
		default:
			ret = -EPERM;
			printk(KERN_EMERG"cmd is no find!!\n");
			break;
	}
	return ret;
	
}


static const struct file_operations dht11_fops = {
 	.owner 			= THIS_MODULE,
	.unlocked_ioctl = dht11_ioctl,
};


static  struct miscdevice  dht11_drv = {
		.minor =	MISC_DYNAMIC_MINOR,
		.name = 	DEVICE_NAME,
		.fops = 	&dht11_fops,
};


static int __init  dht11_init(void)
{
	int ret;
	//current_kernel_time 
	ktime_get_ts(&tstart);
	//ndelay(2);
	ktime_get_ts(&tsend);
	time_data[0]=1000*1000*(tsend.tv_sec-tstart.tv_sec)+(tsend.tv_nsec-tstart.tv_nsec)/1000;
	printk(KERN_EMERG"HZ:%d min del time: %d !\n",HZ,time_data[0]);
	
	ret = gpio_request(DATE_IO,"signal");
	if(ret < 0)	
	{
		printk(KERN_EMERG"Pins may not apply 0x%x!\n",ret);
		gpio_free(DATE_IO);
		ret = gpio_request(DATE_IO,"signal");
		if(ret < 0)
		{
			printk(KERN_EMERG"Pins may not apply again 0x%x!\n",ret);
			return ret;
		}
	}
	
		
	
    ret = misc_register(&dht11_drv);
	if (ret) 
	{
		printk(KERN_EMERG"misc_register %s fail\n",DEVICE_NAME);
		return ret;
	}
	
	DATE_IO_OUT;
	DATE_IO_SET;
    return ret;
}


static void __exit dht11_exit(void)
{
	gpio_free(DATE_IO);
	misc_deregister(&dht11_drv);
}


module_init(dht11_init);
module_exit(dht11_exit);


MODULE_DESCRIPTION("fbdev driver for Broadsheet controller");
MODULE_AUTHOR("Jaya Kumar");
MODULE_LICENSE("GPL");





