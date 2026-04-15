/*
 * DMA Demo Application - DMA 内存到内存传输测试程序
 *
 * 功能：
 * 1. 通过 mmap 直接访问 DMA 缓冲区
 * 2. 向源缓冲区写入测试数据
 * 3. 触发 DMA 传输
 * 4. 验证目标缓冲区的数据
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>

#define DEVICE_PATH "/dev/dma_demo"
#define DMA_BUF_SIZE (4 * 1024)  /* 4KB 缓冲区 */

/* IOCTL 命令定义 - 必须与驱动一致 */
#define DMA_DEMO_MAGIC      'D'
#define DMA_DEMO_IOCTL_START    _IOW(DMA_DEMO_MAGIC, 0, struct dma_demo_xfer)
#define DMA_DEMO_IOCTL_GET_SRC  _IOR(DMA_DEMO_MAGIC, 1, unsigned long)
#define DMA_DEMO_IOCTL_GET_DST  _IOR(DMA_DEMO_MAGIC, 2, unsigned long)
#define DMA_DEMO_IOCTL_CLEAR    _IO(DMA_DEMO_MAGIC, 3)

/* DMA 传输参数结构体 */
struct dma_demo_xfer {
    size_t len;           /* 传输长度 */
    int verify;           /* 是否验证结果 */
};

/* 打印缓冲区内容 */
static void print_buffer(const char *name, uint8_t *buf, size_t len)
{
    int i;
    printf("%s (first %zu bytes):\n  ", name, len > 64 ? 64 : len);
    for (i = 0; i < len && i < 64; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n  ");
        }
    }
    if (len > 64) {
        printf("... (%zu more bytes)\n", len - 64);
    } else {
        printf("\n");
    }
}

/* 填充测试数据 - 每次加1的模式 */
static void fill_test_data(uint8_t *buf, size_t len)
{
    int i;
    for (i = 0; i < (int)len; i++) {
        buf[i] = (uint8_t)(i & 0xFF);
    }
}

/* 验证数据 */
static int verify_data(uint8_t *src, uint8_t *dst, size_t len)
{
    int i;
    int errors = 0;
    
    for (i = 0; i < (int)len; i++) {
        if (src[i] != dst[i]) {
            if (errors < 10) {
                printf("  Data mismatch at offset %d: expected 0x%02x, got 0x%02x\n",
                       i, src[i], dst[i]);
            }
            errors++;
        }
    }
    
    if (errors > 0) {
        printf("  Total errors: %d\n", errors);
        return -1;
    }
    
    return 0;
}

/* 测试1: 基本 DMA 传输测试 */
static int test_basic_xfer(int fd, uint8_t *src_buf, uint8_t *dst_buf)
{
    struct dma_demo_xfer xfer;
    int ret;
    size_t test_len = 16;  /* 测试16字节 */
    
    printf("\n=== Test 1: Basic DMA Transfer ===\n");
    
    /* 清空缓冲区 */
    memset(src_buf, 0, DMA_BUF_SIZE);
    memset(dst_buf, 0, DMA_BUF_SIZE);
    
    /* 填充测试数据 */
    fill_test_data(src_buf, test_len);
    
    printf("Before DMA transfer:\n");
    print_buffer("  Source", src_buf, test_len);
    print_buffer("  Dest  ", dst_buf, test_len);
    
    /* 启动 DMA 传输 */
    xfer.len = test_len;
    xfer.verify = 1;
    
    printf("Starting DMA transfer of %zu bytes...\n", test_len);
    ret = ioctl(fd, DMA_DEMO_IOCTL_START, &xfer);
    if (ret < 0) {
        perror("Failed to start DMA transfer");
        return ret;
    }
    
    printf("DMA transfer completed successfully!\n");
    
    /* 打印传输后的数据 */
    printf("After DMA transfer:\n");
    print_buffer("  Source", src_buf, test_len);
    print_buffer("  Dest  ", dst_buf, test_len);
    
    /* 验证数据 */
    ret = verify_data(src_buf, dst_buf, test_len);
    if (ret < 0) {
        printf("TEST FAILED: Data verification failed\n");
        return ret;
    }
    
    printf("TEST PASSED: Data verification successful\n");
    return 0;
}

/* 测试2: 4KB 完整传输测试 */
static int test_full_xfer(int fd, uint8_t *src_buf, uint8_t *dst_buf)
{
    struct dma_demo_xfer xfer;
    int ret;
    size_t test_len = DMA_BUF_SIZE;  /* 测试4KB */
    
    printf("\n=== Test 2: Full 4KB DMA Transfer ===\n");
    
    /* 清空缓冲区 */
    memset(src_buf, 0, DMA_BUF_SIZE);
    memset(dst_buf, 0, DMA_BUF_SIZE);
    
    /* 填充测试数据 */
    fill_test_data(src_buf, test_len);
    
    printf("Starting DMA transfer of %zu bytes (4KB)...\n", test_len);
    
    /* 启动 DMA 传输 */
    xfer.len = test_len;
    xfer.verify = 1;
    
    ret = ioctl(fd, DMA_DEMO_IOCTL_START, &xfer);
    if (ret < 0) {
        perror("Failed to start DMA transfer");
        return ret;
    }
    
    printf("DMA transfer completed successfully!\n");
    
    /* 验证数据 */
    ret = verify_data(src_buf, dst_buf, test_len);
    if (ret < 0) {
        printf("TEST FAILED: Data verification failed\n");
        return ret;
    }
    
    printf("TEST PASSED: Full 4KB transfer successful\n");
    return 0;
}

