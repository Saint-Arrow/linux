#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>


#define procfs_name "proc_test"
struct proc_dir_entry *Our_Proc_File;
static char msg[256];
static char *mydir;

int procfile_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len=strlen(msg);
	if(count>len-off)
	{
		count=len-off;
	}
	memcpy(page+off,msg+off,count);
	return off+count;
}
int procfile_write(struct file *file, const char __user *buffer,
			 unsigned long count, void *data)
{
	unsigned long count2=count;
	if(count2>=sizeof(msg)) count2=sizeof(msg)-1;
	if(copy_from_user(msg,buffer,count2)) return -EFAULT;
	msg[count2]='\0';

	return count2;

}

int procfile_init()
{
	 mydir = proc_mkdir("mydir",NULL);
	 if(!mydir)
	 {
			 printk(KERN_ERR "can't create /proc/mydir\n");
			 return -1;
	 }


	Our_Proc_File=create_proc_entry(procfs_name,0644,mydir);
	if(Our_Proc_File==NULL)
	{
		remove_proc_entry(procfs_name,mydir);
		printk(KERN_ALERT "error create /proc/%s",procfs_name);
		return -ENOMEM;
	}
	Our_Proc_File->read_proc=procfile_read;
	Our_Proc_File->write_proc = procfile_write;
	//Our_Proc_File->owner = THIS_MODULE;
	Our_Proc_File->mode = S_IFREG | S_IRUGO;
	Our_Proc_File->uid = 0;
	Our_Proc_File->gid = 0;
	Our_Proc_File->size = 37;
	printk("/proc/%s created\n",procfs_name);
	return 0;
}

void proc_exit()
{
        remove_proc_entry(procfs_name,mydir);
        printk(KERN_INFO "/proc/%s removed\n",procfs_name);
		remove_proc_entry("mydir",NULL);

}

module_init(procfile_init);
module_exit(proc_exit);


