/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#ifndef TP_SAMPLE
#define TP_SAMPLE

#include <linux/proc_fs.h>	/* for struct inode and struct file */
#include <linux/tracepoint.h>

DECLARE_TRACE(subsys_event,
	TP_PROTO(struct inode *inode, struct file *file),
	TP_ARGS(inode, file));
    
#endif