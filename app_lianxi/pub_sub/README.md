```c
#include <stdint.h>
#include <stdbool.h>
#include "lwrb/lwrb.h"

#define MAX_TOPICS 20
#define MAX_SUBS_PER_TOPIC 10

// 订阅者结构（支持静态分配，适合嵌入式）
typedef struct {
    lwrb_t buf;                    // 环形缓冲区
    uint8_t* buffer;               // 指向缓冲区（可以是静态数组或malloc）
    size_t buffer_size;            // 缓冲区总大小
    size_t max_msg_size;           // 能处理的最大消息大小
    size_t queue_depth;            // 队列深度
    bool needs_retry;              // 队列写失败时设置为true
    uint32_t consecutive_failures; // 连续失败次数（用于健康检查）
    bool is_static;                // 标记是否为静态分配（用于销毁时判断是否free）
} subscriber_t;

// 主题结构
typedef struct {
    uint32_t topic_id;
    size_t msg_size;             // 主题的消息大小（固定，由发布者注册时指定）
    bool has_failed_subs;        // 快速判断：是否有订阅者需要重发
    subscriber_t* subscribers[MAX_SUBS_PER_TOPIC];
    int sub_count;               // 当前订阅者数量
} topic_t;

// 发布者结构
typedef struct {
    topic_t topics[MAX_TOPICS];
    int topic_count;
} topic_publisher_t;
```

// ========== API声明 ==========

// 初始化发布者
int topic_publisher_init(topic_publisher_t* pub);

// 注册主题（指定消息大小）
// topic_id: 主题ID
// msg_size: 该主题的消息大小（固定，所有订阅者都按此大小接收）
// 返回值：0成功，-1失败
int topic_publisher_register_topic(topic_publisher_t* pub, uint32_t topic_id, size_t msg_size);

// 获取主题的消息大小（用于订阅者创建时参考）
// 返回值：消息大小，0表示主题不存在
size_t topic_get_msg_size(topic_publisher_t* pub, uint32_t topic_id);

// 方式1：静态初始化订阅者（适合嵌入式，无malloc）
// sub: 订阅者结构指针（通常是全局变量或栈变量）
// buffer: 外部提供的缓冲区数组
// buffer_size: 缓冲区大小
// max_msg_size: 能处理的最大消息大小（应该 >= topic的msg_size）
// queue_depth: 队列深度（能缓存多少条消息）
// 返回值：0成功，-1失败
int subscriber_init_static(subscriber_t* sub, 
                           uint8_t* buffer, 
                           size_t buffer_size,
                           size_t max_msg_size,
                           size_t queue_depth);

// 方式2：动态创建订阅者（需要malloc/free）
// 返回值：成功返回subscriber_t指针，失败返回NULL
subscriber_t* subscriber_create_dynamic(size_t max_msg_size, size_t queue_depth);

// 销毁订阅者（如果是动态创建的，会free内存）
void subscriber_destroy(subscriber_t* sub);

// 订阅主题
int topic_subscribe(topic_publisher_t* pub, 
                    uint32_t topic_id,
                    subscriber_t* sub);

// 取消订阅
int topic_unsubscribe(topic_publisher_t* pub,
                      uint32_t topic_id,
                      subscriber_t* sub);

// 检查主题是否有需要重发的订阅者
bool topic_has_needs_retry(topic_publisher_t* pub, uint32_t topic_id);

// 发布消息到主题（可能部分订阅者失败）
// 返回值：失败的订阅者数量，0表示全部成功
int topic_publisher_push(topic_publisher_t* pub,
                         uint32_t topic_id,
                         const void* data, 
                         size_t size);

// 重发消息（只针对needs_retry为true的订阅者）
// 返回值：重发成功的数量
int topic_publisher_retry(topic_publisher_t* pub,
                          uint32_t topic_id,
                          const void* data, 
                          size_t size);



// ========== 使用示例 ==========