/* 测试3: 多次传输测试 */
static int test_multiple_xfer(int fd, uint8_t *src_buf, uint8_t *dst_buf)
{
    struct dma_demo_xfer xfer;
    int ret;
    int i;
    size_t test_len = 256;  /* 每次256字节 */
    int num_tests = 5;
    
    printf("\n=== Test 3: Multiple DMA Transfers ===\n");
    printf("Running %d transfers of %zu bytes each...\n", num_tests, test_len);
    
    for (i = 0; i < num_tests; i++) {
        /* 清空目标缓冲区 */
        memset(dst_buf, 0, DMA_BUF_SIZE);
        
        /* 填充不同的测试数据 */
        int j;
        for (j = 0; j < (int)test_len; j++) {
            src_buf[j] = (uint8_t)((i + j) & 0xFF);
        }
        
        /* 启动 DMA 传输 */
        xfer.len = test_len;
        xfer.verify = 0;  /* 应用层自己验证 */
        
        ret = ioctl(fd, DMA_DEMO_IOCTL_START, &xfer);
        if (ret < 0) {
            perror("Failed to start DMA transfer");
            return ret;
        }
        
        /* 验证数据 */
        ret = verify_data(src_buf, dst_buf, test_len);
        if (ret < 0) {
            printf("TEST FAILED: Transfer %d failed verification\n", i + 1);
            return ret;
        }
        
        printf("  Transfer %d/%d passed\n", i + 1, num_tests);
    }
    
    printf("TEST PASSED: All %d transfers successful\n", num_tests);
    return 0;
}

/* 测试4: 边界测试 */
static int test_boundary_xfer(int fd, uint8_t *src_buf, uint8_t *dst_buf)
{
    struct dma_demo_xfer xfer;
    int ret;
    size_t test_sizes[] = {1, 16, 64, 256, 1024, 4096};
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);
    int i;
    
    printf("\n=== Test 4: Boundary Size Tests ===\n");
    printf("Testing various transfer sizes...\n");
    
    for (i = 0; i < num_tests; i++) {
        size_t len = test_sizes[i];
        
        /* 清空缓冲区 */
        memset(src_buf, 0, DMA_BUF_SIZE);
        memset(dst_buf, 0, DMA_BUF_SIZE);
        
        /* 填充测试数据 */
        fill_test_data(src_buf, len);
        
        /* 启动 DMA 传输 */
        xfer.len = len;
        xfer.verify = 0;
        
        ret = ioctl(fd, DMA_DEMO_IOCTL_START, &xfer);
        if (ret < 0) {
            printf("  Size %zu: FAILED (ret=%d)\n", len, ret);
            return ret;
        }
        
        /* 验证数据 */
        ret = verify_data(src_buf, dst_buf, len);
        if (ret < 0) {
            printf("  Size %zu: FAILED verification\n", len);
            return ret;
        }
        
        printf("  Size %zu bytes: PASSED\n", len);
    }
    
    printf("TEST PASSED: All boundary tests successful\n");
    return 0;
}

/* 显示用法 */
static void usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -h         Show this help\n");
    printf("  -t <test>  Run specific test (0=all, 1=single)\n");
    printf("  -l <len>   Set transfer length (default: 16)\n");
    printf("\nExamples:\n");
    printf("  %s -t 1 -l 256    # Test 256 bytes transfer\n", prog);
    printf("  %s -t 0           # Run all tests\n", prog);
}

/* 简化测试：直接使用 ioctl 触发内核态 DMA 测试 */
static int test_kernel_dma(int fd, size_t len)
{
    struct dma_demo_xfer xfer;
    int ret;

    printf("\n=== Kernel DMA Transfer Test (%zu bytes) ===\n", len);

    xfer.len = len;
    xfer.verify = 1;

    printf("Starting DMA transfer...\n");
    ret = ioctl(fd, DMA_DEMO_IOCTL_START, &xfer);
    if (ret < 0) {
        perror("Failed to start DMA transfer");
        return ret;
    }

    printf("DMA transfer completed successfully!\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    int ret = 0;
    int test_num = 1;  /* 默认运行测试1 */
    int opt;
    size_t test_len = 16;

    /* 解析命令行参数 */
    while ((opt = getopt(argc, argv, "ht:l:")) != -1) {
        switch (opt) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 't':
            test_num = atoi(optarg);
            break;
        case 'l':
            test_len = atoi(optarg);
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    printf("=====================================\n");
    printf("    DMA Demo Test Application\n");
    printf("=====================================\n");

    /* 打开设备 */
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        printf("Make sure the driver is loaded: insmod dma_demo_driver.ko\n");
        return -1;
    }

    printf("Device opened: %s\n", DEVICE_PATH);

    /* 运行测试 */
    switch (test_num) {
    case 0:
        /* 运行多个不同大小的测试 */
        ret |= test_kernel_dma(fd, 16);
        ret |= test_kernel_dma(fd, 256);
        ret |= test_kernel_dma(fd, 1024);
        ret |= test_kernel_dma(fd, 4096);
        break;
    case 1:
        ret = test_kernel_dma(fd, test_len);
        break;
    default:
        printf("Invalid test number: %d\n", test_num);
        usage(argv[0]);
        ret = -1;
    }

    /* 清理 */
    close(fd);

    printf("\n=====================================\n");
    if (ret == 0) {
        printf("    ALL TESTS PASSED!\n");
    } else {
        printf("    SOME TESTS FAILED!\n");
    }
    printf("=====================================\n");

    return ret;
}
