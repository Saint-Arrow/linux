#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <stdio.h>  
#include <poll.h>  
#include <signal.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <fcntl.h> 
//https://www.cnblogs.com/subo_peng/p/5325324.html
int fd;
void my_signal_fun(int signum)  
{  
    unsigned char key_val;  
    read(fd, &key_val, 1);  
    printf("key_val: 0x%x\n", key_val);  
} 
int main(int argc,char **argv)
{
    unsigned char key_val;  
    int ret;  
    int Oflags;
	
	fd=open("/dev/buttons", O_RDWR);
	if (fd < 0)  
	{  
		printf("can't open dev!\n");  
		return -1;
	} 
	/*****step:0******/
	fcntl(fd, F_SETOWN, getpid()); 
	/*****step:1******/
	Oflags = fcntl(fd, F_GETFL); 
	fcntl(fd, F_SETFL, Oflags | FASYNC); 
	/*****step:2******/ 
	signal(SIGIO, my_signal_fun);  

	while (1)  
	{  
		sleep(1000);  
	}
}