/*
#include <stdio.h>
#include <string.h>

// ========== 嵌入式静态分配示例（推荐）==========

// 定义消息结构（主题的数据结构是固定的）
typedef struct {
    float temperature;
    float humidity;
    uint32_t timestamp;
} weather_data_t;  // 12字节

typedef struct {
    float latitude;
    float longitude;
    float altitude;
    uint32_t timestamp;
} gps_data_t;  // 16字节

// 全局定义订阅者结构（静态分配）
subscriber_t temp_subscribers[3];
subscriber_t gps_subscribers[2];

// 全局定义缓冲区（静态数组）
// 温度主题：12字节消息 × 50条 = 600字节（向上取整到640）
uint8_t temp_buf_0[640];
uint8_t temp_buf_1[640];
uint8_t temp_buf_2[640];

// GPS主题：16字节消息 × 30条 = 480字节（向上取整到512）
uint8_t gps_buf_0[512];
uint8_t gps_buf_1[512];

int main() {
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    
    // 1. 注册主题（指定消息大小）
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));  // TOPIC_WEATHER
    topic_publisher_register_topic(&pub, 2, sizeof(gps_data_t));      // TOPIC_GPS
    
    // 2. 静态初始化订阅者
    // 订阅者根据主题的msg_size来配置自己的max_msg_size
    // 通常 max_msg_size >= topic.msg_size 即可
    
    // 温度订阅者：能处理12字节的消息，缓存50条
    subscriber_init_static(&temp_subscribers[0], 
                           temp_buf_0, 
                           sizeof(temp_buf_0),
                           sizeof(weather_data_t),  // max_msg_size = 12
                           50);                      // queue_depth
    
    subscriber_init_static(&temp_subscribers[1], 
                           temp_buf_1, 
                           sizeof(temp_buf_1),
                           sizeof(weather_data_t),
                           50);
    
    subscriber_init_static(&temp_subscribers[2], 
                           temp_buf_2, 
                           sizeof(temp_buf_2),
                           sizeof(weather_data_t),
                           50);
    
    // GPS订阅者：能处理16字节的消息，缓存30条
    subscriber_init_static(&gps_subscribers[0],
                           gps_buf_0,
                           sizeof(gps_buf_0),
                           sizeof(gps_data_t),  // max_msg_size = 16
                           30);                  // queue_depth
    
    subscriber_init_static(&gps_subscribers[1],
                           gps_buf_1,
                           sizeof(gps_buf_1),
                           sizeof(gps_data_t),
                           30);
    
    // 3. 订阅主题
    topic_subscribe(&pub, 1, &temp_subscribers[0]);
    topic_subscribe(&pub, 1, &temp_subscribers[1]);
    topic_subscribe(&pub, 1, &temp_subscribers[2]);
    
    topic_subscribe(&pub, 2, &gps_subscribers[0]);
    topic_subscribe(&pub, 2, &gps_subscribers[1]);
    
    // 4. 发布消息（消息大小必须与主题注册时一致）
    weather_data_t weather = {
        .temperature = 25.5f,
        .humidity = 60.0f,
        .timestamp = 1234567890
    };
    
    int failed = topic_publisher_push(&pub, 1, &weather, sizeof(weather_data_t));
    
    if (failed > 0) {
        printf("Detected %d failed subscribers\n", failed);
        
        // 5. 重发
        int retried = topic_publisher_retry(&pub, 1, &weather, sizeof(weather_data_t));
        printf("Retried: %d succeeded\n", retried);
    }
    
    // 6. GPS发布
    gps_data_t gps = {
        .latitude = 39.9042f,
        .longitude = 116.4074f,
        .altitude = 50.0f,
        .timestamp = 1234567890
    };
    
    topic_publisher_push(&pub, 2, &gps, sizeof(gps_data_t));
    
    return 0;
}
*/
// ========== 动态分配示例（通用系统）==========
/*
int main_dynamic() {
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    
    topic_publisher_register_topic(&pub, 1);
    
    // 动态创建订阅者（内部会malloc）
    subscriber_t* sub1 = subscriber_create_dynamic(64, 20);
    subscriber_t* sub2 = subscriber_create_dynamic(256, 20);
    
    if (!sub1 || !sub2) {
        printf("Failed to create subscribers\n");
        return -1;
    }
    
    topic_subscribe(&pub, 1, sub1);
    topic_subscribe(&pub, 1, sub2);
    
    // ... 使用 ...
    
    // 必须销毁（释放malloc的内存）
    subscriber_destroy(sub1);
    subscriber_destroy(sub2);
    
    return 0;
}
*/

// ========== 关键设计说明 ==========

/*
 * 1. SPSC安全性保证：
 *    - 只有Publisher调用 lwrb_write()
 *    - 只有Subscriber调用 lwrb_read()/lwrb_skip()
 *    - needs_retry标志由Publisher在write失败时设置
 * 
 * 2. 主题消息大小固定：
 *    - 注册主题时指定 msg_size（例如 sizeof(weather_data_t)）
 *    - 该主题的所有消息都必须是这个大小
 *    - 订阅者的 max_msg_size 应该 >= topic.msg_size
 *    - 发布时必须使用正确的大小：topic_publisher_push(pub, topic_id, &data, topic.msg_size)
 * 
 * 3. 重发机制工作流程：
 *    a) topic_publisher_push() 检查消息大小是否等于 topic.msg_size
 *    b) 遍历所有订阅者，检查 sub->max_msg_size >= size
 *    c) 对每个订阅者调用 lwrb_write()
 *    d) 如果失败，设置 sub->needs_retry = true
 *    e) 更新 topic->has_failed_subs = true
 *    f) 调用者检查返回值，决定是否需要重发
 *    g) topic_publisher_retry() 只重发 needs_retry=true 的订阅者
 *    h) 成功后清除 needs_retry 标志
 * 
 * 4. 线程安全：
 *    - Publisher线程：唯一写者，可以安全调用 push/retry
 *    - Subscriber线程：唯一读者，可以安全调用 read/skip
 *    - 不需要额外的锁（依赖lwrb的atomic操作）
 * 
 * 5. 静态 vs 动态分配：
 *    - 静态：subscriber_init_static()
 *      * 优点：无malloc，确定性高，适合嵌入式
 *      * 缺点：需要手动管理缓冲区数组
 *      * 使用场景：资源受限、实时性要求高的系统
 *    
 *    - 动态：subscriber_create_dynamic()
 *      * 优点：使用方便，自动管理内存
 *      * 缺点：有malloc开销，可能失败
 *      * 使用场景：Linux/Windows等通用系统
 * 
 * 6. 缓冲区大小计算：
 *    - buffer_size >= max_msg_size × queue_depth + 1
 *    - lwrb需要额外1字节用于区分满/空状态
 *    - 示例：max_msg=12, depth=50 → buffer_size >= 601（建议640）
 * 
 * 7. 注意事项：
 *    - 调用者需要保留数据副本用于重发
 *    - 重发应该尽快进行，避免订阅者队列继续积压
 *    - 可以结合连续失败次数实现健康检查
 *    - 如果连续失败太多，考虑移除该订阅者
 *    - 静态分配的订阅者不需要调用 subscriber_destroy()
 *    - 动态创建的订阅者必须调用 subscriber_destroy() 释放内存
 *    - 同一主题的消息大小必须一致（编译时确定）
 */
