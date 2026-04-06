# PubSub 发布-订阅模块

基于lwrb环形缓冲区实现的轻量级发布-订阅模式，专为嵌入式系统设计。

## 特性

- ✅ **静态内存分配**：无malloc，适合资源受限的嵌入式系统
- ✅ **SPSC线程安全**：单生产者单消费者模型，无需额外锁
- ✅ **固定消息大小**：每个主题的消息结构在编译时确定
- ✅ **可靠重发机制**：自动跟踪失败的订阅者并支持重试
- ✅ **灵活配置**：支持静态和动态两种订阅者创建方式
- ✅ **完整测试**：56个测试用例覆盖所有功能

## 文件结构

```
pub_sub/
├── lwrb/              # lwrb环形缓冲区库
│   ├── lwrb.h
│   └── lwrb.c
├── pubsub.h           # 发布-订阅模块头文件
├── pubsub.c           # 发布-订阅模块实现
├── test_pubsub.c      # 测试用例
├── Makefile           # 编译配置
└── README.md          # 本文档
```

## 快速开始

### 编译和测试

```bash
make clean
make test
```

### 基本使用示例

```c
#include "pubsub.h"

// 1. 定义消息结构
typedef struct {
    float temperature;
    float humidity;
    uint32_t timestamp;
} weather_data_t;

// 2. 全局静态分配
topic_publisher_t publisher;
subscriber_t weather_subs[3];
uint8_t weather_buf_0[640];
uint8_t weather_buf_1[640];
uint8_t weather_buf_2[640];

// 3. 初始化
void system_init(void) {
    topic_publisher_init(&publisher);
    
    // 注册主题（指定消息大小）
    topic_publisher_register_topic(&publisher, 1, sizeof(weather_data_t));
    
    // 初始化订阅者
    subscriber_init_static(&weather_subs[0], weather_buf_0, 
                          sizeof(weather_buf_0),
                          sizeof(weather_data_t), 50);
    
    // 订阅主题
    topic_subscribe(&publisher, 1, &weather_subs[0]);
}

// 4. 发布消息
void publish_weather(void) {
    weather_data_t data = {
        .temperature = 25.5f,
        .humidity = 60.0f,
        .timestamp = get_timestamp()
    };
    
    int failed = topic_publisher_push(&publisher, 1, &data, sizeof(data));
    
    if (failed > 0) {
        // 重发给失败的订阅者
        topic_publisher_retry(&publisher, 1, &data, sizeof(data));
    }
}

// 5. 订阅者读取
void subscriber_task(void) {
    weather_data_t data;
    size_t bytes = lwrb_read(&weather_subs[0].buf, &data, sizeof(data));
    
    if (bytes == sizeof(data)) {
        printf("Temp: %.2f, Humidity: %.2f\n", 
               data.temperature, data.humidity);
    }
}
```

## API参考

### 发布者管理

```c
// 初始化发布者
int topic_publisher_init(topic_publisher_t* pub);

// 注册主题（指定消息大小）
int topic_publisher_register_topic(topic_publisher_t* pub, 
                                   uint32_t topic_id, 
                                   size_t msg_size);

// 获取主题消息大小
size_t topic_get_msg_size(topic_publisher_t* pub, uint32_t topic_id);
```

### 订阅者管理

```c
// 静态初始化（推荐用于嵌入式）
int subscriber_init_static(subscriber_t* sub, 
                           uint8_t* buffer, 
                           size_t buffer_size,
                           size_t max_msg_size,
                           size_t queue_depth);

// 动态创建（需要malloc/free）
subscriber_t* subscriber_create_dynamic(size_t max_msg_size, 
                                        size_t queue_depth);

// 销毁订阅者
void subscriber_destroy(subscriber_t* sub);
```

### 订阅管理

```c
// 订阅主题
int topic_subscribe(topic_publisher_t* pub, 
                    uint32_t topic_id,
                    subscriber_t* sub);

// 取消订阅
int topic_unsubscribe(topic_publisher_t* pub,
                      uint32_t topic_id,
                      subscriber_t* sub);
```

### 发布和重发

