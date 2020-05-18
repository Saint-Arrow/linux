#include "ccallp.h"
#include <stdio.h>
#include <stdlib.h>
 
int great_function_from_python(int a,char *buf)
{
    Py_Initialize();
 
     // 检查初始化是否成功  
    // if ( !Py_IsInitialized() ) {  
    //	printf("error!");
    //     return -1;  
    // }  
 
    int res;
    PyObject *pModule,*pFunc;
    PyObject *pArgs, *pValue;
 
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
 
    /*import ccallp.py file 注意没有.py*/
    pModule = PyImport_Import(PyString_FromString("ccallp"));
 
    if ( !pModule ) {  
        printf("can't find ccallp.py");  
        getchar();  
        return -1;  
    }  
 
    /* great_module.great_function */
    pFunc = PyObject_GetAttrString(pModule,"great_function");
 
    /* build args */
    pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs,0,PyInt_FromLong(a));
	PyTuple_SetItem(pArgs,1,PyString_FromString(buf));
 
    /*call*/
    pValue = PyObject_CallObject(pFunc,pArgs);
 
    res = PyInt_AsLong(pValue);
 
    Py_Finalize();
 
    return res;
 
}
 
/*也可以直接执行以下命令  反注释main 函数 直接运行这个文件
    ： gcc ccallp.c -o ccallp -I/usr/include/python2.7/ -lpython2.7
    ：  ./ccallp 
*/
// int main(int argc, char *argv[])
// {
 
//  printf("%d\n",great_function_from_python(4));
 
// }
