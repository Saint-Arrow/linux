#!/usr/bin/env python
# -*- coding:utf-8 -*-
import os,time
import sys
#环境变量获取以及程序运行路径
print(os.environ['PWD'],type(os.environ['PWD']))
print(os.environ['USER'],type(os.environ['USER']))
#输入参数列表
print(sys.argv,len(sys.argv))

def usage():
    print("please enter"+sys.argv[0]+" cmd")
    
def check_argc(argv,max):
    return (len(argv)<max)
    

    
def touch_encode_file(file_name):
    file=open(file_name,"w")
    file.write("/**  \n")
    file.write(" *auto create by python,according to doxygen \n")
    file.write("*/ \n")
    file.close()

def touch_sh(file_name):
    file=open(file_name,"w")
    file.write("#auto create by python\n")
    file.close()

def touch_py(file_name):
    file=open(file_name,"w")
    file.write("#!/usr/bin/env python\n")
    file.write("# -*- coding:utf-8 -*-\n")
    file.write("#auto create by python\n")
    file.close()

if check_argc(sys.argv,2):
    usage()
    sys.exit (1)
if (sys.argv[1]=="touch"):
        if check_argc(sys.argv,3):
            print("please enter file name")
            sys.exit (1)
        file_name=sys.argv[2]
        if(".sh"==os.path.splitext(file_name)[1]):
            touch_sh(file_name)
        elif(".py"==os.path.splitext(file_name)[1]):
            touch_py(file_name)
        else:
            touch_encode_file(file_name)



