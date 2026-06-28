#!/bin/bash
#==============================================================================
# build_kernel.sh
#
# 功能:
#   编译 Linux 5.15 LTS 内核, 生成可供 QEMU 启动的 bzImage 镜像。
#
#   默认面向 x86_64 架构, 使用 defconfig 并额外开启若干便于调试的选项
#   (debug_info, printk 时间戳, devtmpfs 自动挂载等)。
#
# 用法:
#   ./build_kernel.sh                # 完整流程: 配置 + 编译
#   ./build_kernel.sh menuconfig     # 先打开菜单配置, 再编译
#   ./build_kernel.sh clean          # 清理编译产物
#   ./build_kernel.sh modules        # 额外编译并安装模块到 output/modules
#
# 产物:
#   output/arch/x86_64/boot/bzImage  -> 压缩内核镜像 (供 QEMU -kernel 使用)
#   output/vmlinux                   -> 未压缩内核 (含调试符号, 供 gdb 调试)
#   output/modules/                  -> 编译出的内核模块 (make modules_install)
#==============================================================================
set -e

#----------------------------- 路径与变量 -------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 自动查找内核源码目录: 优先 linux-5.15.*, 其次 linux-stable / linux
KERNEL_SRC_DIR=""
for d in "$SCRIPT_DIR"/linux-5.15.* "$SCRIPT_DIR"/linux-stable "$SCRIPT_DIR"/linux; do
    if [ -f "$d/Makefile" ] && grep -q '^VERSION = 5' "$d/Makefile" 2>/dev/null; then
        KERNEL_SRC_DIR="$d"
        break
    fi
done

if [ -z "$KERNEL_SRC_DIR" ]; then
    echo "[ERROR] 未找到 Linux 5.x 内核源码目录"
    echo "        请将内核源码解压到: $SCRIPT_DIR/linux-5.15.x/"
    echo "        或参考 README.md 获取源码"
    exit 1
fi

# 编译输出独立目录, 避免污染源码树
BUILD_DIR="${SCRIPT_DIR}/output"
JOBS="$(nproc)"

#----------------------------- 工具检查 ---------------------------------------
for tool in gcc make flex bison; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "[ERROR] 缺少工具: $tool"
        echo "        Ubuntu/Debian 安装: sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev bc"
        exit 1
    fi
done

#----------------------------- 子函数 -----------------------------------------
do_clean() {
    echo "[清理] 删除编译输出目录 ..."
    rm -rf "$BUILD_DIR"
    echo "      -> 已清理: $BUILD_DIR"
}

# 在 defconfig 基础上追加便于调试/运行的选项
tune_config() {
    local cfg="$BUILD_DIR/.config"
    echo "[配置] 调整内核配置 ..."
    # 追加选项 (scripts/config 会自动处理依赖)
    #
    # 注意: 以下选项被禁用以避免依赖 libelf-dev (objtool / BTF / ORC):
    #   - CONFIG_STACK_VALIDATION  : objtool 栈校验, 依赖 libelf
    #   - CONFIG_DEBUG_INFO_BTF    : BPF Type Format, resolve_btfids 依赖 libelf
    #   - CONFIG_UNWINDER_ORC      : ORC unwinder, 由 objtool 生成, 依赖 libelf
    # 改用 CONFIG_UNWINDER_FRAME_POINTER (帧指针栈回溯) 替代 ORC, 无需 libelf。
    "$KERNEL_SRC_DIR/scripts/config" \
        --file "$cfg" \
        --enable  CONFIG_DEBUG_INFO \
        --enable  CONFIG_DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT \
        --enable  CONFIG_PRINTK_TIME \
        --enable  CONFIG_DEVTMPFS \
        --enable  CONFIG_DEVTMPFS_MOUNT \
        --enable  CONFIG_BLK_DEV_INITRD \
        --enable  CONFIG_RD_GZIP \
        --enable  CONFIG_VIRTIO \
        --enable  CONFIG_VIRTIO_PCI \
        --enable  CONFIG_VIRTIO_BLK \
        --enable  CONFIG_VIRTIO_NET \
        --enable  CONFIG_EXT4_FS \
        --enable  CONFIG_PROC_FS \
        --enable  CONFIG_SYSFS \
        --enable  CONFIG_TMPFS \
        --disable CONFIG_STACK_VALIDATION \
        --disable CONFIG_DEBUG_INFO_BTF \
        --disable CONFIG_UNWINDER_ORC \
        --enable  CONFIG_UNWINDER_FRAME_POINTER \
        2>/dev/null || true
    # 解决依赖并应用
    make -C "$KERNEL_SRC_DIR" O="$BUILD_DIR" olddefconfig >/dev/null 2>&1
}

do_config() {
    echo "[1/2] 配置内核 (x86_64 defconfig) ..."
    mkdir -p "$BUILD_DIR"
    make -C "$KERNEL_SRC_DIR" O="$BUILD_DIR" x86_64_defconfig >/dev/null 2>&1
    tune_config
    echo "      -> 配置完成: $BUILD_DIR/.config"
    echo "      -> 内核版本: $(make -C "$KERNEL_SRC_DIR" O="$BUILD_DIR" kernelversion 2>/dev/null)"
}

do_build() {
    echo "[2/2] 编译内核 (使用 ${JOBS} 个核心) ..."
    echo "      源码: $KERNEL_SRC_DIR"
    echo "      输出: $BUILD_DIR"
    make -C "$KERNEL_SRC_DIR" O="$BUILD_DIR" -j"$JOBS" bzImage 2>&1 | tail -n 20

    local bzimage="$BUILD_DIR/arch/x86_64/boot/bzImage"
    if [ ! -f "$bzimage" ]; then
        echo "[ERROR] 编译失败, 未生成 bzImage"
        exit 1
    fi
    echo ""
    echo "[完成] 内核编译成功!"
    echo "  bzImage : $bzimage ($(du -h "$bzimage" | cut -f1))"
    echo "  vmlinux : $BUILD_DIR/vmlinux"
}

do_modules() {
    echo "[附加] 编译并安装模块到 $BUILD_DIR/modules ..."
    make -C "$KERNEL_SRC_DIR" O="$BUILD_DIR" -j"$JOBS" modules 2>&1 | tail -n 5
    make -C "$KERNEL_SRC_DIR" O="$BUILD_DIR" \
        INSTALL_MOD_PATH="$BUILD_DIR/modules" modules_install >/dev/null 2>&1
    echo "      -> 模块安装完成: $BUILD_DIR/modules/lib/modules/"
}

#----------------------------- 主流程 -----------------------------------------
echo "================================================"
echo "  Linux 5.15 LTS 内核编译脚本"
echo "  源码: ${KERNEL_SRC_DIR}"
echo "  输出: ${BUILD_DIR}"
echo "================================================"

case "${1:-build}" in
    clean)
        do_clean
        ;;
    menuconfig)
        do_config
        echo "[配置] 打开菜单配置 ..."
        make -C "$KERNEL_SRC_DIR" O="$BUILD_DIR" menuconfig
        do_build
        ;;
    modules)
        do_config
        do_build
        do_modules
        ;;
    build|"")
        do_config
        do_build
        ;;
    *)
        echo "用法: $0 {build|menuconfig|modules|clean}"
        exit 1
        ;;
esac

echo ""
echo "下一步: 运行 ./run_qemu.sh 启动 QEMU 测试"
