#include "ccallp.h"
#include <stdio.h>
#include <stdlib.h>
 
// 执行命令：gcc ccallp.c ccallpMain.c -o ccallpMain -I/usr/include/python2.7/ -lpython2.7
//  ./ccallpMain
 
int main(int argc, char *argv[])
{
	int int_a=atoi(argv[1]);
	char *str_sample="123";
    printf("%d\n",great_function_from_python(int_a,str_sample));
 
}
