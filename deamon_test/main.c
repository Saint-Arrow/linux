#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <errno.h>
#include <sys/resource.h>
#define LOCKFILE "/dev/deamon.pid"
#if 1
int lock_set(int fd,int type)  
{  
    struct flock old_lock,lock;  
    lock.l_whence = SEEK_SET;  
    lock.l_start = 0;  
    lock.l_len = 0;  
    lock.l_type = type;  
    lock.l_pid = -1;  
          
    fcntl(fd,F_GETLK,&lock);  
  
    if(lock.l_type != F_UNLCK)  
    {  
          
        if (lock.l_type == F_RDLCK)    
        {  
            syslog(LOG_ERR,"pid %d Read lock already set by %d\n",getpid(),lock.l_pid);  
        }  
        else if (lock.l_type == F_WRLCK)   
        {  
            syslog(LOG_ERR,"pid %d Write lock already set by %d\n",getpid(),lock.l_pid);  
        }                         
    }  
      
    lock.l_type = type;  
      
    if (fcntl(fd,F_SETLK,&lock) < 0)  
    {  
        syslog(LOG_ERR,"pid %d Lock failed : type = %d\n",getpid(),lock.l_type);  
        return 1;  
    }  
    if((errno==EACCES)||(errno==EAGAIN))  
	{
		syslog(LOG_ERR,"pid %d Lock failed2 : type = %d\n",getpid(),lock.l_type);  
        return 1; 
	}
    switch (lock.l_type)  
    {  
        case F_RDLCK:  
        {  
            syslog(LOG_ERR,"Read lock set by %d\n",getpid());  
        }  
        break;  
        case F_WRLCK:  
        {  
            syslog(LOG_ERR,"write lock set by %d\n",getpid());  
        }  
        break;  
        case F_UNLCK:  
        {  
            syslog(LOG_ERR,"Release lock by %d\n",getpid());    
        }  
        break;  
          
        default:  
        break;  
  
    }  
    return 0;  
} 

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
int already_running(void)
{
	int fd;
	char buf[16];
	fd=open(LOCKFILE,O_RDWR|O_CREAT,LOCKMODE);
	if(fd < 0)
	{
		syslog(LOG_ERR,"can not open %s:%s\n",LOCKFILE,strerror(errno));
		exit(1);
	}
	if(lock_set(fd,F_WRLCK))
	{
		close(fd);
		exit(1);
	}
	lock_set(fd,F_WRLCK);
	ftruncate(fd,0);
	sprintf(buf,"%ld",(long)getpid());
	write(fd,buf,strlen(buf)+1);
	
	return 0;
}
#else
FILE *fp=NULL;
void open_file(void)
{
	if(fp==NULL)
	{
		if((fp=fopen(LOCKFILE,"r"))==NULL)
		{
			syslog(LOG_ERR,"can not open %s:%s\n",LOCKFILE,strerror(errno));
			return 1;
		}
	}
}
int lock_file(void)
{
	open_file();
	if(flock(fp->_fileno,LOCK_EX|LOCK_NB)!=0)
	{
		syslog(LOG_ERR,"can not lock %s\n",LOCKFILE);
		return 1;
	}
	syslog(LOG_ERR,"leave\n");
	return 0;
}
int unlock_file(void)
{
	open_file();
	flock(fp->_fileno,LOCK_UN);
	fclose(fp);
	fp=NULL;
	return 0;
}
int already_running(void)
{
	int ret=0;
	ret=lock_file();
	if(ret)
	{
		syslog(LOG_ERR,"can not lock_file %s\n",LOCKFILE);
		exit(1);
	}
}
#endif
int daemonize(const char *cmd)
{
	int i,fd0,fd1,fd2;
	pid_t pid;
	struct rlimit rl;
	//struct sigaction sa;
	
	/*
	*Clear file creation mask.
	*/
	umask(0);
	
	/*
	*Become a session leader
	*/
	if ((pid = fork())<0)
	{
		syslog(LOG_ERR,"%s: can't fork", cmd);
		return -1;
	}
	else if (pid !=0)/* parent */
	{
		exit(0);
	}
	else
	{
		setsid();
	}
	
	/*
	*Close all open file descriptors.
	*/
	getrlimit(RLIMIT_NOFILE,&rl);
	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max =1024;
	for(i=0;i<rl.rlim_max; i++)
		close(i);

	/*
	Change the current working directory to the root so* we won't prevent file systems from being unmounted.
	*/
	if (chdir("/")< 0)
		syslog(LOG_ERR,"%s:can't change directory to /",cmd);
	
	/*
	*Attach file descriptors 0， 1, and 2 to /dev/null.
	*/
	fd0 = open("/dev/null",O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
	return 0;
}

int main(int argc,char *argv[])
{
	int ret=0;
	int count=0;
	ret=daemonize(argv[0]);
	if(ret)
	{
		syslog(LOG_ERR,"daemonize fail\n");
	}
	
	already_running();
	
	while(1)
	{
		sleep(1);
		syslog(LOG_ERR,"count:%d\n",count++);
	}
}