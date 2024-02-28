#include <fstream> //提供了文件输入输出相关的类和函数，用于文件操作
#include <iomanip> //提供了控制输入输出格式的工具，如设置输出精度、对齐方式等
#include <iostream> //提供了输入输出流的类和函数，包括 std::cin、std::cout
#include <cstdlib> //提供了杂项函数和类型，如字符串转换、随机数生成 system调用
#include <cstring> //提供了字符串处理相关的类和函数，包括 std::string 类和相关的操作函数
#include <vector> //提供了动态数组（向量）相关的类和函数，包括 std::vector 类
#include <algorithm>

#include <ctime>
#include <chrono> //时间相关的函数

/**
 * 使用函数模板，注意使用 typename 这个C++的关键词来定义变量类型，函数中使用T作为变量类型
 * 虽然说函数模板提高了代码复用性，但是有些结构体，数组之类的比较大小也是不能使用的，此时就需要重载
*/
template<typename T>
bool judge_max(T T1,T T2)
{
  if(T1>=T2) return true;
  else return false;
}

class temp_class_test{
  
public:
  int a;
  int b;
  temp_class_test(int _a,int _b):a(_a),b(_b) {}
};
bool judge_max(temp_class_test a1,temp_class_test a2)
{
  if(a1.a > a2.a) return true;
  else return false;
}


int main()
{
  std::cout <<"4>3?"<<judge_max(4,3)<<std::endl;
  std::cout <<"2>3?"<<judge_max(2,3)<<std::endl;
  temp_class_test a1(2,2);
  temp_class_test b1(3,3);
  std::cout <<"函数模板重载："<<judge_max(a1,b1)<<" "<<judge_max(b1,a1)<<std::endl;
}