# Busybox initramfs 文件系统

本目录使用 **gcc 静态编译 busybox**，并构建出一个可作为 Linux 内核 `initramfs` 启动的根文件系统镜像。

## 目录结构

```
thirdpart/busybox/
├── busybox-1.36.1.tar.bz2      # busybox 1.36.1 源码包
├── busybox-1.36.1/             # 解压后的源码目录 (含编译产物 busybox)
├── build_initramfs.sh          # 一键构建脚本
├── output/                     # 构建产物 (运行脚本后生成)
│   ├── rootfs/                 # 解包后的根文件系统目录
│   └── rootfs.cpio.gz          # initramfs 镜像 (供内核启动用)
└── README.md                   # 本说明文档
```

## 构建步骤

### 1. 环境依赖

```bash
# Ubuntu/Debian
sudo apt install build-essential libncurses-dev cpio bzip2
```

需要确认存在静态库 `/usr/lib/x86_64-linux-gnu/libc.a`（由 `libc6-dev` 提供）。

### 2. 一键构建

```bash
cd thirdpart/busybox
./build_initramfs.sh
```

脚本会自动完成三步：

| 步骤 | 说明 |
|------|------|
| [1/3] 编译 busybox | `make defconfig` → 开启 `CONFIG_STATIC=y` → `make -j$(nproc)` |
| [2/3] 构建根文件系统 | 安装 busybox applet、创建 `/dev` 节点、`/etc` 配置、`/init` 入口 |
| [3/3] 打包镜像 | `find . \| cpio -H newc -o \| gzip -9 > rootfs.cpio.gz` |

> 若 busybox 已编译过，可跳过编译步骤以加快速度：
> ```bash
> ./build_initramfs.sh skip_build
> ```

### 3. 构建产物

- **`output/rootfs.cpio.gz`**：initramfs 镜像（gzip 压缩的 newc 格式 cpio 归档），约 1.4 MB
- **`output/rootfs/`**：解包后的根文件系统目录，包含 408 个 busybox 命令

## 根文件系统内容

```
rootfs/
├── bin/        # busybox 及其 applet 符号链接 (ls, cat, sh, cp ...)
├── sbin/       # 系统命令 (init, ifconfig, mount, reboot ...)
├── usr/bin/    # 用户命令
├── usr/sbin/   # 用户系统命令
├── etc/
│   ├── fstab           # 文件系统挂载表
│   ├── inittab         # busybox init 配置
│   ├── passwd/group/shadow
│   ├── profile         # shell 环境变量
│   └── init.d/rcS      # 启动脚本
├── dev/        # 静态设备节点 (console, null, zero, tty ...)
├── proc/       # procfs 挂载点
├── sys/        # sysfs 挂载点
├── tmp/        # 临时文件
├── root/       # root 用户主目录
├── mnt/        # 挂载点
├── lib/        # 库目录 (静态编译, 实际为空)
├── var/log/    # 日志目录
├── init        # initramfs 入口脚本 (内核执行)
└── linuxrc     # -> bin/busybox
```

## 使用方法

### 配合 QEMU 启动测试

```bash
qemu-system-x86_64 \
    -kernel <path/to/bzImage> \
    -initrd thirdpart/busybox/output/rootfs.cpio.gz \
    -append "console=ttyS0 rdinit=/init" \
    -nographic
```

### 烧写到嵌入式设备

将 `rootfs.cpio.gz` 作为内核的 `initramfs`/`initrd` 一起打包，或解包后放入 NAND/eMMC 的根分区。

## 关键技术点

1. **静态编译**：开启 `CONFIG_STATIC=y`，busybox 静态链接 glibc，生成的 `busybox` 单文件约 2.6 MB，无动态库依赖，可在任意 Linux 系统运行。
2. **cpio newc 格式**：Linux 内核的 initramfs 解析器只识别 `newc`（SVR4）格式的 cpio 归档，因此打包时必须使用 `-H newc`。
3. **`/init` 入口**：内核解压 initramfs 后会执行根目录的 `/init`（需可执行权限），由它挂载 `/proc`、`/sys`、`/dev` 并启动 shell。
4. **静态设备节点**：`/dev/console`、`/dev/null` 等在 devtmpfs 挂载前就需要可用，因此预先用 `mknod` 创建。

## 重新配置 busybox

如需自定义 busybox 功能（菜单配置）：

```bash
cd thirdpart/busybox/busybox-1.36.1
make menuconfig      # 交互式菜单
# 确保 Settings -> Build Options -> Build static binary 选中
make -j$(nproc)
```

然后重新运行 `./build_initramfs.sh skip_build` 重新打包。
