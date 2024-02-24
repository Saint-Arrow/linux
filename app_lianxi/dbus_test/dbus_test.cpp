#include <fstream> //提供了文件输入输出相关的类和函数，用于文件操作
#include <iomanip> //提供了控制输入输出格式的工具，如设置输出精度、对齐方式等
#include <iostream> //提供了输入输出流的类和函数，包括 std::cin、std::cout
#include <cstdlib> //提供了杂项函数和类型，如字符串转换、随机数生成 system调用
#include <cstring> //提供了字符串处理相关的类和函数，包括 std::string 类和相关的操作函数
#include <vector> //提供了动态数组（向量）相关的类和函数，包括 std::vector 类


#include <ctime>
#include <chrono> //时间相关的函数

#include <dbus/dbus.h>

DBusConnection *test_conn=nullptr;

DBusConnection *
init_connection (DBusBusType type, const char *name)
{
  DBusConnection *connection;
  DBusError error = DBUS_ERROR_INIT;

  connection = dbus_bus_get (type, &error);

  if (connection == NULL)
    {
      fprintf (stderr, "Failed to connect to bus: %s: %s\n",
               error.name, error.message);
      dbus_error_free (&error);
      exit (1);
    }

    if (dbus_bus_request_name (connection, name, DBUS_NAME_FLAG_DO_NOT_QUEUE,
                                 NULL) != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
      {
        fprintf (stderr, "failed to take bus name %s\n", name);
        exit (1);
      }

      return connection;
}
static DBusHandlerResult
filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  //过滤器函数是用于拦截所有传入的消息，并根据特定的条件来决定是否将消息传递给消息处理函数
  //DBUS_HANDLER_RESULT_HANDLED 为已处理消息，可以确保消息只会处理一次
  //DBUS_HANDLER_RESULT_NOT_YET_HANDLED 代表消息处理函数可以接着处理
  if (dbus_message_get_type (message) != DBUS_MESSAGE_TYPE_METHOD_CALL)
    return DBUS_HANDLER_RESULT_HANDLED;

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void  DBusObjectPathUnregisterFunction_test(DBusConnection  *connection,
                                                                void            *user_data)
{
  if(nullptr != user_data)
    std::cout<<"Unregister "<<(char *)user_data<<std::endl;
}
DBusHandlerResult DBusObjectPathMessageFunction_test    (DBusConnection  *connection,
                                                                DBusMessage     *message,
                                                                void            *user_data)
{
  const char* member = dbus_message_get_member(message);
  const char *path = dbus_message_get_path(message);
  std::cout<<"path:"<<path<<" msg:"<<member<<std::endl;

  {
    DBusMessage *reply = dbus_message_new_method_return (message);

    if (reply == NULL)
      std::cout<<"allocating reply"<<std::endl;

    if (!dbus_connection_send (connection, reply, NULL))
      std::cout<<"sending reply"<<std::endl;

    dbus_message_unref (reply);
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}
/*注意xml信息中的换行*/
const char* introspection_xml = 
        "<node>\n"
        "  <interface name='test.method.server.Introspectable'>\n"
        "    <method name='Introspect'>\n"
        "      <arg name='data' direction='in' type='s'/>\n"
        "    </method>\n"
        "  </interface>\n"
        "</node>\n";
DBusHandlerResult DBusObjectPathMessageFunction_introspect    (DBusConnection  *connection,
                                                                DBusMessage     *message,
                                                                void            *user_data)
{
  DBusMessage *reply = dbus_message_new_method_return(message);

  // 添加字符串参数到消息中
  if (!dbus_message_append_args(reply, DBUS_TYPE_STRING, &introspection_xml, DBUS_TYPE_INVALID)) {
      // 错误处理
  }

  // 发送返回消息
  if (!dbus_connection_send(connection, reply, NULL)) {
      // 错误处理
  }

  // 释放消息
  dbus_message_unref(reply);
  return DBUS_HANDLER_RESULT_HANDLED;
}
int main()
{
  test_conn=init_connection(DBUS_BUS_SYSTEM,"test.method.server");
  if(nullptr != test_conn)
  {
    //添加过滤器函数
    dbus_connection_add_filter (test_conn, filter, NULL, NULL);

    //注册接口以及method和对应的处理函数
    DBusObjectPathVTable  vtable;
    vtable.message_function=DBusObjectPathMessageFunction_test;
    vtable.unregister_function=DBusObjectPathUnregisterFunction_test;
    dbus_connection_register_object_path(test_conn,"/test/method/server",&vtable,NULL);
    dbus_connection_register_object_path(test_conn,"/server1",&vtable,NULL);//总线名称和path不一定是一致的，是2个概念
    {//测试unregister_function回调的情况
    dbus_connection_register_object_path(test_conn,"/test/method/server2",&vtable,(void *)"user path string");
    std::cout<<"test api:dbus_connection_unregister_object_path "<<std::endl;
    dbus_connection_unregister_object_path(test_conn,"/test/method/server2");
    }
    
    // 实现 Introspectable 接口
    DBusObjectPathVTable  introspectable_vtable;
    introspectable_vtable.message_function=DBusObjectPathMessageFunction_introspect;
    //供给客户端使用的服务建议在 /org/freedesktop/DBus 上提供 Introspect 方法
    dbus_connection_register_fallback(test_conn, "/org/freedesktop/DBus", &introspectable_vtable,
                                      NULL);

    //消息分发
    while (dbus_connection_read_write_dispatch (test_conn, -1))
    {}

    dbus_connection_unref (test_conn);
  }
  else
  {
    std::cout<<"init_connection fail"<<std::endl;
  }
}