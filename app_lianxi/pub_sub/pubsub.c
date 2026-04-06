#include "pubsub.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ========== 内部辅助函数 ==========

static topic_t* find_topic(topic_publisher_t* pub, uint32_t topic_id) {
    if (!pub) {
        return NULL;
    }
    
    for (int i = 0; i < pub->topic_count; i++) {
        if (pub->topics[i].topic_id == topic_id) {
            return &pub->topics[i];
        }
    }
    
    return NULL;
}

// ========== API实现 ==========

int topic_publisher_init(topic_publisher_t* pub) {
    if (!pub) {
        return -1;
    }
    
    memset(pub, 0, sizeof(topic_publisher_t));
    return 0;
}

int topic_publisher_register_topic(topic_publisher_t* pub, uint32_t topic_id, size_t msg_size) {
    if (!pub || msg_size == 0) {
        return -1;
    }
    
    // 检查是否已存在
    for (int i = 0; i < pub->topic_count; i++) {
        if (pub->topics[i].topic_id == topic_id) {
            return -1;  // 主题已存在
        }
    }
    
    if (pub->topic_count >= MAX_TOPICS) {
        return -1;  // 主题数量超限
    }
    
    // 注册新主题
    topic_t* topic = &pub->topics[pub->topic_count];
    topic->topic_id = topic_id;
    topic->msg_size = msg_size;
    topic->has_failed_subs = false;
    topic->sub_count = 0;
    
    pub->topic_count++;
    
    return 0;
}

size_t topic_get_msg_size(topic_publisher_t* pub, uint32_t topic_id) {
    if (!pub) {
        return 0;
    }
    
    topic_t* topic = find_topic(pub, topic_id);
    if (topic) {
        return topic->msg_size;
    }
    
    return 0;
}

int subscriber_init_static(subscriber_t* sub, 
                           uint8_t* buffer, 
                           size_t buffer_size,
                           size_t max_msg_size,
                           size_t queue_depth) {
    if (!sub || !buffer || max_msg_size == 0 || queue_depth == 0) {
        return -1;
    }
    
    // 验证缓冲区大小是否足够（lwrb需要额外1字节）
    size_t required_size = max_msg_size * queue_depth + 1;
    if (buffer_size < required_size) {
        return -1;
    }
    
    // 初始化字段
    sub->buffer = buffer;
    sub->buffer_size = buffer_size;
    sub->max_msg_size = max_msg_size;
    sub->queue_depth = queue_depth;
    sub->needs_retry = false;
    sub->consecutive_failures = 0;
    sub->is_static = true;
    
    // 初始化lwrb
    lwrb_init(&sub->buf, buffer, buffer_size);
    
    return 0;
}

subscriber_t* subscriber_create_dynamic(size_t max_msg_size, size_t queue_depth) {
    if (max_msg_size == 0 || queue_depth == 0) {
        return NULL;
    }
    
    // 分配订阅者结构
    subscriber_t* sub = (subscriber_t*)malloc(sizeof(subscriber_t));
    if (!sub) {
        return NULL;
    }
    
    // 计算并分配缓冲区
    size_t buffer_size = max_msg_size * queue_depth + 1;
    sub->buffer = (uint8_t*)malloc(buffer_size);
    if (!sub->buffer) {
        free(sub);
        return NULL;
    }
    
    // 初始化字段
    sub->buffer_size = buffer_size;
    sub->max_msg_size = max_msg_size;
    sub->queue_depth = queue_depth;
    sub->needs_retry = false;
    sub->consecutive_failures = 0;
    sub->is_static = false;
    
    // 初始化lwrb
    lwrb_init(&sub->buf, sub->buffer, buffer_size);
    
    return sub;
}

void subscriber_destroy(subscriber_t* sub) {
    if (!sub) {
        return;
    }
    
    // 只释放动态分配的内存
    if (!sub->is_static && sub->buffer) {
        free(sub->buffer);
        sub->buffer = NULL;
    }
    
    // 重置标志
    sub->needs_retry = false;
    sub->consecutive_failures = 0;
}

int topic_subscribe(topic_publisher_t* pub, uint32_t topic_id, subscriber_t* sub) {
    if (!pub || !sub) {
        return -1;
    }
    
    topic_t* topic = find_topic(pub, topic_id);
    if (!topic) {
        return -1;
    }
    
    if (topic->sub_count >= MAX_SUBS_PER_TOPIC) {
        return -1;
    }
    
    // 检查是否已订阅
    for (int i = 0; i < topic->sub_count; i++) {
        if (topic->subscribers[i] == sub) {
            return 0;  // 已订阅
        }
    }
    
    // 添加订阅者
    topic->subscribers[topic->sub_count++] = sub;
    return 0;
}

