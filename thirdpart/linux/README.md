# Linux 5.15 LTS 内核 (QEMU 测试环境)

本目录引用 **Linux 5.15 LTS** 长期维护内核源码，并提供一键编译 + QEMU 启动脚本，
配合 [`thirdpart/busybox`](../busybox/README.md) 构建的 initramfs 根文件系统，
即可在宿主机上完整地启动一个 Linux 内核，用于测试驱动、文件系统、内核特性等。

> **为什么选 5.15？** 5.15 是官方长期支持 (LTS) 版本，维护周期到 2026 年底，
> 同时也是 Ubuntu 22.04 的内核版本，资料丰富、兼容性好。

## 目录结构

```
thirdpart/linux/
├── linux-5.15.210.tar.gz        # 内核源码压缩包 (体积大, 已 gitignore)
├── linux-5.15.210/              # 解压后的内核源码 (已 gitignore)
├── build_kernel.sh              # 内核编译脚本
├── run_qemu.sh                  # QEMU 启动脚本 (内核 + busybox initramfs)
├── output/                      # 编译产物 (已 gitignore)
│   ├── arch/x86_64/boot/bzImage # 压缩内核镜像 (供 QEMU -kernel)
│   ├── vmlinux                  # 未压缩内核 (含调试符号, 供 gdb)
│   └── modules/                 # 内核模块 (可选)
├── .gitignore                   # 忽略源码本体/压缩包/编译产物
└── README.md                    # 本文档
```

> **版本控制说明**：内核源码本体（解压后约 1.3GB）和压缩包体积过大，
> 已通过 `.gitignore` 排除，**不会**提交到 git 仓库。仓库只跟踪
> `build_kernel.sh`、`run_qemu.sh`、`README.md`、`.gitignore` 这几个文件。
> 其他人 clone 仓库后，需按下方步骤自行获取内核源码。

## 一、获取内核源码

### 方式 A：使用已上传的压缩包（当前方式）

```bash
cd thirdpart/linux
tar -xzf linux-5.15.210.tar.gz      # 解压得到 linux-5.15.210/
```

### 方式 B：从官方仓库克隆（推荐用于获取最新补丁）

```bash
cd thirdpart/linux
# 浅克隆 5.15 LTS 维护分支 (只取最新一次提交, 速度快)
git clone --depth 1 --branch linux-5.15.y --single-branch \
    https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git linux-5.15.210
```

> `build_kernel.sh` 会自动识别 `linux-5.15.*` 目录，无需手动指定路径。

## 二、环境依赖

```bash
# Ubuntu / Debian
sudo apt install build-essential libncurses-dev bison flex \
                 libssl-dev libelf-dev bc cpio qemu-system-x86
```

| 工具 | 用途 |
|------|------|
| `build-essential` | gcc / make 编译内核 |
| `libncurses-dev` | `make menuconfig` 菜单配置 |
| `bison` `flex` | 内核构建工具链依赖 |
| `libssl-dev` `libelf-dev` `bc` | 内核配置/编译依赖 |
| `qemu-system-x86` | 运行 QEMU 启动内核 |

## 三、编译内核

```bash
cd thirdpart/linux
./build_kernel.sh              # 默认: defconfig + 编译 bzImage
```

脚本支持的子命令：

| 命令 | 说明 |
|------|------|
| `./build_kernel.sh` | 默认流程：x86_64 defconfig + 追加调试选项 + 编译 |
| `./build_kernel.sh menuconfig` | 打开菜单配置后编译（自定义功能） |
| `./build_kernel.sh modules` | 额外编译并安装内核模块到 `output/modules/` |
| `./build_kernel.sh clean` | 清理 `output/` 编译产物 |

默认开启的便利选项（在 defconfig 基础上）：

