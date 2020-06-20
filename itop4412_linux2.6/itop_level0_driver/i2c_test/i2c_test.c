#include"../my_driver.h"

unsigned char test_buf[3];

int test_probe(struct i2c_client *i2c_client_p, const struct i2c_device_id *i2c_device_id_p)
{
	int ret;
	test_buf[0]=0x01;
	test_buf[1]=0xff;
	test_buf[2]=0xff;
	struct i2c_msg i2c_msg_test[]={
		{
		.addr=0x1a,
		.flags=0,//w
		.len=1,
		.buf=&test_buf[0],
		},
		{
		.addr=0x1a,
		.flags=1,//r
		.len=2,
		.buf=&test_buf[1],
		},
	};
	printk(KERN_EMERG "test_probe enter\n");
	ret=i2c_transfer(i2c_client_p->adapter,i2c_msg_test,3);
	if(ret<0)
	{
		printk(KERN_EMERG "test_probe i2c_transfer error %d\n",ret);
		return ret;
	}
	else
	{
		printk(KERN_EMERG "test_probe i2c_transfer success ,%d,%d\n",test_buf[1],test_buf[2]);
		return 0;
	}
}
int test_remove(struct i2c_client *i2c_client_p)
{
	printk(KERN_EMERG "test_remove enter\n");
}
int test_command(struct i2c_client *i2c_client_p, unsigned int cmd, void *arg)
{
	int ret;
	printk(KERN_EMERG "test_command enter\n");
	struct i2c_msg i2c_msg_test[]={
		{
		.addr=0x1a,
		.flags=0,//w
		.len=1,
		.buf=&test_buf[0],
		},
		{
		.addr=0x1a,
		.flags=1,//r
		.len=2,
		.buf=&test_buf[1],
		},
	};
	test_buf[0]=arg;
	if(cmd)
	{
		i2c_msg_test[1].flags=1;
	}
	else
	{
		i2c_msg_test[1].flags=0;
	}
	test_buf[1]=0xff;
	test_buf[2]=0xff;
	ret=i2c_transfer(i2c_client_p->adapter,i2c_msg_test,3);
	if(ret<0)
	{
		printk(KERN_EMERG "test_command i2c_transfer error\n");
		return ret;
	}
	else
	{
		printk(KERN_EMERG "test_command i2c_transfer success\n");
		return test_buf[1];
	}
	 
}
static const struct i2c_device_id i2c_test_id[] = {
	{ "i2c_test", 0 },
	{ },
};
struct i2c_driver  i2c_driver_test={
	.probe=test_probe,
	.remove=test_remove,
	.command=test_command,
	.id_table	= i2c_test_id,
	.driver	= {
		.name	= "i2c_test",
		.owner	= THIS_MODULE,
	},
};
static void i2c_io_init(void)
{
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
