#ifndef PUBSUB_H
#define PUBSUB_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "lwrb/lwrb.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TOPICS 20
#define MAX_SUBS_PER_TOPIC 10

// ========== 数据结构定义 ==========

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
    size_t msg_size;               // 主题的消息大小（固定，由发布者注册时指定）
    bool has_failed_subs;          // 快速判断：是否有订阅者需要重发
    subscriber_t* subscribers[MAX_SUBS_PER_TOPIC];
    int sub_count;                 // 当前订阅者数量
} topic_t;

// 发布者结构
typedef struct {
    topic_t topics[MAX_TOPICS];
    int topic_count;
} topic_publisher_t;

// ========== API声明 ==========

/**
 * @brief 初始化发布者
 * @param pub 发布者指针
 * @return 0成功，-1失败
 */
int topic_publisher_init(topic_publisher_t* pub);

/**
 * @brief 注册主题（指定消息大小）
 * @param pub 发布者指针
 * @param topic_id 主题ID
 * @param msg_size 该主题的消息大小（固定，所有订阅者都按此大小接收）
 * @return 0成功，-1失败
 */
int topic_publisher_register_topic(topic_publisher_t* pub, uint32_t topic_id, size_t msg_size);

/**
 * @brief 获取主题的消息大小（用于订阅者创建时参考）
 * @param pub 发布者指针
 * @param topic_id 主题ID
 * @return 消息大小，0表示主题不存在
 */
size_t topic_get_msg_size(topic_publisher_t* pub, uint32_t topic_id);

/**
 * @brief 静态初始化订阅者（适合嵌入式，无malloc）
 * @param sub 订阅者结构指针
 * @param buffer 外部提供的缓冲区数组
 * @param buffer_size 缓冲区大小
 * @param max_msg_size 能处理的最大消息大小（应该 >= topic的msg_size）
 * @param queue_depth 队列深度（能缓存多少条消息）
 * @return 0成功，-1失败
 */
int subscriber_init_static(subscriber_t* sub, 
                           uint8_t* buffer, 
                           size_t buffer_size,
                           size_t max_msg_size,
                           size_t queue_depth);

/**
 * @brief 动态创建订阅者（需要malloc/free）
 * @param max_msg_size 能处理的最大消息大小
 * @param queue_depth 队列深度
 * @return 成功返回subscriber_t指针，失败返回NULL
 */
subscriber_t* subscriber_create_dynamic(size_t max_msg_size, size_t queue_depth);

/**
 * @brief 销毁订阅者（如果是动态创建的，会free内存）
 * @param sub 订阅者指针
 */
void subscriber_destroy(subscriber_t* sub);

/**
 * @brief 订阅主题
 * @param pub 发布者指针
 * @param topic_id 主题ID
 * @param sub 订阅者指针
 * @return 0成功，-1失败
 */
int topic_subscribe(topic_publisher_t* pub, 
                    uint32_t topic_id,
                    subscriber_t* sub);

/**
 * @brief 取消订阅
 * @param pub 发布者指针
 * @param topic_id 主题ID
 * @param sub 订阅者指针
 * @return 0成功，-1失败
 */
int topic_unsubscribe(topic_publisher_t* pub,
                      uint32_t topic_id,
                      subscriber_t* sub);

/**
 * @brief 检查主题是否有需要重发的订阅者
 * @param pub 发布者指针
 * @param topic_id 主题ID
 * @return true有失败的订阅者，false没有
 */
bool topic_has_needs_retry(topic_publisher_t* pub, uint32_t topic_id);

/**
 * @brief 发布消息到主题（可能部分订阅者失败）
 * @param pub 发布者指针
 * @param topic_id 主题ID
 * @param data 消息数据
 * @param size 消息大小（必须等于topic.msg_size）
 * @return 失败的订阅者数量，0表示全部成功，-1表示错误
 */
int topic_publisher_push(topic_publisher_t* pub,
                         uint32_t topic_id,
                         const void* data, 
                         size_t size);

/**
 * @brief 重发消息（只针对needs_retry为true的订阅者）
 * @param pub 发布者指针
 * @param topic_id 主题ID
 * @param data 消息数据
 * @param size 消息大小
 * @return 重发成功的数量，-1表示错误
 */
int topic_publisher_retry(topic_publisher_t* pub,
                          uint32_t topic_id,
                          const void* data, 
                          size_t size);

/**
 * @brief 获取主题统计信息
 * @param pub 发布者指针
 * @param topic_id 主题ID
 * @param total_subs 输出：总订阅者数量
 * @param failed_subs 输出：失败的订阅者数量
 * @return 0成功，-1失败
 */
int topic_get_stats(topic_publisher_t* pub, 
                    uint32_t topic_id,
                    int* total_subs,
                    int* failed_subs);

#ifdef __cplusplus
}
#endif

#endif // PUBSUB_H