int topic_unsubscribe(topic_publisher_t* pub, uint32_t topic_id, subscriber_t* sub) {
    if (!pub || !sub) {
        return -1;
    }
    
    topic_t* topic = find_topic(pub, topic_id);
    if (!topic) {
        return -1;
    }
    
    // 查找并移除订阅者
    for (int i = 0; i < topic->sub_count; i++) {
        if (topic->subscribers[i] == sub) {
            // 用最后一个元素填补
            topic->subscribers[i] = topic->subscribers[topic->sub_count - 1];
            topic->sub_count--;
            return 0;
        }
    }
    
    return -1;  // 未找到
}

bool topic_has_needs_retry(topic_publisher_t* pub, uint32_t topic_id) {
    if (!pub) {
        return false;
    }
    
    topic_t* topic = find_topic(pub, topic_id);
    if (!topic) {
        return false;
    }
    
    return topic->has_failed_subs;
}

int topic_publisher_push(topic_publisher_t* pub,
                         uint32_t topic_id,
                         const void* data, 
                         size_t size) {
    if (!pub || !data || size == 0) {
        return -1;
    }
    
    topic_t* topic = find_topic(pub, topic_id);
    if (!topic) {
        return -1;
    }
    
    // 检查消息大小是否与主题一致
    if (size != topic->msg_size) {
        return -1;
    }
    
    if (topic->sub_count == 0) {
        return 0;  // 没有订阅者，不算失败
    }
    
    int failed_count = 0;
    
    // 遍历所有订阅者
    for (int i = 0; i < topic->sub_count; i++) {
        subscriber_t* sub = topic->subscribers[i];
        
        // 检查订阅者是否能处理这个消息大小
        if (size > sub->max_msg_size) {
            sub->needs_retry = false;  // 重发也没用
            sub->consecutive_failures++;
            failed_count++;
            continue;
        }
        
        // 写入队列
        size_t written = lwrb_write(&sub->buf, data, size);
        
        if (written == size) {
            sub->needs_retry = false;
            sub->consecutive_failures = 0;
        } else {
            sub->needs_retry = true;
            sub->consecutive_failures++;
            failed_count++;
        }
    }
    
    // 更新快速标志
    topic->has_failed_subs = (failed_count > 0);
    
    return failed_count;
}

int topic_publisher_retry(topic_publisher_t* pub,
                          uint32_t topic_id,
                          const void* data, 
                          size_t size) {
    if (!pub || !data || size == 0) {
        return -1;
    }
    
    topic_t* topic = find_topic(pub, topic_id);
    if (!topic) {
        return -1;
    }
    
    if (size != topic->msg_size) {
        return -1;
    }
    
    int success_count = 0;
    
    // 只重发需要重试的订阅者
    for (int i = 0; i < topic->sub_count; i++) {
        subscriber_t* sub = topic->subscribers[i];
        
        if (!sub->needs_retry) {
            continue;  // 跳过不需要重试的
        }
        
        // 尝试重发
        size_t written = lwrb_write(&sub->buf, data, size);
        
        if (written == size) {
            sub->needs_retry = false;
            sub->consecutive_failures = 0;
            success_count++;
        }
    }
    
    // 更新快速标志
    bool still_has_failed = false;
    for (int i = 0; i < topic->sub_count; i++) {
        if (topic->subscribers[i]->needs_retry) {
            still_has_failed = true;
            break;
        }
    }
    topic->has_failed_subs = still_has_failed;
    
    return success_count;
}

int topic_get_stats(topic_publisher_t* pub, 
                    uint32_t topic_id,
                    int* total_subs,
                    int* failed_subs) {
    if (!pub || !total_subs || !failed_subs) {
        return -1;
    }
    
    topic_t* topic = find_topic(pub, topic_id);
    if (!topic) {
        return -1;
    }
    
    *total_subs = topic->sub_count;
    *failed_subs = 0;
    
    for (int i = 0; i < topic->sub_count; i++) {
        if (topic->subscribers[i]->needs_retry) {
            (*failed_subs)++;
        }
    }
    
    return 0;
}
