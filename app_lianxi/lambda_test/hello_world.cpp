#include <ctype.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
/*相关头文件的文件名中不得有扩展名*/



using namespace std;

//#define NULL_VIRTUAL_TEST


int g_count=7;//全局作用于变量

typedef void(*test)(int num);
    
void print_test(test f,int num)
{
    f(num);
}
void f0(int num)
{ 
    std::cout << __LINE__ <<" "<<"num= "<<num<<std::endl;
}

int main(int argc,char *argv[] )
{
    int g_count2=8;
	std::cout << argv[0]<<std::endl;

	std::cout <<__LINE__ <<" "<< "g_count:"<<g_count<<std::endl;
    std::cout <<__LINE__ <<" "<< "g_count2:"<<g_count2<<std::endl;

    std::cout <<"--------------------------------------"<<std::endl;
    /*
    *使用 [] 表示不捕获任何变量。在这种情况下，Lambda 函数只能访问其参数和静态变量
    */
    print_test(f0,10);
    print_test([](int c)->void{
        std::cout << __LINE__ <<" "<<"print_test num: "<<c<<std::endl;
    },11);
    
    auto lam=[c=12]()->void{
        std::cout << __LINE__ <<" "<<"run lambda c:"<<c<<std::endl;
    };
    lam();
    
    std::cout <<"--------------------------------------"<<std::endl;
    /*单独指定
    *非自动存储期的变量通常是具有静态存储期或线程存储期的变量，如全局变量、静态变量或静态成员变量
    *具有非自动存储期的变量，可以选择使用按引用捕获来解决该问题
    */
    [g_count,&g_count2]()->void{
        std::cout << __LINE__ <<" "<<"run lambda "<<std::endl;
        g_count++;
        g_count2++;
    }();
    std::cout <<__LINE__ <<" "<< "g_count:"<<g_count<<std::endl;
    std::cout <<__LINE__ <<" "<< "g_count2:"<<g_count2<<std::endl;

    std::cout <<"--------------------------------------"<<std::endl;
    /**
    *[&] 捕获了当前作用域中的所有变量，并允许在 Lambda 函数体内对这些变量进行修改
    */
    [&]()->void{
        std::cout << __LINE__ <<" "<<"run lambda "<<std::endl;
        g_count++;
        g_count2++;
    }();
    std::cout <<__LINE__ <<" "<< "g_count:"<<g_count<<std::endl;
    std::cout <<__LINE__ <<" "<< "g_count2:"<<g_count2<<std::endl;
	
}
