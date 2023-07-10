#include <ctype.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
/*相关头文件的文件名中不得有扩展名*/

using namespace std;

//#define NULL_VIRTUAL_TEST

class base_type
{
	public:
		int a_public;
	public:
		base_type(int);
		base_type(void);
		~base_type(void);
		int set(int a)
			{
			this->a_public=a;//this 指针是个特殊的const 指针
			}
		#ifdef NULL_VIRTUAL_TEST
		virtual void print()=0;//纯虚函数要求派生类实现，否则派生类无法创建对象
		#else
		virtual void print() //虚函数是实现多态的关键
			{
			cout << "a_public"<<a_public << endl;
			}
		#endif
		
		static int s1;
		static void print_s(void)
			{
			cout << "s:"<< s1 << endl;//static 成员函数无法访问this以及非静态成员，静态成员属于类所有，而不是类对象
			}

		const int c1;//const 成员属于类对象所有，所以必须要在类构造函数中初始化，在类对象生命期内无法改变，但是不同类对象的const可能是不同的
	protected://注意不是 protect
		int a_protect;
	private:
		int a;
		
};

int base_type::s1=3;//静态成员的初始化为了和c语言区分，不需要定义的时候加static，声明的要加，需要加域操作符

/*构造函数和析构函数都没有返回值，析构函数没有入参(只有一个，无法重载)，构造函数可以有多个入参，允许多个构造函数
重载的条件是参数个数或者类型不同,多个构造函数编译器会自动根据参数情况选择
c++是格式强制检查，构造函数中的初始化列表和定义用:隔开，之间用，隔开
*/
base_type::base_type(void):a_public(1),a_protect(2),a(3),c1(0)
{
	cout<<"init base type"<<endl;
}
base_type::base_type(int c):a_public(1),a_protect(2),a(3),c1(c) 
{
	cout<<"init base type c1:"<<c1<<endl;
}

base_type::~base_type(void)
{
	cout<<"deinit base type"<<endl;
}

/*只有public 继承时，派生类才是基类的子类型，派生类的对象才可以作为基类的对象处理，从而实现多态*/
class less1:public base_type
{
	public:
		int a1;
		void print()
			{
			cout << "a1" << a1 << endl;
			}
};
class less2:public base_type
{
	public:
		int a1;
		void print()
			{
			cout << "a2" << a1 << endl;
			}
};


int main(int argc,char *argv[] )
{
	/*cout endl 是 std namespace的一部分，可以在文件头前面加上 using namespace std,
	那这里就可以省略域运算符 :: ,cin cout endl 只能作用于有格式的输入输出，无法用于文件操作
	endl:就是换行输出，
	*/
	std::cout << "hello"<<std::endl;
	/*static const*/
	#ifndef NULL_VIRTUAL_TEST
	base_type b0(6);
	base_type::print_s();
	#endif
    /*继承和多态测试*/
	less1 l1;
	less2 l2;	
	l2.print();
	base_type *b1=&l1;
	l1.print();
	l1.set(5);
	b1->print();
	
}
