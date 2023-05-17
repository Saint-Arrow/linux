#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include<stdint.h>
#include <stdlib.h>

#define DEVIDE_NAME "/dev/dht11"

typedef unsigned short int u16;
typedef unsigned char u8;
#define  READ_DATE  _IOR('D', 1,struct dht11_date)
struct dht11_date{
        u16 temp;  //温度 高8位是整数 低8位是小数
        u16 hum;   //湿度 高8位是整数 低8位是小数
};

void prit_temp_hum(struct dht11_date date)
{
    float temp = 0.0;
	float hum = 0.0;

	temp = date.temp >> 8; //整数
	temp += (date.temp&&0xff)/100.0;  //小数
	
	hum = date.hum >>8; //整数
	
	hum += (date.hum&&0xff)/100.0;  //小数
	
	printf("temp=%.3f ,hum=%.3f\n",temp,hum);
	
}

int sleep_time=2;


int main(int argc,char *argv[])
{
   int fd;
    int ret;
   struct dht11_date date;
    fd =open(DEVIDE_NAME,O_RDWR);
    if(fd < 0)
	{
	   printf("open file is fail\n");
	   return -1;
    }
		
	if(argc>1)
	{
		sleep_time=atoi(argv[1]);
	}
	while(1)
	{
     
		ret = ioctl(fd,READ_DATE,&date);
		if(ret == 0)
		{	
			
			prit_temp_hum(date);
		}
		//else printf("date get fail\n");
		sleep(sleep_time);
	}
     close(fd);
}