```c
// 发布消息
int topic_publisher_push(topic_publisher_t* pub,
                         uint32_t topic_id,
                         const void* data, 
                         size_t size);

// 重发消息（只发给失败的订阅者）
int topic_publisher_retry(topic_publisher_t* pub,
                          uint32_t topic_id,
                          const void* data, 
                          size_t size);

// 检查是否有需要重发的订阅者
bool topic_has_needs_retry(topic_publisher_t* pub, uint32_t topic_id);

// 获取统计信息
int topic_get_stats(topic_publisher_t* pub, 
                    uint32_t topic_id,
                    int* total_subs,
                    int* failed_subs);
```

## 设计要点

### 1. 消息大小固定

每个主题的消息大小在注册时确定，所有该主题的消息必须使用相同的大小：

```c
// 注册时指定
topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));

// 发布时必须使用正确的大小
topic_publisher_push(&pub, 1, &data, sizeof(weather_data_t));  // ✓
topic_publisher_push(&pub, 1, &data, 100);                      // ✗ 错误
```

### 2. SPSC线程安全

- **Publisher线程**：唯一调用`lwrb_write()`的线程
- **Subscriber线程**：唯一调用`lwrb_read()/lwrb_skip()`的线程
- **无需额外锁**：依赖lwrb的atomic操作保证线程安全

### 3. 重发机制

```
发布流程：
1. topic_publisher_push() 遍历所有订阅者
2. 对每个订阅者调用 lwrb_write()
3. 如果失败，设置 sub->needs_retry = true
4. 返回失败的订阅者数量

重发流程：
1. 调用者检测到有失败（返回值 > 0）
2. 从订阅者队列读取一些消息腾出空间
3. 调用 topic_publisher_retry() 重发
4. 成功后清除 needs_retry 标志
```

### 4. 缓冲区配置

```c
// 缓冲区大小计算公式
buffer_size >= max_msg_size × queue_depth + 1

// 示例：
// - 消息大小：12字节 (weather_data_t)
// - 队列深度：50条
// - 缓冲区：12 × 50 + 1 = 601字节（建议640字节留余量）
```

## 测试覆盖

运行 `make test` 执行56个测试用例：

- ✅ Publisher初始化
- ✅ Topic注册和管理
- ✅ Subscriber静态/动态创建
- ✅ 订阅/取消订阅
- ✅ 基本发布功能
- ✅ 消息大小检查
- ✅ 队列满处理
- ✅ 重发机制
- ✅ 多订阅者支持
- ✅ 部分失败重试
- ✅ 多主题管理

## 应用场景

### 物联网传感器网络

```c
// 温度、湿度、GPS等多个主题
topic_publisher_register_topic(&pub, TOPIC_TEMP, sizeof(float));
topic_publisher_register_topic(&pub, TOPIC_HUMID, sizeof(float));
topic_publisher_register_topic(&pub, TOPIC_GPS, sizeof(gps_data_t));

// 多个订阅者可以订阅不同主题
topic_subscribe(&pub, TOPIC_TEMP, &display_sub);
topic_subscribe(&pub, TOPIC_TEMP, &logger_sub);
topic_subscribe(&pub, TOPIC_GPS, &map_sub);
```

### 实时控制系统

```c
// 控制命令主题
typedef struct {
    uint16_t motor_speed;
    uint16_t servo_angle;
    uint8_t flags;
} control_cmd_t;

topic_publisher_register_topic(&pub, TOPIC_CONTROL, sizeof(control_cmd_t));

// 高优先级订阅者（深队列）
subscriber_init_static(&motor_ctrl, motor_buf, sizeof(motor_buf),
                      sizeof(control_cmd_t), 100);

// 低优先级订阅者（浅队列）
subscriber_init_static(&monitor, monitor_buf, sizeof(monitor_buf),
                      sizeof(control_cmd_t), 10);
```

## 注意事项

1. **消息大小一致性**：同一主题的所有消息必须大小相同
2. **保留数据副本**：调用者需要保留数据用于重发
3. **及时重发**：队列满后应尽快重发，避免继续积压
4. **静态vs动态**：嵌入式系统推荐使用静态分配
5. **缓冲区大小**：根据消息频率和处理延迟合理配置队列深度

## 性能特点

- **内存占用**：完全静态分配，编译时可计算
- **时间确定性**：无malloc/free，无动态扩容
- **并发性能**：无锁设计，基于atomic操作
- **扩展性**：最多20个主题，每个主题最多10个订阅者

## 许可证

本项目基于lwrb库实现，遵循相应的开源许可证。
