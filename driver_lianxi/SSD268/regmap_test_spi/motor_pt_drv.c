/*KEY_WORD: Optocoupler Step Motor PTZ Control Driver MISC DEV*/
#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/fs.h>	
#include <linux/init.h>	
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>	
#include <asm/irq.h>	
#include <asm/io.h>
#include <linux/sched.h>	
#include <linux/interrupt.h>	
#include <linux/device.h>	
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/hrtimer.h>
#include <linux/time.h>

#include <linux/spi/spi.h>
#include <linux/regmap.h>

#include "TMC5041_Register.h"


#define DS3232_REG_SECONDS	0x00
#define DS3232_REG_MINUTES	0x01
#define DS3232_REG_HOURS	0x02
#define DS3232_REG_AMPM		0x02
#define DS3232_REG_DAY		0x03
#define DS3232_REG_DATE		0x04
#define DS3232_REG_MONTH	0x05
#define DS3232_REG_CENTURY	0x05
#define DS3232_REG_YEAR		0x06
#define DS3232_REG_ALARM1         0x07	/* Alarm 1 BASE */
#define DS3232_REG_ALARM2         0x0B	/* Alarm 2 BASE */
#define DS3232_REG_CR		0x0E	/* Control register */
#define DS3232_REG_SR	0x0F	/* control/status register */


#define H_MOTOR (1)
#define V_MOTOR (0)

struct regmap *tmc5041_regmap;


#if 0
struct bcm53xxspi {
	struct bcma_device *core;
	struct spi_master *master;
	void __iomem *mmio_base;

	size_t read_offset;
	bool bspi;				/* Boot SPI mode with memory mapping */
};
#endif