- `CONFIG_DEBUG_INFO`：生成调试符号，配合 gdb 调试内核
- `CONFIG_PRINTK_TIME`：dmesg 输出带时间戳
- `CONFIG_DEVTMPFS` + `CONFIG_DEVTMPFS_MOUNT`：自动挂载 devtmpfs
- `CONFIG_BLK_DEV_INITRD` + `CONFIG_RD_GZIP`：支持 gzip 压缩的 initramfs
- `CONFIG_VIRTIO_*`：virtio 块设备/网卡，配合 QEMU 高速 IO
- `CONFIG_EXT4_FS`：ext4 文件系统支持

编译产物：

- **`output/arch/x86_64/boot/bzImage`**：压缩内核镜像（约 10MB），供 QEMU `-kernel` 使用
- **`output/vmlinux`**：未压缩内核（含调试符号），供 gdb 调试

## 四、构建根文件系统（initramfs）

内核需要一个根文件系统才能启动。使用 [`thirdpart/busybox`](../busybox/README.md)
构建的 initramfs：

```bash
cd thirdpart/busybox
./build_initramfs.sh            # 生成 output/rootfs.cpio.gz
```

> busybox 静态编译，生成的 initramfs 无动态库依赖，可在任意内核上启动。

## 五、QEMU 启动测试

确保已编译内核（第三步）和 initramfs（第四步），然后：

```bash
cd thirdpart/linux
./run_qemu.sh                  # 默认串口模式, 直接在终端交互
```

启动后会出现 busybox shell 提示符 `busybox:/#`，可执行 `ls`、`cat /proc/version`、
`dmesg` 等命令测试内核。

`run_qemu.sh` 支持的模式：

| 模式 | 命令 | 说明 |
|------|------|------|
| 串口（默认） | `./run_qemu.sh` | `-nographic`，直接在当前终端交互，`Ctrl+A X` 退出 |
| 图形窗口 | `./run_qemu.sh graphic` | 弹出 QEMU 图形窗口 |
| GDB 调试 | `./run_qemu.sh gdb` | 暂停启动，等待 gdb 连接 `:1234` |
| 磁盘镜像 | `./run_qemu.sh disk` | 额外挂载 ext4 磁盘镜像，测试块设备文件系统 |

### GDB 调试内核

```bash
# 终端 1: 启动 QEMU (暂停在内核入口)
./run_qemu.sh gdb

# 终端 2: 连接 gdb
gdb thirdpart/linux/output/vmlinux
(gdb) target remote :1234
(gdb) break start_kernel        # 在内核入口下断点
(gdb) continue
```

### 测试块设备文件系统（ext4）

```bash
./run_qemu.sh disk
# 进入 busybox shell 后:
mount -t ext4 /dev/vda /mnt
ls /mnt
echo hello > /mnt/testfile      # 写入测试
umount /mnt
```

## 六、完整工作流（速查）

```bash
# 1. 编译内核
cd thirdpart/linux && ./build_kernel.sh

# 2. 构建 initramfs (若尚未构建)
cd ../busybox && ./build_initramfs.sh

# 3. 启动 QEMU
cd ../linux && ./run_qemu.sh
```

## 七、常见问题

**Q: 编译报错 `error: 'struct ...' has no member named ...`？**
A: 通常是工具链版本过旧。5.15 内核要求 gcc ≥ 5.1，建议使用 gcc 9/10/11。

**Q: QEMU 启动后卡住没有输出？**
A: 检查内核命令行是否包含 `console=ttyS0`（脚本已默认添加）。
   若用图形模式，确认内核开启了 `CONFIG_VGA_CONSOLE`。

**Q: 想测试自己写的驱动模块？**
A: 先 `./build_kernel.sh modules` 安装模块，把你的 `.ko` 放进 busybox rootfs，
   重新打包 initramfs，启动后 `insmod xxx.ko` 加载。

**Q: 如何更新到 5.15 的最新补丁？**
A: 见「方式 B」，`git -C linux-5.15.210 pull` 拉取 `linux-5.15.y` 分支最新提交。
