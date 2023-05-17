#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


#include <linux/regmap.h>
#include <linux/i2c.h>
struct ac108_priv {
	struct i2c_client *i2c;
    struct regmap *regmap;
	//struct snd_soc_codec *codec;
	//struct voltage_supply vol_supply;
};
struct i2c_client *i2c_clt=NULL;

//I2C devices register method_1: i2c_board_info (i2c_register_board_info)
//I2C devices register method_2: device tree source (in .dts file)
#define I2C_BUS_NUM 3
static struct i2c_board_info ac108_i2c_board_info[] = {
	{I2C_BOARD_INFO("ac108", 0x29),},//ac108_0
};
static const struct i2c_device_id ac108_i2c_id[] = {
	{ "ac108", 0 },//ac108_0
	{ }
};
MODULE_DEVICE_TABLE(i2c, ac108_i2c_id);
static const struct regmap_config aic3x_regmap = {
	.reg_bits = 16,
	.val_bits = 8,
};
static int ac108_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *i2c_id)
{
	struct ac108_priv *ac108;
	//struct device_node *np = i2c->dev.of_node;
	//char *regulator_name = NULL;
	int ret = 0;
    int value=0;
    
	ac108 = devm_kzalloc(&i2c->dev, sizeof(struct ac108_priv), GFP_KERNEL);
	if (ac108 == NULL) {
		dev_err(&i2c->dev, "Unable to allocate ac108 private data\n");
		return -ENOMEM;
	}
    ac108->regmap = devm_regmap_init_i2c(i2c, &aic3x_regmap);
	if (IS_ERR(ac108->regmap)) {
		ret = PTR_ERR(ac108->regmap);
		return ret;
	}
    i2c_set_clientdata(i2c, ac108);
    ret=regmap_read(ac108->regmap,0x010f,&value);
    
    printk(KERN_EMERG "%s[%d] ret:%d value:0x%x\n",__func__,__LINE__,ret,value);
    return 0;
}
static int ac108_i2c_remove(struct i2c_client *i2c)
{
	return 0;
}
static struct i2c_driver ac108_i2c_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "ac108",
		.owner = THIS_MODULE,
	},
	.probe = ac108_i2c_probe,
	.remove = ac108_i2c_remove,
    .id_table = ac108_i2c_id,
};
static int __init hello_init(void)
{
    int ret = 0;
    
#if 1
   struct i2c_adapter* i2c_adap;
    i2c_adap = i2c_get_adapter(I2C_BUS_NUM);
    i2c_clt = i2c_new_device(i2c_adap, &ac108_i2c_board_info[0]);
    if(!i2c_clt)
    {
		printk("226\n");
    }   
	
	i2c_put_adapter(i2c_adap); 
#endif
    printk(KERN_ALERT"hello driver enter");
	ret = i2c_add_driver(&ac108_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register ac108 i2c driver : %d \n", ret);
	
    return 0;
}
static void __exit hello_exit(void)
{
    i2c_del_driver(&ac108_i2c_driver);
    if(i2c_clt) i2c_unregister_device(i2c_clt);
    printk(KERN_ALERT"hello driver exit");
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");