/****************************************************************************
* 名    称：void Motor_Spi_Write(u8 addr, unsigned int data)
* 功    能：写进一个数据帧
* 入口参数：u8 addr——8位地址  unsigned int *data-32位SPI数据
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/  
int Motor_Spi_Write(unsigned char addr,int data)
{	
    int ret = 0;
    printk("----Motor_Spi_Write addr0x%x   data 0x%x \n",addr, data);

	ret = regmap_bulk_write(tmc5041_regmap, addr, &data, 4);  
	if (ret)
    {
        printk("----ret %d \n", ret);
        return ret;
    }
    else
    {
    	return 0;
    }
}

int Motor_Spi_read(unsigned char addr)
{	
    int data = 0;
    int ret = 0;
   
	ret = regmap_bulk_read(tmc5041_regmap, addr, &data, 4); 
	if (ret)
    {
        printk("----ret %d \n", ret);
        return ret;
    }
    else
    {
        printk("----Motor_Spi_read  addr0x%x   data 0x%x \n",addr, data);
    	return data;
    }
}


static long tmc5041_open_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return 0;
}
 
static int tmc5041_open(struct inode * inode, struct file * file)
{
    return 0;
}
 
 
static int tmc5041_close(struct inode * inode, struct file * file)
{
    return 0;
}

static struct file_operations tmc5041_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = tmc5041_open_ioctl,
    .open = tmc5041_open,
    .release = tmc5041_close
};
 
 
static struct miscdevice tmc5041_dev = {
   .minor     = MISC_DYNAMIC_MINOR,
   .name  = "tmc5041_motor",
   .fops = &tmc5041_fops,
};

static int __init tmc5041_init(void)
{
    int ret;
    unsigned int tmp;
    int data_temp;
    struct spi_device *spi_device;

	static const struct regmap_config config = {
		.reg_bits = 8,
		.val_bits = 8,
		.write_flag_mask = 0x80,
	};


    struct device *dev = bus_find_device_by_name(&spi_bus_type, NULL, "spi1.0");
    if (dev == NULL) {
        pr_err("Failed to register ac108 i2c driver : %d \n", ret);
    }
    spi_device=to_spi_device(dev);
    if (spi_device == NULL) {
        pr_err("Failed to register ac108 i2c driver : %d \n", ret);
    }
    spi_device->max_speed_hz=2*1000*1000;
    spi_device->mode=SPI_MODE_3;
    spi_device->bits_per_word=8;
    ret=spi_setup(spi_device);
    printk(KERN_ALERT"hello driver enter 0x%x \n",ret);
    
    tmc5041_regmap=devm_regmap_init_spi(spi_device,&config);
    
    data_temp = 0x00000118;
    ret=regmap_bulk_write(tmc5041_regmap, 0x80,&data_temp,4);
    printk(KERN_ALERT"hello driver enter 0x%x \n",ret);
    
    ret=regmap_bulk_read(tmc5041_regmap, 0x00,&tmp,4);
    printk(KERN_ALERT"---------------regmap_bulk_read 0x%x \n",tmp);
    ret=regmap_bulk_read(tmc5041_regmap,0x00,&tmp,4);
    printk(KERN_ALERT"---------------regmap_bulk_read 0x%x \n",tmp);
    printk("==========================================tmc5041_init\n");




    ret = misc_register(&tmc5041_dev);
    if (0 != ret)
    {
        printk("Kernel: register ssp device failed!\n");
        return -1;
    }


#if 0
    int res;
	unsigned int tmp;
	static const struct regmap_config config = {
		.reg_bits = 8,
		.val_bits = 8,
		.write_flag_mask = 0x80,
	};
    printk("==========================================tmc5041_probe\n");

    printk(KERN_ERR "aic32x4: invalid DAI master/slave interface\n");	
    tmc5041_regmap = devm_regmap_init_spi(spi, &config);
	if (IS_ERR(tmc5041_regmap)) {
		dev_err(&spi->dev, "%s: regmap allocation failed: %ld\n",
			__func__, PTR_ERR(tmc5041_regmap));
		return PTR_ERR(tmc5041_regmap);
	}
    else
    {
        //printk("Ctrl/Stat Reg: %s \n", spi->dev);
    }

	spi->mode = SPI_MODE_3;
	spi->bits_per_word = 8;
	spi_setup(spi);
#endif

	ret = regmap_read(tmc5041_regmap, DS3232_REG_SECONDS, &tmp);
	if (ret)
		return ret;

	/* Control settings
	 *
	 * CONTROL_REG
	 * BIT 7	6	5	4	3	2	1	0
	 *     EOSC	BBSQW	CONV	RS2	RS1	INTCN	A2IE	A1IE
	 *
	 *     0	0	0	1	1	1	0	0
	 *
	 * CONTROL_STAT_REG
	 * BIT 7	6	5	4	3	2	1	0
	 *     OSF	BB32kHz	CRATE1	CRATE0	EN32kHz	BSY	A2F	A1F
	 *
	 *     1	0	0	0	1	0	0	0
	 */
	ret = regmap_read(tmc5041_regmap, DS3232_REG_CR, &tmp);

    printk("----res %d \n", ret);

	if (ret)
		return ret;
	ret = regmap_write(tmc5041_regmap, DS3232_REG_CR, tmp & 0x1c);

    printk("----res %d \n", ret);




	//Motor_Spi_Write(TMC5041_GCONF,0x00000108); // GCONF=8: Enable PP and INT outputs
	Motor_Spi_Write(TMC5041_GCONF,  0x00000208); // GCONF=8: Enable PP and INT outputs

	Motor_Spi_Write(TMC5041_CHOPCONF(H_MOTOR),0x000100C5); // CHOPCONF: TOFF=5, HSTRT=4, HEND=1, TBL=2, CHM=0 (spreadCycle)
    Motor_Spi_read(TMC5041_CHOPCONF(H_MOTOR));
    Motor_Spi_read(TMC5041_CHOPCONF(H_MOTOR));

    
	Motor_Spi_Write(TMC5041_CHOPCONF(V_MOTOR),0x000100C5); // CHOPCONF: TOFF=5, HSTRT=4, HEND=1, TBL=2, CHM=0 (spreadCycle)
    Motor_Spi_read(TMC5041_CHOPCONF(V_MOTOR));
    Motor_Spi_read(TMC5041_CHOPCONF(V_MOTOR));

    Motor_Spi_Write(TMC5041_IHOLD_IRUN(H_MOTOR),0x00011F10); // IHOLD_IRUN: IHOLD=15, IRUN=31 (max. current), IHOLDDELAY=1
    Motor_Spi_read(TMC5041_IHOLD_IRUN(H_MOTOR));

    Motor_Spi_Write(TMC5041_IHOLD_IRUN(V_MOTOR),0x00011F10); // IHOLD_IRUN: IHOLD=15, IRUN=31 (max. current), IHOLDDELAY=1
    Motor_Spi_read(TMC5041_IHOLD_IRUN(V_MOTOR));
    
	Motor_Spi_Write(TMC5041_TZEROWAIT(H_MOTOR),0x00003E8);//1388);//5000	 2710); // TZEROWAIT=10000
    Motor_Spi_read(TMC5041_TZEROWAIT(H_MOTOR));

    Motor_Spi_Write(TMC5041_TZEROWAIT(V_MOTOR),0x00003E8);//1388);//5000	 2710); // TZEROWAIT=10000
    Motor_Spi_read(TMC5041_TZEROWAIT(V_MOTOR));


    
	Motor_Spi_Write(TMC5041_PWMCONF(H_MOTOR) ,0x000401C8); // PWM_CONF: AUTO=1, 2/1024 Fclk, Switch amplitude limit=200, Grad=1
	Motor_Spi_Write(TMC5041_PWMCONF(V_MOTOR) ,0x000401C8); // PWM_CONF: AUTO=1, 2/1024 Fclk, Switch amplitude limit=200, Grad=1

	Motor_Spi_Write(TMC5041_VHIGH(H_MOTOR),0x000AFC80);//61A80); // VHIGH=400 000: Set VHIGH to a high value to allow stealthChop
	Motor_Spi_Write(TMC5041_VCOOLTHRS(H_MOTOR),0x000AECE0);//07530); // VCOOLTHRS=30000: Set upper limit for stealthChop to about 30RPM
	Motor_Spi_Write(TMC5041_VHIGH(V_MOTOR),0x000AFC80); // VHIGH=400 000: Set VHIGH to a high value to allow stealthChop
	Motor_Spi_Write(TMC5041_VCOOLTHRS(V_MOTOR),0x000AECE0);//07530); // VCOOLTHRS=30000: Set upper limit for stealthChop to about 30RPM


