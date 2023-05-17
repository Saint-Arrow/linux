/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/interrupt.h>

//#include <mach/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>


#define _CHECK_OF(ret,string) \
if(ret<0)\
{\
    pr_err("%s:%d \n",string,ret);\
    break;\
}


#define _CHECK_OF_ARRAY(ret,string) \
if(ret<0)\
{\
    if(ret==-EINVAL)\
    {\
        pr_err("%s:EINVAL \n",string);\
    }\
    else if(ret==-ENODATA)\
    {\
        pr_err("%s:ENODATA \n",string);\
    }\
    else if(ret==-EOVERFLOW)\
    {\
        pr_err("%s:EOVERFLOW \n",string);\
    }\
    else \
    {\
        pr_err("%s:%d \n",string,ret);\
    }\
    break;\
}

struct device_node  *np,*np_root;
struct property*    blprop;
u8 u8_array[4];
u32 u32_array[4];
u64 u64_array[4];
const char *prop_string;
int board_init(void)
{
	int     ret;
    np_root=of_find_node_by_path("/");
    if(NULL==np_root)
    {
        pr_err("can not find / in dts\n");
        return -1;
    }
    
    /*for_each_of_allnodes 未导出符号表无法加载驱动*/
    np=of_find_all_nodes(np_root); 
    while(np)
    {
        /**
        *假如 dts中的配置： spi_bus0: spi@12070000 
        *那name是 spi,full_name是 /soc/spi@12070000
        */
        pr_err("%s:%s,0x%x\n",np->name,np->full_name,np->type);
        np=of_find_all_nodes(np);
    }
    pr_err("print all node finish\n");
    

    /**注意 of_find_node_by_name 搜索的是name,不是full_name，所以它是区分不了spi0还是1的*/
    np=of_find_node_by_name(NULL,"spi");
    if(np)
    {
        const char *prop="reg";
        do{
        int  size=of_property_count_u32_elems(np,prop);
        ret=of_property_read_u32_array(np,prop,u32_array,size);
        _CHECK_OF(ret,prop);
        pr_err("prop:%s size:%d 0x%x 0x%x\n",prop,size,u32_array[0],u32_array[1]);
        }while(0);
    }
    
    
    np=of_find_compatible_node(NULL,NULL,"hisilicon,hisi-femac-mdio");/*该方法也无法区分compatible一致的节点*/
    if(np)
    {
        const char *prop="clock-names";
        do{
        ret=of_property_read_string(np,prop,&prop_string);
        _CHECK_OF(ret,prop);
        pr_err("%s:%s\n",prop,prop_string);
        }while(0);
        
        int  size;
        blprop=of_find_property(np,prop,&size);
        if(blprop)
        {
            pr_err("prop:%s name:%s,_flags:0x%x size:%d\n",prop,blprop->name,blprop->_flags,size);
        }
    }
    
    np=of_find_node_by_path("/soc/gpio_chip@120b4000");/*只有该方法使用全路径的情况才能获取真实对应的node节点*/
    if(np)
    {
        const char *prop="interrupts";
        int  size;
        blprop=of_find_property(np,prop,&size);/*字符串的话就是sizeof，数组的话一般是4*个数，猜测可能与32位或64位有关*/
        if(blprop)
        {
            pr_err("prop:%s name:%s,_flags:0x%x size:%d\n",prop,blprop->name,blprop->_flags,size);
        }
    }
    
    
    pr_err("board_init\n");
    return 0;
}
void board_cleanup(void)
{
    pr_err("board_cleanup\n");
}

module_init(board_init);
module_exit(board_cleanup);
MODULE_LICENSE("GPL");