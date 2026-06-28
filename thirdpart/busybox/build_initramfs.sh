#!/bin/bash
#==============================================================================
# build_initramfs.sh
#
# 功能:
#   1. 使用 gcc 静态编译 busybox (若尚未编译)
#   2. 构建 initramfs 根文件系统目录结构
#      (/bin /sbin /etc /proc /sys /dev /usr ...)
#   3. 用 cpio + gzip 打包成 initramfs 镜像文件 (rootfs.cpio.gz)
#
# 用法:
#   ./build_initramfs.sh            # 完整流程: 编译 + 构建根fs + 打包
#   ./build_initramfs.sh skip_build # 跳过 busybox 编译, 仅构建根fs + 打包
#
# 产物:
#   output/rootfs.cpio.gz   -> 可作为内核 initramfs 启动的镜像
#   output/rootfs/          -> 解包后的根文件系统目录
#==============================================================================
set -e

#----------------------------- 路径与变量 -------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUSYBOX_SRC_DIR="${SCRIPT_DIR}/busybox-1.36.1"
BUSYBOX_BIN="${BUSYBOX_SRC_DIR}/busybox"
OUTPUT_DIR="${SCRIPT_DIR}/output"
ROOTFS_DIR="${OUTPUT_DIR}/rootfs"
INITRAMFS_IMG="${OUTPUT_DIR}/rootfs.cpio.gz"

#----------------------------- 工具检查 ---------------------------------------
for tool in gcc make cpio find; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "[ERROR] 缺少工具: $tool, 请先安装"
        exit 1
    fi
done

#----------------------------- 步骤1: 编译 busybox ----------------------------
build_busybox() {
    echo "[1/3] 编译静态 busybox ..."
    cd "$BUSYBOX_SRC_DIR"

    # 默认配置
    make defconfig >/dev/null

    # 开启静态编译
    if grep -q "^# CONFIG_STATIC is not set" .config; then
        sed -i 's/^# CONFIG_STATIC is not set/CONFIG_STATIC=y/' .config
    fi
    # tc 命令在某些内核头文件下编译失败, 这里禁用
    if grep -q "^CONFIG_TC=y" .config; then
        sed -i 's/^CONFIG_TC=y/# CONFIG_TC is not set/' .config
    fi

    make -j"$(nproc)" >/dev/null 2>&1

    if [ ! -x "$BUSYBOX_BIN" ]; then
        echo "[ERROR] busybox 编译失败"
        exit 1
    fi
    echo "      -> busybox 编译成功: $(file -b "$BUSYBOX_BIN" | cut -d, -f1-2)"
    cd "$SCRIPT_DIR"
}

