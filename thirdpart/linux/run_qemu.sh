#!/bin/bash
#==============================================================================
# run_qemu.sh
#
# 功能:
#   使用 QEMU 启动编译好的 Linux 内核, 并以 busybox 构建的 initramfs 作为
#   根文件系统, 用于在宿主机上测试内核 / 驱动 / 文件系统。
#
#   内核镜像 : thirdpart/linux/output/arch/x86_64/boot/bzImage
#   根文件系统: thirdpart/busybox/output/rootfs.cpio.gz
#
# 用法:
#   ./run_qemu.sh                 # 默认: 串口控制台 (-nographic)
#   ./run_qemu.sh graphic         # 图形窗口启动
#   ./run_qemu.sh gdb             # 启动并等待 gdb 连接 (端口 1234), 用于内核调试
#   ./run_qemu.sh disk            # 额外挂载一个 ext4 磁盘镜像 (测试块设备文件系统)
#
# QEMU 来源 (按优先级自动选择):
#   1. 便携版: thirdpart/qemu-portable/run-qemu.sh (免安装, 推荐, 无需 sudo)
#   2. 系统版: qemu-system-x86_64 (需 sudo apt install qemu-system-x86)
#
# 前置条件:
#   1. 已运行 thirdpart/linux/build_kernel.sh 生成 bzImage
#   2. 已运行 thirdpart/busybox/build_initramfs.sh 生成 rootfs.cpio.gz
#==============================================================================
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

#----------------------------- 路径定位 ---------------------------------------
BZIMAGE="$SCRIPT_DIR/output/arch/x86_64/boot/bzImage"
INITRAMFS="$REPO_ROOT/thirdpart/busybox/output/rootfs.cpio.gz"
DISK_IMG="$SCRIPT_DIR/output/disk.img"

#----------------------------- 选择 QEMU 可执行文件 ---------------------------
# 优先使用仓库内便携版 (免安装), 其次回退到系统安装的 qemu-system-x86_64
PORTABLE_QEMU="$REPO_ROOT/thirdpart/qemu-portable/run-qemu.sh"
PORTABLE_QEMU_IMG="$REPO_ROOT/thirdpart/qemu-portable/run-qemu-img.sh"

if [ -x "$PORTABLE_QEMU" ]; then
    QEMU_BIN="$PORTABLE_QEMU"
    QEMU_IMG_BIN="$PORTABLE_QEMU_IMG"
    echo "[QEMU] 使用便携版: $QEMU_BIN"
elif command -v qemu-system-x86_64 >/dev/null 2>&1; then
    QEMU_BIN="qemu-system-x86_64"
    QEMU_IMG_BIN="qemu-img"
    echo "[QEMU] 使用系统版: $QEMU_BIN"
else
    echo "[ERROR] 未找到 QEMU"
    echo "        方案1: 使用便携版 (推荐, 免安装) - 见 thirdpart/qemu-portable/README.md"
    echo "        方案2: 安装系统版 - sudo apt install qemu-system-x86"
    exit 1
fi

if [ ! -f "$BZIMAGE" ]; then
    echo "[ERROR] 未找到内核镜像: $BZIMAGE"
    echo "        请先运行: cd thirdpart/linux && ./build_kernel.sh"
    exit 1
fi

if [ ! -f "$INITRAMFS" ]; then
    echo "[ERROR] 未找到 initramfs: $INITRAMFS"
    echo "        请先运行: cd thirdpart/busybox && ./build_initramfs.sh"
    exit 1
fi

#----------------------------- 运行模式解析 -----------------------------------
MODE="${1:-serial}"

# 公共参数: 内核 + initramfs + 内核命令行
#   console=ttyS0   : 串口控制台输出
#   rdinit=/init    : initramfs 入口 (busybox 的 /init)
#   nokaslr         : 关闭内核地址随机化, 便于调试
KERNEL_CMDLINE="console=ttyS0 rdinit=/init nokaslr"

QEMU_COMMON=(
    -kernel "$BZIMAGE"
    -initrd "$INITRAMFS"
    -append "$KERNEL_CMDLINE"
    -m 512M
    -smp 2
)

case "$MODE" in
    serial|"")
        # 纯串口模式, 直接在当前终端交互
        echo "[启动] 串口模式 (-nographic)"
        exec "$QEMU_BIN" \
            "${QEMU_COMMON[@]}" \
            -nographic
        ;;
    graphic)
        # 图形窗口模式
        echo "[启动] 图形模式"
        exec "$QEMU_BIN" \
            "${QEMU_COMMON[@]}" \
            -serial mon:stdio
        ;;
    gdb)
        # 调试模式: -S 暂停启动, -gdb tcp::1234 等待 gdb 连接
        echo "[启动] GDB 调试模式"
        echo "  在另一个终端运行:"
        echo "    gdb $SCRIPT_DIR/output/vmlinux"
        echo "    (gdb) target remote :1234"
        echo "    (gdb) continue"
        exec "$QEMU_BIN" \
            "${QEMU_COMMON[@]}" \
            -nographic \
            -S -gdb tcp::1234
        ;;
    disk)
        # 挂载磁盘镜像模式: 用于测试 ext4 等块设备文件系统
        if [ ! -f "$DISK_IMG" ]; then
            echo "[准备] 创建 ext4 磁盘镜像: $DISK_IMG (64MB)"
            mkdir -p "$(dirname "$DISK_IMG")"
            "$QEMU_IMG_BIN" create -f raw "$DISK_IMG" 64M >/dev/null
            mkfs.ext4 -q "$DISK_IMG"
        fi
        echo "[启动] 磁盘镜像模式 (virtio-blk, /dev/vda)"
        echo "  进入系统后可执行: mount -t ext4 /dev/vda /mnt"
        exec "$QEMU_BIN" \
            "${QEMU_COMMON[@]}" \
            -nographic \
            -drive file="$DISK_IMG",format=raw,if=virtio
        ;;
    *)
        echo "用法: $0 {serial|graphic|gdb|disk}"
        echo "  serial  (默认) 串口控制台, 直接在终端交互"
        echo "  graphic         图形窗口启动"
        echo "  gdb             暂停启动并等待 gdb 连接 (端口 1234)"
        echo "  disk            额外挂载 ext4 磁盘镜像, 测试块设备文件系统"
        exit 1
        ;;
esac
