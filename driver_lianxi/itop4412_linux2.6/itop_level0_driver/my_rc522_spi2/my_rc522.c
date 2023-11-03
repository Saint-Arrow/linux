#include"../my_driver.h"
#include "spidev_test.h"

static struct file_operations spi_ops;
static struct miscdevice spi_dev;
struct spi_driver  spi_driver_test;
struct spi_device *my_spi;
#define RC522_RESET_PIN	EXYNOS4_GPK1(0)
void my_rc522_reset()
{
	//printk("************************ %s\n", __FUNCTION__);
	if(gpio_request_one(RC522_RESET_PIN, GPIOF_OUT_INIT_HIGH, "RC522_RESET"))
                pr_err("failed to request GPK1_0 for RC522 reset control\n");

        s3c_gpio_setpull(RC522_RESET_PIN, S3C_GPIO_PULL_UP);
        gpio_set_value(RC522_RESET_PIN, 0);

        mdelay(5);

        gpio_set_value(RC522_RESET_PIN, 1);
        gpio_free(RC522_RESET_PIN);
}
static int write_test(unsigned char *buffer, int len)
{
	int status;
	struct spi_transfer	t = {
		.tx_buf		= buffer,
		.len		= len,
	};
	struct spi_message	m;
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	DECLARE_COMPLETION_ONSTACK(done);
	m.complete = complete;
	m.context = &done;
	
	printk("spi_async send begin!\n");
	status = spi_async(my_spi,&m);
	if(status == 0){
		wait_for_completion(&done);
		status = m.status;
		if (status == 0)
			status = m.actual_length;
	}
	return status;
}

static int read_test(unsigned char *buffer, int len)
{
	int status;
	struct spi_transfer	t = {
		.rx_buf		= buffer,
		.len		= len,
	};
	struct spi_message	m;
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	DECLARE_COMPLETION_ONSTACK(done);
	m.complete = complete;
	m.context = &done;
	
	printk("spi_async read begin!\n");
	status = spi_async(my_spi,&m);
	if(status == 0){
		wait_for_completion(&done);
		status = m.status;
		if (status == 0)
			status = m.actual_length;
	}
	return status;
}
static unsigned char ReadRawRC(int addr)
{
	int ret;
	unsigned char  ReData;
	unsigned char  Address;
	
	Address  = (unsigned char)addr << 1;
	Address |= (1 << 7);
	Address &= ~(1 << 0);
	
	ret = write_test(&Address, 1);
	if (ret < 0)
		printk("spi:SPI Write error\n");

	udelay(100);

	ret = read_test(&ReData, 1);
	if (ret < 0)
		printk("spi:SPI Read error\n");

	return ReData;
}

static int WriteRawRC(int addr, int data)
{
	int ret;
	unsigned char  TxBuf[2];

	//bit7:MSB=0,bit6~1:addr,bit0:RFU=0
	TxBuf[0] = ((unsigned char)addr << 1)&0x7E;
	//TxBuf[0] &= 0x7E;

	TxBuf[1] = (unsigned char)data;
	
	ret = write_test(TxBuf, 2);
	
	if (ret < 0)
		printk("spi:SPI Write error\n");

	udelay(10);

	return ret;
}
static int rc522_init()
{
	int ret;
	char version = 0;

	//reset
	WriteRawRC(CommandReg, PCD_RESETPHASE);
	udelay(10);
	WriteRawRC(ModeReg, 0x3D);
	WriteRawRC(TReloadRegL, 30);
	WriteRawRC(TReloadRegH, 0);
	WriteRawRC(TModeReg, 0x8D);
	WriteRawRC(TPrescalerReg, 0x3E);

	version = ReadRawRC(VersionReg);
	printk("Chip Version: 0x%x\n", version);

	return 0;
}
static int spi_open_func(struct file *filp)
{
	return 0;
}

static int spi_release_func(struct file *filp)
{
	return 0;
}

static ssize_t spi_read_func(struct file *filp, char __user *buffer, size_t count, loff_t *ppos){

}

static ssize_t spi_write_func(struct file *filp, char __user *buffer, size_t count, loff_t *ppos){

}
static struct file_operations spi_ops = {
	.owner 	= THIS_MODULE,
	.open 	= spi_open_func,
	.release= spi_release_func,
	.write  = spi_write_func,
	.read 	= spi_read_func,
};


static struct miscdevice spi_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.fops	= &spi_ops,
	.name	= "rc522",
};
int test_probe(struct spi_client *spi_client_p, const struct spi_device_id *spi_device_id_p)
{
	int ret;
	printk(KERN_EMERG "test_probe enter\n");
	my_spi=spi_client_p;
	my_rc522_reset();
	rc522_init();
	misc_register(&spi_dev);
	return 0;
}
int test_remove(struct spi_client *spi_client_p)
{
	printk(KERN_EMERG "test_remove enter\n");
}

struct spi_driver  spi_driver_test={
	.probe=test_probe,
	.remove=test_remove,
	.driver	= {
		.name	= "my_rc522",
		.owner	= THIS_MODULE,
	},
};
static void spi_io_init(void)
{
	int ret;

	return 0;
}
static int spi_init(void)
{
	int ret;
	printk(KERN_EMERG "spi_init enter\n");
	spi_io_init();
	ret=spi_register_driver(&spi_driver_test);
	if(ret<0)
	{
		printk(KERN_EMERG "spi_add_driver error\n");
		return ret;
	}
}
static int spi_exit(void)
{
	printk(KERN_EMERG "spi_exit exit\n");

	spi_unregister_driver(&spi_driver_test);
}

module_init(spi_init);
module_exit(spi_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");
