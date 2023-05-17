#include"../my_driver.h"

unsigned char test_buf[3];
static struct file_operations i2c_ops;
static struct miscdevice i2c_dev;
struct i2c_driver  i2c_driver_test;
static struct i2c_client *this_client;

static int i2c_open_func(struct file *filp)
{
	return 0;
}

static int i2c_release_func(struct file *filp)
{
	return 0;
}

static ssize_t i2c_read_func(struct file *filp, char __user *buffer, size_t count, loff_t *ppos){
	int ret;
	u8 reg_data;
	
	ret = copy_from_user(&reg_data,buffer,1);
	
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,	//0x38
			.flags	= 0,	//写
			.len	= 1,	//要写的数据的长度
			.buf	= &reg_data,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= &reg_data,
		},
	};
	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0) {
		pr_err("read reg error!\n");
	}
	ret = copy_to_user(buffer,&reg_data,1);
	
	return ret;
}

static ssize_t i2c_write_func(struct file *filp, char __user *buffer, size_t count, loff_t *ppos){
	int ret;
	u8 buf[2];
	struct i2c_msg msgs[1];
	
	ret = copy_from_user(&buf,buffer,2);
	
	msgs[0].addr	= this_client->addr;	//0x38
	msgs[0].flags	= 0;	//写
	msgs[0].len	= 2;	//第一个是要写的寄存器地址，第二个是要写的内容
	msgs[0].buf	= buf;

	ret = i2c_transfer(this_client->adapter, msgs, 1);
	if (ret < 0) {
		pr_err("write reg 0x%02x error!\n",buf[0]);
	}
	ret = copy_to_user(buffer,buf,1);
	
	return ret;
}
static struct file_operations i2c_ops = {
	.owner 	= THIS_MODULE,
	.open 	= i2c_open_func,
	.release= i2c_release_func,
	.write  = i2c_write_func,
	.read 	= i2c_read_func,
};


static struct miscdevice i2c_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.fops	= &i2c_ops,
	.name	= "i2c_control",
};
int test_probe(struct i2c_client *i2c_client_p, const struct i2c_device_id *i2c_device_id_p)
{
	int ret;
	test_buf[0]=0xa6;
	test_buf[1]=0xff;
	test_buf[2]=0xff;
	struct i2c_msg i2c_msg_test[]={
		{
		.addr=i2c_client_p->addr,
		.flags=0,//w
		.len=1,
		.buf=&test_buf[0],
		},
		{
		.addr=i2c_client_p->addr,
		.flags=1,//r
		.len=1,
		.buf=&test_buf[1],
		},
	};
	printk(KERN_EMERG "test_probe enter ,0x%x\n",i2c_client_p->addr);
	ret=i2c_transfer(i2c_client_p->adapter,i2c_msg_test,2);
	if(ret<0)
	{
		printk(KERN_EMERG "test_probe i2c_transfer error %d\n",ret);
		return ret;
	}
	else
	{
		printk(KERN_EMERG "test_probe i2c_transfer success ,%d,%d\n",test_buf[1],test_buf[2]);
	}
	this_client=i2c_client_p;
	misc_register(&i2c_dev);
	return 0;
}
int test_remove(struct i2c_client *i2c_client_p)
{
	printk(KERN_EMERG "test_remove enter\n");
}

static const struct i2c_device_id i2c_test_id[] = {
	{ "i2c_test", 0 },
	{ },
};
struct i2c_driver  i2c_driver_test={
	.probe=test_probe,
	.remove=test_remove,
	.id_table	= i2c_test_id,
	.driver	= {
		.name	= "i2c_test",
		.owner	= THIS_MODULE,
	},
};
static void i2c_io_init(void)
{
	int ret;
	ret = gpio_request(EXYNOS4_GPL0(2), "TP1_EN");
	if (ret) {
		printk(KERN_ERR "failed to request TP1_EN for "
				"I2C control\n");
		//return err;
	}
	gpio_direction_output(EXYNOS4_GPL0(2), 1);
	s3c_gpio_cfgpin(EXYNOS4_GPL0(2), S3C_GPIO_OUTPUT);
	gpio_free(EXYNOS4_GPL0(2));
	mdelay(5);
	return 0;
}
static int i2c_init(void)
{
	int ret;
	printk(KERN_EMERG "i2c_init enter\n");
	i2c_io_init();
	ret=i2c_add_driver(&i2c_driver_test);
	if(ret<0)
	{
		printk(KERN_EMERG "i2c_add_driver error\n");
		return ret;
	}
}
static int i2c_exit(void)
{
	printk(KERN_EMERG "i2c_exit exit\n");

	i2c_del_driver(&i2c_driver_test);
}

module_init(i2c_init);
module_exit(i2c_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");
