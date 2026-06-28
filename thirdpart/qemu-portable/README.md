# 便携版 QEMU（免编译 / 免安装 / 免 sudo）

本目录是一个**完全独立、不污染系统**的 QEMU 运行环境。
通过解压 Ubuntu 20.04 (focal) 官方 `.deb` 包获得，未执行 `make install`，也未用 `dpkg -i` 安装。

- **版本**：QEMU 4.2.1 (Debian 1:4.2-3ubuntu6.30)
- **架构**：x86_64 系统态模拟（`qemu-system-x86_64`）
- **用途**：启动 Linux 内核 / 运行完整虚拟机

## 目录结构

```
qemu-portable/
├── run-qemu.sh          # qemu-system-x86_64 启动器（自动设置库路径与数据目录）
├── run-qemu-img.sh      # qemu-img 启动器（创建/转换磁盘镜像）
├── debs/                # 下载的原始 .deb 包（可删除，保留便于复现）
└── rootfs/              # 解压后的 QEMU 及其依赖库（实际运行目录）
    ├── usr/bin/         # qemu-system-x86_64, qemu-img 等可执行文件
    ├── usr/lib/...      # 依赖的 .so 库（spice/slirp/aio 等）
    └── usr/share/qemu/  # 固件（bios.bin / vgabios / pxe 等）
```

## 快速使用

### 方式 1：直接调用启动器

```bash
# 查看版本
/home/cwj/data1/linux/thirdpart/qemu-portable/run-qemu.sh --version

# 启动内核（无图形界面，串口输出到终端）
/home/cwj/data1/linux/thirdpart/qemu-portable/run-qemu.sh \
    -kernel /path/to/bzImage \
    -initrd /path/to/rootfs.cpio.gz \
    -append "console=ttyS0" \
    -nographic -m 512M
```

### 方式 2：设置别名（推荐，加到 `~/.bashrc`）

```bash
alias qemu-system-x86_64='/home/cwj/data1/linux/thirdpart/qemu-portable/run-qemu.sh'
alias qemu-img='/home/cwj/data1/linux/thirdpart/qemu-portable/run-qemu-img.sh'
```

之后即可像系统安装版一样直接用 `qemu-system-x86_64` 命令。

## 启动 Linux 内核的典型流程

```bash
# 1. 编译内核（在内核源码目录）
cd thirdpart/linux
make defconfig
make -j$(nproc)
# 产物：arch/x86/boot/bzImage

# 2. 制作根文件系统（用 busybox，见 thirdpart/busybox/）
# 产物：rootfs.cpio.gz

# 3. 用 QEMU 启动
qemu-system-x86_64 \
    -kernel arch/x86/boot/bzImage \
    -initrd rootfs.cpio.gz \
    -append "console=ttyS0" \
    -nographic -m 512M
```

## KVM 加速（可选）

若服务器 CPU 支持虚拟化，加 `-enable-kvm` 可大幅提速：

```bash
# 检查是否支持
egrep -c '(vmx|svm)' /proc/cpuinfo   # >0 表示支持
ls -l /dev/kvm                       # 存在则可用

# 使用加速
qemu-system-x86_64 -enable-kvm -kernel ... -initrd ... -nographic
```

> 注意：使用 KVM 需要对 `/dev/kvm` 有读写权限（通常需加入 `kvm` 组或用 sudo）。
> 无 KVM 时 QEMU 用 TCG 纯软件模拟，速度较慢但功能完整。

## 如何复现（重新构建本目录）

```bash
cd thirdpart/qemu-portable
mkdir -p debs && cd debs

# 下载 QEMU 及依赖的 .deb（Ubuntu 20.04 focal）
apt-get download qemu-system-x86 qemu-system-data qemu-system-common qemu-utils \
    libslang2 libaio1 libpmem1 libcacard0 libslirp0 libspice-server1 \
    libusbredirparser1 libvirglrenderer1 seabios ipxe-qemu

# 解压到 rootfs/
cd .. && mkdir -p rootfs
for d in debs/*.deb; do dpkg-deb -x "$d" rootfs/; done

# 链接 seabios 固件到 qemu 数据目录（改为复制，软链接在某些场景失效）
cp -f rootfs/usr/share/seabios/*.bin rootfs/usr/share/qemu/

# 复制 iPXE 网卡 ROM（否则报 failed to find romfile "efi-e1000.rom"）
cp -f rootfs/usr/lib/ipxe/qemu/*.rom rootfs/usr/share/qemu/
```

## 备注

- 本 QEMU 是 Ubuntu 打包版，版本较旧（4.2.1）。若需新版本特性，需源码编译（要求 Python≥3.9、libglib2.0-dev 等，可能需 sudo 装依赖）。
- `debs/` 目录可安全删除以节省空间（约 10MB），保留便于在其他机器复现。
