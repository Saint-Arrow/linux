#ifndef CCALLP_H
#define CCALLP_H
 
//#include "....(python2.7安装路径)/include/python2.7/Python.h" 
#include "Python.h" 
//Python.h 文件所在路径在你的python2.7 安装目录下的include/python2.7/ 下面。 
// 不知道路径的，可以直接命令：find  / -name Python.h查看，然后复制到include “..” 里面 （我的输入这命令，出来一大堆东西，但还是可以容易找出自己的安装路径）
int great_function_from_python(int a,char *buf);
#endif
