#include <fstream> //提供了文件输入输出相关的类和函数，用于文件操作
#include <iomanip> //提供了控制输入输出格式的工具，如设置输出精度、对齐方式等
#include <iostream> //提供了输入输出流的类和函数，包括 std::cin、std::cout
#include <cstdlib> //提供了杂项函数和类型，如字符串转换、随机数生成 system调用
#include <cstring> //提供了字符串处理相关的类和函数，包括 std::string 类和相关的操作函数
#include <vector> //提供了动态数组（向量）相关的类和函数，包括 std::vector 类
#include <string>
#include <algorithm>

#include <ctime>
#include <chrono> //时间相关的函数
/**
std::vector：动态数组，支持快速随机访问，可以动态添加和删除元素，适合需要频繁访问元素且大小不固定的情况。
std::list：双向链表，支持快速插入和删除操作，但不支持快速随机访问，适合在中间位置频繁插入或删除元素的情况。
std::deque：双端队列，支持在两端进行快速插入和删除操作，同时也支持随机访问，是一种折衷方案，适合需要在两端进行频繁操作的情况。
std::set：集合，内部元素自动排序，不允许重复元素存在，支持快速查找、插入和删除操作。
std::map：映射，存储键值对，根据键快速查找对应的值，不允许重复的键存在。
std::unordered_set：无序集合，与 std::set 类似，但内部元素无序存储，通过哈希表实现，查找、插入和删除的平均时间复杂度为 O(1)。
std::unordered_map：无序映射，与 std::map 类似，但内部元素无序存储，通过哈希表实现。
std::stack：栈，后进先出的数据结构，只允许在栈顶进行插入和删除操作。
std::queue：队列，先进先出的数据结构，只允许在队尾插入，在队头删除。
std::priority_queue：优先队列，内部元素按照一定规则（通常是元素大小）自动排序，可以快速获取最大或最小元素。
*/

void print(int v)
{
  std::cout<<v<<" "<<std::endl;
}
void vector_test(void)
{
  std::cout<<"----enter "<<__func__<<std::endl;
  std::vector<int> v1;
  v1.push_back(10);//push_back 默认是尾插
  v1.push_back(20);
  v1.push_back(30);
  std::cout<<"size:"<<v1.size()<<std::endl;
  std::for_each(v1.begin(),v1.end(),print);

  //std::vector<int>::iterator it=v1.begin();//显式的使用迭代器或者直接使用auto来推导
  auto it=v1.begin();
  //for(;it!=v1.end();it++)
  for(int x:v1)//C++ 引入了范围-based for循环，可以更简单的进行迭代
  {
      std::cout<<*it<<std::endl;
  }
  std::cout<<"----leave "<<__func__<<std::endl;
}
/**
 * string内部封装了动态分配的内存，减少内存泄漏和潜在的缓冲区溢出问题
 * 功能比char *更丰富
*/
void string_test(void)
{
  std::cout<<"----enter "<<__func__<<std::endl;
  std::string s1;
  s1.append("ok");
  s1.append("_ok");
  std::cout<<s1<<std::endl;
  std::cout<<"compare with ok,res="<<s1.compare("ok")<<std::endl;
  size_t pos = s1.find("vowel");
  if (pos != std::string::npos) {
      std::cout << "Found vowel at position: " << pos << std::endl;
  } else {
      std::cout << "No vowel found" << std::endl;
  }

  std::cout<<"----leave "<<__func__<<std::endl;
}
int main()
{
  vector_test();
  string_test();
}
