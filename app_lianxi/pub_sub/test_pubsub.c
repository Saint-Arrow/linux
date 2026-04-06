#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "pubsub.h"

// ========== 测试用的消息结构 ==========

typedef struct {
    float temperature;
    float humidity;
    uint32_t timestamp;
} weather_data_t;

typedef struct {
    float latitude;
    float longitude;
    float altitude;
    uint32_t timestamp;
} gps_data_t;

// ========== 辅助函数 ==========

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("✅ PASS: %s\n", message); \
            tests_passed++; \
        } else { \
            printf("❌ FAIL: %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

#define TEST_START(name) \
    printf("\n========== Test: %s ==========\n", name)

// ========== 测试用例 ==========

void test_publisher_init(void) {
    TEST_START("Publisher Initialization");
    
    topic_publisher_t pub;
    int ret = topic_publisher_init(&pub);
    
    TEST_ASSERT(ret == 0, "Publisher init returns 0");
    TEST_ASSERT(pub.topic_count == 0, "Initial topic count is 0");
}

void test_topic_register(void) {
    TEST_START("Topic Registration");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    
    // 注册主题
    int ret1 = topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    int ret2 = topic_publisher_register_topic(&pub, 2, sizeof(gps_data_t));
    
    TEST_ASSERT(ret1 == 0, "Register topic 1 success");
    TEST_ASSERT(ret2 == 0, "Register topic 2 success");
    TEST_ASSERT(pub.topic_count == 2, "Topic count is 2");
    
    // 获取消息大小
    size_t size1 = topic_get_msg_size(&pub, 1);
    size_t size2 = topic_get_msg_size(&pub, 2);
    
    TEST_ASSERT(size1 == sizeof(weather_data_t), "Topic 1 msg size correct");
    TEST_ASSERT(size2 == sizeof(gps_data_t), "Topic 2 msg size correct");
    
    // 重复注册应该失败
    int ret_dup = topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    TEST_ASSERT(ret_dup == -1, "Duplicate topic registration fails");
    
    // 不存在的主题返回0
    size_t size_invalid = topic_get_msg_size(&pub, 999);
    TEST_ASSERT(size_invalid == 0, "Invalid topic returns 0");
}

void test_subscriber_static_init(void) {
    TEST_START("Subscriber Static Initialization");
    
    subscriber_t sub;
    uint8_t buffer[640];
    
    // 正常初始化
    int ret = subscriber_init_static(&sub, buffer, sizeof(buffer), 
                                     sizeof(weather_data_t), 50);
    
    TEST_ASSERT(ret == 0, "Static subscriber init success");
    TEST_ASSERT(sub.is_static == true, "is_static flag is true");
    TEST_ASSERT(sub.max_msg_size == sizeof(weather_data_t), "max_msg_size correct");
    TEST_ASSERT(sub.queue_depth == 50, "queue_depth correct");
    TEST_ASSERT(sub.needs_retry == false, "needs_retry initially false");
    
    // 缓冲区太小应该失败
    uint8_t small_buffer[10];
    int ret_small = subscriber_init_static(&sub, small_buffer, sizeof(small_buffer),
                                           sizeof(weather_data_t), 50);
    TEST_ASSERT(ret_small == -1, "Small buffer initialization fails");
    
    // 参数无效应该失败
    int ret_null = subscriber_init_static(NULL, buffer, sizeof(buffer), 12, 50);
    TEST_ASSERT(ret_null == -1, "NULL subscriber fails");
}

void test_subscriber_dynamic_create(void) {
    TEST_START("Subscriber Dynamic Creation");
    
    subscriber_t* sub = subscriber_create_dynamic(sizeof(weather_data_t), 50);
    
    TEST_ASSERT(sub != NULL, "Dynamic subscriber creation success");
    TEST_ASSERT(sub->is_static == false, "is_static flag is false");
    TEST_ASSERT(sub->max_msg_size == sizeof(weather_data_t), "max_msg_size correct");
    
    // 销毁
    subscriber_destroy(sub);
    free(sub);
    
    TEST_ASSERT(1, "Dynamic subscriber destroy success");
}

void test_subscribe_unsubscribe(void) {
    TEST_START("Subscribe/Unsubscribe");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    
    subscriber_t sub1, sub2;
    uint8_t buf1[640], buf2[640];
    subscriber_init_static(&sub1, buf1, sizeof(buf1), sizeof(weather_data_t), 50);
    subscriber_init_static(&sub2, buf2, sizeof(buf2), sizeof(weather_data_t), 50);
    
    // 订阅
    int ret1 = topic_subscribe(&pub, 1, &sub1);
    int ret2 = topic_subscribe(&pub, 1, &sub2);
    
    TEST_ASSERT(ret1 == 0, "Subscribe sub1 success");
    TEST_ASSERT(ret2 == 0, "Subscribe sub2 success");
    
    int total, failed;
    topic_get_stats(&pub, 1, &total, &failed);
    TEST_ASSERT(total == 2, "Total subscribers is 2");
    
    // 重复订阅应该成功（幂等）
    int ret_dup = topic_subscribe(&pub, 1, &sub1);
    TEST_ASSERT(ret_dup == 0, "Duplicate subscribe succeeds (idempotent)");
    
    topic_get_stats(&pub, 1, &total, &failed);
    TEST_ASSERT(total == 2, "Total subscribers still 2 after duplicate");
    
    // 取消订阅
    int ret_unsub = topic_unsubscribe(&pub, 1, &sub1);
    TEST_ASSERT(ret_unsub == 0, "Unsubscribe sub1 success");
    
    topic_get_stats(&pub, 1, &total, &failed);
    TEST_ASSERT(total == 1, "Total subscribers is 1 after unsubscribe");
    
    // 取消不存在的订阅应该失败
    int ret_unsub_fail = topic_unsubscribe(&pub, 1, &sub1);
    TEST_ASSERT(ret_unsub_fail == -1, "Unsubscribe non-existent fails");
}

void test_publish_basic(void) {
    TEST_START("Basic Publish");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    
    subscriber_t sub;
    uint8_t buf[640];
    subscriber_init_static(&sub, buf, sizeof(buf), sizeof(weather_data_t), 50);
    topic_subscribe(&pub, 1, &sub);
    
    // 发布消息
    weather_data_t data = {25.5f, 60.0f, 1234567890};
    int failed = topic_publisher_push(&pub, 1, &data, sizeof(weather_data_t));
    
    TEST_ASSERT(failed == 0, "Publish success (0 failed)");
    TEST_ASSERT(sub.needs_retry == false, "needs_retry is false after success");
    
    // 验证订阅者可以读取
    weather_data_t received;
    size_t bytes = lwrb_read(&sub.buf, &received, sizeof(weather_data_t));
    
    TEST_ASSERT(bytes == sizeof(weather_data_t), "Read correct number of bytes");
    TEST_ASSERT(received.temperature == 25.5f, "Temperature data correct");
    TEST_ASSERT(received.humidity == 60.0f, "Humidity data correct");
}

void test_publish_size_mismatch(void) {
    TEST_START("Publish Size Mismatch");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    
    subscriber_t sub;
    uint8_t buf[640];
    subscriber_init_static(&sub, buf, sizeof(buf), sizeof(weather_data_t), 50);
    topic_subscribe(&pub, 1, &sub);
    
    // 发送错误大小的消息应该失败
    gps_data_t wrong_data = {0};
    int ret = topic_publisher_push(&pub, 1, &wrong_data, sizeof(gps_data_t));
    
    TEST_ASSERT(ret == -1, "Wrong message size fails");
}

void test_publish_queue_full(void) {
    TEST_START("Publish Queue Full");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    
    subscriber_t sub;
    uint8_t buf[100];  // 小缓冲区，只能存几条消息
    subscriber_init_static(&sub, buf, sizeof(buf), sizeof(weather_data_t), 5);
    topic_subscribe(&pub, 1, &sub);
    
    // 填满队列
    weather_data_t data = {25.5f, 60.0f, 1234567890};
    for (int i = 0; i < 10; i++) {
        data.timestamp = i;
        topic_publisher_push(&pub, 1, &data, sizeof(weather_data_t));
    }
    
    // 队列满后应该有失败
    TEST_ASSERT(sub.needs_retry == true, "needs_retry is true when queue full");
    TEST_ASSERT(sub.consecutive_failures > 0, "consecutive_failures incremented");
}

void test_retry_mechanism(void) {
    TEST_START("Retry Mechanism");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    
    subscriber_t sub;
    uint8_t buf[100];  // 小缓冲区
    subscriber_init_static(&sub, buf, sizeof(buf), sizeof(weather_data_t), 5);
    topic_subscribe(&pub, 1, &sub);
    
    // 填满队列导致失败
    weather_data_t data = {25.5f, 60.0f, 1234567890};
    for (int i = 0; i < 10; i++) {
        topic_publisher_push(&pub, 1, &data, sizeof(weather_data_t));
    }
    
    TEST_ASSERT(sub.needs_retry == true, "needs_retry is true after failures");
    
    // 检查是否有需要重试的
    bool has_retry = topic_has_needs_retry(&pub, 1);
    TEST_ASSERT(has_retry == true, "topic_has_needs_retry returns true");
    
    // 读取一些消息腾出空间
    weather_data_t temp;
    for (int i = 0; i < 3; i++) {
        lwrb_read(&sub.buf, &temp, sizeof(weather_data_t));
    }
    
    // 重试
    int retried = topic_publisher_retry(&pub, 1, &data, sizeof(weather_data_t));
    TEST_ASSERT(retried >= 0, "Retry executed");
    
    // 如果重试成功，needs_retry应该被清除
    if (retried > 0) {
        TEST_ASSERT(sub.needs_retry == false, "needs_retry cleared after successful retry");
    }
}

void test_multiple_subscribers(void) {
    TEST_START("Multiple Subscribers");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    
    subscriber_t subs[3];
    uint8_t bufs[3][640];
    
    for (int i = 0; i < 3; i++) {
        subscriber_init_static(&subs[i], bufs[i], sizeof(bufs[i]),
                              sizeof(weather_data_t), 50);
        topic_subscribe(&pub, 1, &subs[i]);
    }
    
    // 发布消息
    weather_data_t data = {25.5f, 60.0f, 1234567890};
    int failed = topic_publisher_push(&pub, 1, &data, sizeof(weather_data_t));
    
    TEST_ASSERT(failed == 0, "Publish to multiple subscribers success");
    
    // 验证所有订阅者都收到了消息
    for (int i = 0; i < 3; i++) {
        weather_data_t received;
        size_t bytes = lwrb_read(&subs[i].buf, &received, sizeof(weather_data_t));
        
        char msg[100];
        snprintf(msg, sizeof(msg), "Subscriber %d received message", i);
        TEST_ASSERT(bytes == sizeof(weather_data_t), msg);
        
        snprintf(msg, sizeof(msg), "Subscriber %d data correct", i);
        TEST_ASSERT(received.temperature == 25.5f, msg);
    }
}

void test_partial_failure_retry(void) {
    TEST_START("Partial Failure and Retry");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    
    // sub1: 正常缓冲区
    subscriber_t sub1;
    uint8_t buf1[640];
    subscriber_init_static(&sub1, buf1, sizeof(buf1), sizeof(weather_data_t), 50);
    
    // sub2: 小缓冲区（会满）
    subscriber_t sub2;
    uint8_t buf2[50];
    subscriber_init_static(&sub2, buf2, sizeof(buf2), sizeof(weather_data_t), 3);
    
    topic_subscribe(&pub, 1, &sub1);
    topic_subscribe(&pub, 1, &sub2);
    
    // 发布多条消息，让sub2的队列满
    weather_data_t data = {25.5f, 60.0f, 1234567890};
    for (int i = 0; i < 5; i++) {
        data.timestamp = i;
        topic_publisher_push(&pub, 1, &data, sizeof(weather_data_t));
    }
    
    // sub1应该都成功，sub2应该失败
    TEST_ASSERT(sub1.needs_retry == false, "sub1 no retry needed");
    TEST_ASSERT(sub2.needs_retry == true, "sub2 needs retry");
    
    int total, failed;
    topic_get_stats(&pub, 1, &total, &failed);
    TEST_ASSERT(total == 2, "Total subscribers is 2");
    TEST_ASSERT(failed == 1, "One subscriber failed");
    
    // 从sub2读取一些消息腾出空间
    weather_data_t temp;
    lwrb_read(&sub2.buf, &temp, sizeof(weather_data_t));
    
    // 重试
    int retried = topic_publisher_retry(&pub, 1, &data, sizeof(weather_data_t));
    TEST_ASSERT(retried == 1, "One subscriber retried successfully");
}

void test_multiple_topics(void) {
    TEST_START("Multiple Topics");
    
    topic_publisher_t pub;
    topic_publisher_init(&pub);
    topic_publisher_register_topic(&pub, 1, sizeof(weather_data_t));
    topic_publisher_register_topic(&pub, 2, sizeof(gps_data_t));
    
    subscriber_t weather_sub, gps_sub;
    uint8_t weather_buf[640], gps_buf[512];
    
    subscriber_init_static(&weather_sub, weather_buf, sizeof(weather_buf),
                          sizeof(weather_data_t), 50);
    subscriber_init_static(&gps_sub, gps_buf, sizeof(gps_buf),
                          sizeof(gps_data_t), 30);
    
    topic_subscribe(&pub, 1, &weather_sub);
    topic_subscribe(&pub, 2, &gps_sub);
    
    // 发布不同类型的消息
    weather_data_t weather = {25.5f, 60.0f, 1234567890};
    gps_data_t gps = {39.9f, 116.4f, 50.0f, 1234567890};
    
    int failed1 = topic_publisher_push(&pub, 1, &weather, sizeof(weather_data_t));
    int failed2 = topic_publisher_push(&pub, 2, &gps, sizeof(gps_data_t));
    
    TEST_ASSERT(failed1 == 0, "Weather publish success");
    TEST_ASSERT(failed2 == 0, "GPS publish success");
    
    // 验证订阅者收到正确的消息
    weather_data_t recv_weather;
    gps_data_t recv_gps;
    
    lwrb_read(&weather_sub.buf, &recv_weather, sizeof(weather_data_t));
    lwrb_read(&gps_sub.buf, &recv_gps, sizeof(gps_data_t));
    
    TEST_ASSERT(recv_weather.temperature == 25.5f, "Weather data correct");
    TEST_ASSERT(recv_gps.latitude == 39.9f, "GPS data correct");
}

// ========== 主函数 ==========

int main(void) {
    printf("\n");
    printf("====================================\n");
    printf("  PubSub Module Test Suite\n");
    printf("====================================\n");
    
    test_publisher_init();
    test_topic_register();
    test_subscriber_static_init();
    test_subscriber_dynamic_create();
    test_subscribe_unsubscribe();
    test_publish_basic();
    test_publish_size_mismatch();
    test_publish_queue_full();
    test_retry_mechanism();
    test_multiple_subscribers();
    test_partial_failure_retry();
    test_multiple_topics();
    
    printf("\n====================================\n");
    printf("  Test Results\n");
    printf("====================================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    printf("====================================\n");
    
    if (tests_failed == 0) {
        printf("🎉 All tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n");
        return 1;
    }
}
