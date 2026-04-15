# 概述
DMA Demo 测试项目，支持在 Linux 下编译以及测试。目前针对 HI3559 平台进行了专门的适配和优化。

## 目的
熟悉 DMA 的基本 API 以及框架，支持多平台 DMA 适配。

## 通用 API 说明

### DMA 引擎 API
| API 类别 | API 函数 | 主要作用 |
| :--- | :--- | :--- |
| 内存管理 | `dma_alloc_coherent` | 分配CPU和设备共享的一致性内存 |
| | `dma_map_single` | 将单块内存映射为DMA可用的总线地址 |
| | `dma_map_sg` | 将分散的内存列表映射为DMA可用的地址 |
| | `dma_unmap_single` | 解除单块内存的DMA映射 |
| | `dma_unmap_sg` | 解除分散内存列表的DMA映射 |
| | `dma_free_coherent` | 释放一致性内存 |
| 缓存同步 | `dma_sync_*` | 手动同步CPU缓存与DMA内存 |
| 通道管理 | `dma_request_chan` | 请求一个DMA通道 |
| | `dma_release_channel` | 释放DMA通道 |
| 传输控制 | `dmaengine_slave_config` | 配置DMA传输参数 |
| | `dmaengine_prep_*` | 准备DMA传输描述符 |
| | `dmaengine_submit` | 提交传输任务到队列 |
| | `dma_async_issue_pending` | 启动DMA传输 |

---

## 平台适配指南

### HI3559 海思平台

#### 专用 API
| API 函数 | 主要作用 |
| :--- | :--- |
| `dmac_channel_allocate` | 分配海思DMA通道 |
| `dmac_channel_free` | 释放海思DMA通道 |
| `dmac_start_m2m` | 启动内存到内存DMA传输 |
| `dmac_wait` | 等待DMA传输完成 |

海思还支持lli的dma模式，即多段内存组成链表结构，dma一直搬运到null为止,可以参考HIEDMA的驱动代码，默认不对接dma框架

#### 关键设计要点

##### 1. 内存分配
使用 `alloc_pages()` 分配物理连续的页面，确保 DMA 控制器能够访问。

##### 2. 缓存同步（关键）
对于 M2M DMA，必须手动同步 CPU 缓存：
```c
// 传输前：clean 源缓冲区，将 CPU 缓存写回内存
__flush_dcache_area(src_cpu_addr, len);

// 传输后：invalidate 目标缓冲区，使 CPU 缓存失效
__flush_dcache_area(dst_cpu_addr, len);
```

##### 3. 物理地址传递
HI3559 平台 DMA 控制器直接访问物理地址，**不需要** `dma_map_page()`：
```c
// 错误做法：dma_map_page() 会失败或返回错误地址
dma_addr_t dma_addr = dma_map_page(dev, page, 0, len, DMA_TO_DEVICE);

// 正确做法：直接使用 page_to_phys()
unsigned long phys_addr = page_to_phys(page);
dmac_start_m2m(channel, src_phys, dst_phys, len);
```

##### 4. 等待传输完成
使用海思专用接口，不要使用 Linux completion 机制：
```c
// 错误做法：使用 completion 等待中断回调
wait_for_completion(&dma_done);

// 正确做法：使用海思 dmac_wait()
dmac_wait(channel);
```

#### HI3559 问题排查记录

##### 问题1: mmap 返回 "No such device or address"
**原因**: 驱动使用 `dma_mmap_coherent()` 映射通过 `kmalloc()` 分配的缓冲区，这是不正确的。

**解决**: 将 `kmalloc()` 改为 `alloc_pages()` 分配物理连续页面，并使用 `remap_pfn_range()` 进行 mmap 映射。

##### 问题2: mmap 成功但访问出现 Bus error（未解决，已绕过）
**原因**: 尝试了多种方案（`SetPageReserved()`、`vm_insert_page()` 等），但 mmap 后用户态访问仍出现 Bus error。根本原因可能是 HI3559 平台对 mmap 的物理内存有特殊要求，或页表映射方式不正确。

**解决方案**: 放弃 mmap 用户态访问方案，改为**内核态数据填充和校验**：
- 驱动在 `ioctl` 中直接填充源缓冲区测试数据
- 触发 DMA 传输
- 驱动直接校验目标缓冲区数据
- 应用层仅通过 `ioctl` 触发测试，不直接访问 DMA 缓冲区

##### 问题3: DMA 传输超时
**原因**: 海思 `dmac_start_m2m` 是异步接口，但代码使用了 completion 等待机制，而中断回调未正确触发。

**解决**: 使用海思专用的 `dmac_wait()` 接口等待传输完成。

##### 问题4: DMA 传输完成但数据未正确传输（目标缓冲区全为0）
**原因**: CPU 缓存与 DMA 内存不一致。CPU 通过虚拟地址填充的数据还在 CPU 缓存中，没有写回到物理内存，DMA 控制器读取的是旧的物理内存数据。

**解决**: 
- **传输前**: 调用 `__flush_dcache_area(src, len)` 将源缓冲区的 CPU 缓存数据写回内存
- **传输后**: 调用 `__flush_dcache_area(dst, len)` 使目标缓冲区的 CPU 缓存失效，确保 CPU 能读取 DMA 传输的数据

##### 问题5: `dma_map_page()` 失败
**原因**: 字符设备没有设置 DMA 掩码，且 HI3559 平台 DMA 控制器可以直接访问物理地址，不需要通过 DMA API 映射。

**解决**: 直接使用 `page_to_phys()` 获取物理地址传给 `dmac_start_m2m`，不使用 `dma_map_page()`。

---

## 使用方法

### 编译
```bash
make clean && make
```

### 加载驱动
```bash
insmod dma_demo_driver.ko
```

### 运行测试
```bash
# 测试256字节传输
./dma_demo_app -t 1 -l 256

# 运行所有测试
./dma_demo_app -t 0
```

