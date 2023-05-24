/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timex.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

#include <linux/delay.h>


int board_init(void)
{
    panic("board_init test");
    
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