#----------------------------- 步骤2: 构建根文件系统 --------------------------
build_rootfs() {
    echo "[2/3] 构建根文件系统目录 ..."
    rm -rf "$ROOTFS_DIR"
    mkdir -p "$ROOTFS_DIR"

    # 创建标准目录
    local dirs="bin sbin etc etc/init.d proc sys dev tmp root usr/bin usr/sbin var/log mnt lib"
    for d in $dirs; do
        mkdir -p "${ROOTFS_DIR}/${d}"
    done

    # 安装 busybox 及其所有 applet 的符号链接
    make -C "$BUSYBOX_SRC_DIR" CONFIG_PREFIX="$ROOTFS_DIR" install >/dev/null 2>&1

    # 创建必要的设备节点 (initramfs 由内核解包, 静态节点可保证早期可用)
    # 注意: mknod 需要权限, 失败不影响后续 cpio 打包
    mknod -m 622 "${ROOTFS_DIR}/dev/console" c 5 1 2>/dev/null || true
    mknod -m 666 "${ROOTFS_DIR}/dev/null"    c 1 3 2>/dev/null || true
    mknod -m 666 "${ROOTFS_DIR}/dev/zero"    c 1 5 2>/dev/null || true
    mknod -m 666 "${ROOTFS_DIR}/dev/tty"     c 5 0 2>/dev/null || true
    mknod -m 444 "${ROOTFS_DIR}/dev/random"  c 1 8 2>/dev/null || true
    mknod -m 444 "${ROOTFS_DIR}/dev/urandom" c 1 9 2>/dev/null || true

    # /etc/fstab
    cat > "${ROOTFS_DIR}/etc/fstab" <<'EOF'
# <device>  <mountpoint>  <type>  <options>  <dump>  <pass>
proc        /proc         proc    defaults   0       0
sysfs       /sys          sysfs   defaults   0       0
devtmpfs    /dev          devtmpfs defaults  0       0
tmpfs       /tmp          tmpfs   defaults   0       0
EOF

    # /etc/inittab (busybox init 配置)
    cat > "${ROOTFS_DIR}/etc/inittab" <<'EOF'
# busybox inittab
::sysinit:/etc/init.d/rcS
::respawn:/sbin/getty -L tty0 0 vt100
::ctrlaltdel:/sbin/reboot
::shutdown:/bin/umount -a -r
EOF

    # /etc/init.d/rcS 启动脚本
    cat > "${ROOTFS_DIR}/etc/init.d/rcS" <<'EOF'
#!/bin/sh
mount -a
echo "============================================"
echo "  Welcome to busybox initramfs (gcc build)  "
echo "============================================"
hostname busybox-rootfs
EOF
    chmod +x "${ROOTFS_DIR}/etc/init.d/rcS"

    # /etc/profile
    cat > "${ROOTFS_DIR}/etc/profile" <<'EOF'
export PATH=/bin:/sbin:/usr/bin:/usr/sbin
export PS1='busybox:\w# '
export HOME=/root
export TERM=linux
EOF

    # /etc/passwd /etc/group /etc/shadow
    cat > "${ROOTFS_DIR}/etc/passwd" <<'EOF'
root::0:0:root:/root:/bin/sh
EOF
    cat > "${ROOTFS_DIR}/etc/group" <<'EOF'
root:x:0:
EOF
    cat > "${ROOTFS_DIR}/etc/shadow" <<'EOF'
root::19000:0:99999:7:::
EOF
    chmod 600 "${ROOTFS_DIR}/etc/shadow"

    # /init 脚本 (initramfs 的入口, 内核会执行它)
    cat > "${ROOTFS_DIR}/init" <<'EOF'
#!/bin/sh
# initramfs 入口脚本
mount -t proc  none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev 2>/dev/null || true

echo ""
echo "============================================"
echo "  Busybox initramfs started successfully"
echo "  Built with gcc static linking"
echo "============================================"
echo ""

# 启动交互式 shell
exec /bin/sh
EOF
    chmod +x "${ROOTFS_DIR}/init"

    echo "      -> 根文件系统构建完成: ${ROOTFS_DIR}"
    echo "      -> busybox applet 数量: $(ls "${ROOTFS_DIR}/bin" "${ROOTFS_DIR}/sbin" "${ROOTFS_DIR}/usr/bin" "${ROOTFS_DIR}/usr/sbin" 2>/dev/null | wc -l)"
}

#----------------------------- 步骤3: 打包 cpio 镜像 --------------------------
pack_initramfs() {
    echo "[3/3] 打包 initramfs 镜像 ..."
    cd "$ROOTFS_DIR"
    # newc 格式是内核 initramfs 要求的 cpio 格式
    find . | cpio -H newc -o --owner root:root 2>/dev/null | gzip -9 > "$INITRAMFS_IMG"
    cd "$SCRIPT_DIR"
    echo "      -> 镜像生成完成: ${INITRAMFS_IMG}"
    echo "      -> 镜像大小: $(du -h "$INITRAMFS_IMG" | cut -f1)"
}

#----------------------------- 主流程 -----------------------------------------
echo "================================================"
echo "  Busybox initramfs 构建脚本"
echo "  源码: ${BUSYBOX_SRC_DIR}"
echo "  输出: ${OUTPUT_DIR}"
echo "================================================"

if [ "$1" != "skip_build" ]; then
    build_busybox
fi

build_rootfs
pack_initramfs

echo ""
echo "[完成] initramfs 文件系统构建成功!"
echo "  镜像文件: ${INITRAMFS_IMG}"
echo "  根目录:   ${ROOTFS_DIR}"
echo ""
echo "使用方法 (配合内核启动):"
echo "  qemu-system-x86_64 -kernel bzImage -initrd ${INITRAMFS_IMG} -append 'console=ttyS0' -nographic"
