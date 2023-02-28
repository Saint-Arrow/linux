/**  
*auto create by /home/cwj/tool.py,according to doxygen 
*/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <linux/pid.h>
#include <linux/module.h>
#include <linux/sched.h>


#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/fdtable.h>
#include <linux/mount.h>

/*
https://zhuanlan.zhihu.com/p/480934356
*/
#include <linux/kallsyms.h>
unsigned long *sys_call_table=NULL;

typedef void (*iterate_supers_self)(struct file_system_type *, void (*f)(struct super_block *, void *)  , void *);
unsigned long fun_addr = 0xc009c59e;//cat /proc/kallsyms |grep iterate_supers
iterate_supers_self iterate_supers_temp;


struct super_block *sb;
struct super_block *sb1;
/*
nfs s_blocksize_bits:13 s_blocksize:8192 s_maxbytes:2147483647
jffs2 s_blocksize_bits:12 s_blocksize:4096 s_maxbytes:4294967295
squashfs s_blocksize_bits:12 s_blocksize:4096 s_maxbytes:17592186040320
tmpfs s_blocksize_bits:12 s_blocksize:4096 s_maxbytes:17592186040320

open file[2]:/dev/console
rootfs s_blocksize_bits:12 s_blocksize:4096 s_maxbytes:17592186040320

open file[14]:socket:[360]
sockfs s_blocksize_bits:12 s_blocksize:4096 s_maxbytes:17592186040320
*/
static void sb_trace_test(struct super_block *sb, void *arg)
{
    if(sb!=NULL)
    {
        pr_err("sb:%p ",sb);
        pr_err("%s s_blocksize_bits:%d s_blocksize:%ld s_maxbytes:%lld",sb->s_type->name,sb->s_blocksize_bits,sb->s_blocksize,sb->s_maxbytes);
        pr_err("s_dev:%d",sb->s_dev);
    }
    else
    {
        pr_err("sb is null");
    }
}
static void get_fs_root_rcu(struct fs_struct *fs, struct path *root)
{
	unsigned seq;

	do {
		seq = read_seqcount_begin(&fs->seq);
		*root = fs->root;
	} while (read_seqcount_retry(&fs->seq, seq));
}
int board_init(void)
{
    struct block_device *bdev;
    char buf[256]="123";
    char* path=NULL;
    struct file *fp=NULL;
    //struct filename *tmp;
    char test_filename[64]="/bin/busybox";
    struct file_system_type *fs_type_p=NULL;
    
    struct dentry *mnt_root;
    struct path root;
    char search_filename[64]="customer/conf_backup";
    struct qstr search_name;
    struct dentry *search_dentry;
    
    struct inode *inode;
    
    
    
    /*******查找遍历 超级块*/
    sys_call_table = (unsigned long *)kallsyms_lookup_name("iterate_supers_type");
    if(sys_call_table!=NULL)
    {
        pr_err("iterate_supers:%p",sys_call_table);
        iterate_supers_temp=(iterate_supers_self)sys_call_table;
    }
    else
    {
        iterate_supers_temp=(iterate_supers_self)fun_addr;
    }
    
    
    fs_type_p=get_fs_type("jffs2");
    if(fs_type_p)
    {
        //iterate_supers_temp(fs_type_p,sb_trace_test,NULL);//这里会发生oops异常
        //iterate_supers_type(fs_type_p,sb_trace_test,NULL);
    }
    
    
    sb=current->fs->pwd.dentry->d_sb;
    sb_trace_test(sb,NULL);
    
    /*
    *https://www.coolcou.com/linux-kernel/linux-kernel-file-system-api/the-linux-kernel-get-super.html
    */
    bdev = sb->s_bdev;//有些super block的block_device属性为NULL
    pr_err("s_bdev:%p",bdev);
    sb1 = get_super(bdev);
    sb_trace_test(sb1,NULL);
    
   
    
    /*******查找 根 目录项*/
    path=d_path((const struct path *) & current->fs->pwd,buf,sizeof(buf));
    if(path) pr_err("pwd :%s",path);
    mnt_root=current->fs->pwd.mnt->mnt_root;
    path=dentry_path_raw(mnt_root,buf,sizeof(buf));
    if(path) pr_err("pwd root :%s",path);
    
    path=d_path((const struct path *) & current->fs->root,buf,sizeof(buf));
    if(path) pr_err("root :%s",path);
    mnt_root=current->fs->root.mnt->mnt_root;
    path=dentry_path_raw(mnt_root,buf,sizeof(buf));
    if(path) pr_err("root root :%s",path);
    
    rcu_read_lock();
	get_fs_root_rcu(current->fs, &root);
	rcu_read_unlock();
    path=d_path((const struct path *) &root,buf,sizeof(buf));
    if(path) pr_err("current->fs root :%s",path);
    
    
    /*******查找指定目录或文件*/
    search_name.name=search_filename;
    search_name.len=strlen(search_filename);
    search_name.hash = full_name_hash(mnt_root, search_name.name, search_name.len);
    search_dentry=d_lookup(mnt_root,(const struct qstr *)&search_name);
    //只能寻找子目录，不能寻找孙目录，另外名字也不能带斜杆，只能1层1层往下找
    if(search_dentry)
    {
        path=dentry_path_raw(search_dentry,buf,sizeof(buf));
        if(path) pr_err("search_name :%s",path);
    }
    else
    {
        pr_err("can not find  :%s",search_filename);
    }
    
    fp=filp_open(test_filename,O_RDONLY,0);
    if(!IS_ERR(fp)) //不能用NULL来判断
    {
        pr_err("open %s ok",test_filename);
        sb_trace_test(fp->f_path.dentry->d_sb,NULL);
    }
    else
    {
       pr_err("open %s fail",test_filename);
    }     
    
    /*****inode的学习 可以参考 lstat64 系统调用 ***/
    /*可以通过 ls -i 查看具体文件的inode编号 或是stat*/
    /*可以通过 df -i 查看 各个分区的 inode 使用情况 不同目录的 inode 是可以重复的*/
    //generic_fillattr i_size_read  i_blocksize
    inode=fp->f_inode;
    pr_err("i_ino:%lu",inode->i_ino);
    pr_err("file size:%llu ",(unsigned long long)i_size_read(inode));
    pr_err("blocksize:%u",i_blocksize(inode));
    pr_err("blocks:%llu",(unsigned long long)inode->i_blocks);
    
    
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