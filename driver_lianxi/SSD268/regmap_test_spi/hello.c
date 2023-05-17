#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
struct ac108_priv {
    struct regmap *regmap;
};
struct ac108_priv motor_status;



static const struct regmap_config aic3x_regmap = {
	.reg_bits = 8,
	.val_bits = 32,
    .write_flag_mask = 0x80,
};

static int __init hello_init(void)
{
    int ret = 0;
    int value=0;
  
    
    struct spi_device *spi_device;
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
    printk(KERN_ALERT"hello driver enter 0x%x",ret);
    
    motor_status.regmap=devm_regmap_init_spi(spi_device,&aic3x_regmap);
    value=0x108;
    ret=regmap_write(motor_status.regmap, 0x00,0x108);
    //ret=regmap_bulk_write(motor_status.regmap, 0x00,&value,4);
    printk(KERN_ALERT"hello driver regmap_write 0x%x\n",ret);
    value=0;
    
    ret=regmap_read(motor_status.regmap, 0x00,&value);
    //ret=regmap_bulk_read(motor_status.regmap, 0x00,&value,4);
    printk(KERN_ALERT"hello driver regmap_read 0x%x 0x%x\n",ret,value);
    ret=regmap_read(motor_status.regmap, 0x00,&value);
    //ret=regmap_bulk_read(motor_status.regmap,0x00,&value,4);
    printk(KERN_ALERT"hello driver regmap_read 0x%x 0x%x\n",ret,value);
    return 0;
}
static void __exit hello_exit(void)
{
    regmap_exit(motor_status.regmap);
    printk(KERN_ALERT"hello driver exit");
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");