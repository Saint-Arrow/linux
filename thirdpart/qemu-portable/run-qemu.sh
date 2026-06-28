#!/bin/sh
# ============================================================
# 便携版 QEMU 启动器（免安装、免 sudo）
# 来源：Ubuntu 20.04 (focal) 的 qemu-system-x86 .deb 解压
# 版本：QEMU 4.2.1
#
# 用法：
#   ./run-qemu.sh --version
#   ./run-qemu.sh -kernel bzImage -initrd rootfs.cpio.gz -append "console=ttyS0" -nographic
#
# 或建立别名（加到 ~/.bashrc）：
#   alias qemu-system-x86_64='/home/cwj/data1/linux/thirdpart/qemu-portable/run-qemu.sh'
# ============================================================

# 定位本脚本所在目录（解析软链接）
SCRIPT_DIR="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
ROOTFS="$SCRIPT_DIR/rootfs"

# 设置库搜索路径（指向解压出来的本地 .so）
export LD_LIBRARY_PATH="$ROOTFS/usr/lib/x86_64-linux-gnu:$ROOTFS/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# 指定 QEMU 数据目录（bios/vgabios/pxe 等固件），否则启动会报 could not load PC BIOS
DATA_DIR="$ROOTFS/usr/share/qemu"

# 执行真正的 qemu-system-x86_64，用 -L 显式指定固件目录，透传其余参数
exec "$ROOTFS/usr/bin/qemu-system-x86_64" -L "$DATA_DIR" "$@"
