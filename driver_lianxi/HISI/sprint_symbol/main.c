/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/interrupt.h>


#include <linux/kallsyms.h>
/*https://www.cnblogs.com/sky-heaven/p/14977059.html*/
int board_init(void)
{
    char buffer[KSYM_SYMBOL_LEN];                     //声明一个文本缓冲区
    int ret;                                          //接收sprint_symbol( )函数返回值
    unsigned long address;                            //表示符号地址
    
    address=0xc024c9cc;
    ret = sprint_symbol( buffer , address );
    printk("ret: %d\n", ret );                        //输出返回值
    printk("buffer: %s\n", buffer );                  //输出文本缓冲区buffer的内容
    printk("\n");
    
    
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