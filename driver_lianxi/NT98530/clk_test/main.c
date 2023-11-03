#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


#include <linux/device.h>
#include <linux/kdev_t.h>

#include <linux/clk.h>
#include <linux/of.h>


static int clk_rate = 74250000;
module_param_named(clk_rate, clk_rate, int, 0600);


static char *clk_name = "2f0900000.ide";
module_param(clk_name, charp, 0644);
MODULE_PARM_DESC(clk_name,"2f0900000.ide");

static struct class *my_class;
static struct device *my_device;

static int __init hello_init(void)
{
    int result = 0;

    // 创建设备类
    my_class = class_create(THIS_MODULE, "my_class");
    if (IS_ERR(my_class)) {
        pr_err("Failed to create class\n");
        result = PTR_ERR(my_class);
        goto out;
    }

    // 创建设备
    my_device = device_create(my_class, NULL, MKDEV(0, 0), NULL, "my_device");
    if (IS_ERR(my_device)) {
        pr_err("Failed to create device\n");
        result = PTR_ERR(my_device);
        goto class_cleanup;
    }
    pr_info("Device created successfully\n");
    
    {
        //const char *clk_name="2f0900000.ide"; // 时钟名称字符串

        struct clk *clk = devm_clk_get(my_device, clk_name);
        if (IS_ERR(clk)) {
            printk(KERN_ERR "Failed to get clock %s\n", clk_name);
            return PTR_ERR(clk);
        }
        unsigned long freq = clk_get_rate(clk);
        printk(KERN_ERR "get clock %s=%d,try to set %d\n", clk_name,freq,clk_rate);

        unsigned long new_freq = clk_rate; // 新的时钟频率
        unsigned long flags = 0; // 标志位
        int ret = clk_set_rate(clk, new_freq);
        if (ret < 0) {
            printk(KERN_ERR "Failed to set clock rate\n");
            return ret;
        }
        //clk_disable_unprepare(clk);
        
        
        ret = clk_prepare_enable(clk); // 启用时钟
        if (ret < 0) {
            printk(KERN_ERR "Failed to enable clock\n");
            return ret;
        }
        clk_disable_unprepare(clk);
        devm_clk_put(my_device,clk);
        
    }

    // 销毁设备
    device_destroy(my_class, MKDEV(0, 0));

    // 销毁设备类
    class_destroy(my_class);

    return -1;

class_cleanup:
    class_destroy(my_class);
out:
    return result;
}
static void __exit hello_exit(void)
{


    pr_info("Device destroyed\n");
    printk(KERN_ALERT"hello driver exit");
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");