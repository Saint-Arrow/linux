
#include <fstream> //提供了文件输入输出相关的类和函数，用于文件操作
#include <iomanip> //提供了控制输入输出格式的工具，如设置输出精度、对齐方式等
#include <iostream> //提供了输入输出流的类和函数，包括 std::cin、std::cout
#include <cstdlib> //提供了杂项函数和类型，如字符串转换、随机数生成 system调用
#include <cstring> //提供了字符串处理相关的类和函数，包括 std::string 类和相关的操作函数
#include <vector> //提供了动态数组（向量）相关的类和函数，包括 std::vector 类
#include <algorithm>

#include <ctime>
#include <chrono> //时间相关的函数

//#include "sound/asound.h"
#include "alsa/asoundlib.h"

int tlv_test(snd_ctl_t *handle)
{
  int err=0;
  snd_ctl_elem_value_t *control;

  // 设置 elem id
  snd_ctl_elem_id_t *id;
  snd_ctl_elem_id_alloca(&id);
  snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
  snd_ctl_elem_id_set_name(id, "Analog In Volume");

  unsigned int tlv[64]={0};
  err =snd_ctl_elem_tlv_read(handle,id,tlv,sizeof(tlv));
  if (err < 0) {
      printf("无法读取控制元素 tlv\n");
      snd_ctl_close(handle);
      return -1;
  }
  std::cout<<"tlv: type:"<<tlv[0]<<" len:"<<tlv[1]<<std::endl;
  std::cout<<"value:"<<tlv[2]<<" "<<tlv[3]<<std::endl;//这里具体的含义和长度需要根据tlv的类型来确定

  {
    long min=0;
    long max=0;
    snd_tlv_get_dB_range(tlv,1,2,&min,&max);//该函数用于测试tlv的转换关系
    std::cout<<"snd_tlv_get_dB_range:"<<min<<"~"<<max<<std::endl;
  }
  
  //snd_ctl_elem_tlv_write(handle,id,tlv);
  //snd_ctl_elem_value_free(control);
}


int get_Volume(snd_ctl_t *handle)
{
  int err=0;
  snd_ctl_elem_value_t *control;

  // 设置 elem id
  snd_ctl_elem_id_t *id;
  snd_ctl_elem_id_alloca(&id);
  snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
  snd_ctl_elem_id_set_name(id, "Analog In Volume");//control的名称可以通过amixer controls来查看

  long min;
  long max;
  err =snd_ctl_get_dB_range(handle,id,&min,&max);
  if (err < 0) {
      printf("无法读取控制元素 tlv\n");
      snd_ctl_close(handle);
      return -1;
  }
  std::cout<<"range :"<<min<<" "<<max<<std::endl;

    // 分配 snd_ctl_elem_value_t 内存
    err = snd_ctl_elem_value_malloc(&control);
    if (err < 0) {
        printf("无法分配 snd_ctl_elem_value_t 内存\n");
        snd_ctl_close(handle);
        return -1;
    }
    // 将 elem id 与 snd_ctl_elem_value_t 关联起来
    snd_ctl_elem_value_set_id(control, id);

    err =snd_ctl_elem_read(handle,control);
    if (err < 0) {
        printf("无法读取控制元素\n");
        snd_ctl_close(handle);
        return -1;
    }
    //一个控制元素也是有多个值的，如左右声道的音量
    std::cout<<"Volume :"<<snd_ctl_elem_value_get_integer(control, 0)<<" "<<snd_ctl_elem_value_get_integer(control, 1)<<std::endl;



    // 释放内存并关闭 ALSA 控制器
    snd_ctl_elem_value_free(control);
    return 0;
}


int main(int argc,char *argv[])
{
  //std::cout<<"here"<<std::endl;
  int ret=0;
  
  snd_ctl_t *handle;
  ret=snd_ctl_open(&handle, "hw:0", 0);
  if(ret<0)
  {
    exit(-1);
  }
  
  tlv_test(handle);

  ret=get_Volume(handle);
  if(ret<0)
  {
    exit(-1);
  }
  

  
  // 关闭音频设备
  snd_ctl_close(handle);
}