#if 0
	if (res)
		return res;

	res = regmap_read(tmc5041_regmap, DS3232_REG_SR, &tmp);
	if (res)
		return res;
	res = regmap_write(tmc5041_regmap, DS3232_REG_SR, tmp & 0x88);
	if (res)
		return res;

	/* Print our settings */
	res = regmap_read(tmc5041_regmap, DS3232_REG_CR, &tmp);
	if (res)
		return res;
	dev_info(&spi->dev, "Control Reg: 0x%02x\n", tmp);

	res = regmap_read(tmc5041_regmap, DS3232_REG_SR, &tmp);
	if (res)
		return res;
	dev_info(&spi->dev, "Ctrl/Stat Reg: 0x%02x\n", tmp);

	//return ds3232_probe(&spi->dev, regmap, spi->irq, "tmc5041");
#endif    
	return 0;
}



#if 0

static struct spi_board_info tmc5041_info = {
	.modalias	= "tmc5041spimotor",
};

static struct spi_driver tmc5041_driver = {
	.driver = {
		.name	 = "tmc5041_motor",
	},
	.probe	 = tmc5041_probe,
};




static int tmc5041_register_driver(void)
{
    //struct device *dev = NULL;
	//struct bcm53xxspi *b53spi;
	struct spi_master *master;
	int err = 0;
#if 0
	if (core->bus->drv_cc.core->id.rev != 42) {
		pr_err("SPI on SoC with unsupported ChipCommon rev\n");
		return -ENOTSUPP;
	}
#endif
    printk("==============================================================spi_alloc_master\n");
	//master = spi_alloc_master(dev, sizeof(*b53spi));
	//if (!master)
	//	return -ENOMEM;

    master = spi_busnum_to_master(1);

    printk("==============================================================spi_alloc_master  OK \n");

#if 0
	b53spi = spi_master_get_devdata(master);
	b53spi->master = master;
	b53spi->core = core;

	if (core->addr_s[0])
		b53spi->mmio_base = devm_ioremap(dev, core->addr_s[0],
						 BCM53XXSPI_FLASH_WINDOW);
	b53spi->bspi = true;
	bcm53xxspi_disable_bspi(b53spi);

	master->transfer_one = bcm53xxspi_transfer_one;
	if (b53spi->mmio_base)
		master->spi_flash_read = bcm53xxspi_flash_read;

	bcma_set_drvdata(core, b53spi);
	err = devm_spi_register_master(dev, master);

	if (err) {

         printk("==============================================================devm_spi_register_master  error \n");
		spi_master_put(master);
		//bcma_set_drvdata(core, NULL);
		return err;
	}
#endif

	/* Broadcom SoCs (at least with the CC rev 42) use SPI for flash only */
	spi_new_device(master, &tmc5041_info);

	printk("==============================================================tmc5041_register_driver\n");

    err = spi_register_driver(&tmc5041_driver);
    if (err) 
    {
        pr_err("SPI on SoC with unsupported ChipCommon rev\n");
        return err;
    }
    return 0;
}



static int __init tmc5041_init(void)
{
	int ret;











#if 0
	printk("==============================================================tmc5041_init\n");
	ret = tmc5041_register_driver();
	if (ret) {
		printk("==============================================================Failed to register tmc5041 driver\n");
		tmc5041_unregister_driver();
	}
#endif    

	return ret;
}
#endif






static void tmc5041_unregister_driver(void)
{
	//spi_unregister_driver(&tmc5041_driver);
	return;
}

static void __exit tmc5041_exit(void)
{
    regmap_exit(tmc5041_regmap);
    printk(KERN_ALERT"tmc5041 driver exit");

    misc_deregister(&tmc5041_dev);
	tmc5041_unregister_driver();
	//tmc5041_unregister_driver();
}


module_init(tmc5041_init)
module_exit(tmc5041_exit)




MODULE_AUTHOR("VHD lym");
MODULE_DESCRIPTION("tmc5041 Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:tmc5041");


