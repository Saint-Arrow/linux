/*======================================================================
    A test program in userspace   
    This example is to introduce the ways to use "select"
     and driver poll                 
      
    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#define FIFO_CLEAR 0x1
#define BUFFER_LEN 64
main()
{
  int fd, num,write_num,read_num;
  char rd_ch[BUFFER_LEN];
  fd_set rfds,wfds;
  
  /*以非阻塞方式打开/dev/globalmem设备文件*/
  //fd = open("/dev/gpio_keys", O_RDWR | O_NONBLOCK);
  fd = open("/dev/gpio_keys", O_RDWR);
  if (fd !=  - 1)
  {
	printf("Device open OK\n");
    while (1)
    {
      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);

      select(fd + 1, &rfds, NULL, NULL, NULL);
      /*数据可获得*/
      if (FD_ISSET(fd, &rfds))
      {
      	printf("Poll monitor:can be read\n");
        read_num=read(fd,rd_ch,sizeof(rd_ch));
        printf("read_num :%d:%c\n",read_num,rd_ch[0]);
      }     
    }
  }
  else
  {
    printf("Device open failure\n");
  }
}
