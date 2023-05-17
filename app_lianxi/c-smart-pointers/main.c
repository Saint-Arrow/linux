#include "../header_file.h"
#include "c-smart-pointers.h"

typedef struct
{
    int test_jk;
}test;
int free_s(void *ptr)
{
    printf("free_struct test_jk=%d\n",((test *)ptr)->test_jk);

}
int main(int argc ,char *argv[])
{
    if(1)
    {   
	smart test *p_s=smalloc(sizeof(int),free_s);
	p_s->test_jk=2;
    }
    smart int *p_int=smalloc(sizeof(int),free_no_action);
    for(int i=0;i<1000;i++)
    {    
	smart char *p=smalloc(1024*1024,free_no_action);
	smart FILE **fd=smalloc(sizeof(FILE **),free_fclose);	
	*fd=fopen("./file_test","w+");
	usleep(100*1000);
    }

    printf("leave\n");
}
