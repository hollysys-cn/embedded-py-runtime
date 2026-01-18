#!/usr/bin/env bash
#
# GDB 连接脚本
# 功能: 连接到 OpenOCD 调试服务器并启动 GDB 调试会话
#
# 使用方式:
#   ./connect-gdb.sh [选项] <elf文件>
#
# 选项:
#   --host <host>        OpenOCD 主机地址，默认: localhost
#   --port <port>        OpenOCD GDB 端口，默认: 3333
#   --load               自动加载程序到目标设备
#   --tui                启用 GDB TUI 模式（文本用户界面）
#   --batch              批处理模式（执行命令后退出）
#   --cmd <command>      执行自定义 GDB 命令
#   --help               显示帮助信息
#
# 编码: UTF-8
# 换行符: LF

set -euo pipefail

# 获取脚本目录和项目根目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# 引入公共函数
# shellcheck source=../common.sh
source "$PROJECT_ROOT/scripts/common.sh"

#######################################
# 配置变量
#######################################
GDB_BIN="arm-none-eabi-gdb"
HOST="localhost"
PORT=3333
LOAD_PROGRAM=0
TUI_MODE=0
BATCH_MODE=0
CUSTOM_COMMANDS=()
ELF_FILE=""

# GDB 初始化文件
GDB_INIT_FILE="${PROJECT_ROOT}/build/.gdbinit"

#######################################
# 显示帮助信息
#######################################
show_help() {
    cat << EOF
GDB 连接脚本 - 连接到 OpenOCD 调试服务器

使用方式:
    $0 [选项] <elf文件>

选项:
    --host <host>        OpenOCD 主机地址，默认: localhost
    --port <port>        OpenOCD GDB 端口，默认: 3333
    --load               自动加载程序到目标设备（执行 load 命令）
    --tui                启用 GDB TUI 模式（文本用户界面）
    --batch              批处理模式（执行命令后退出）
    --cmd <command>      执行自定义 GDB 命令
    --help               显示帮助信息

参数:
    <elf文件>            要调试的 ELF 可执行文件路径

示例:
    $0 build/firmware.elf                        # 连接并加载符号
    $0 --load build/firmware.elf                 # 连接并烧写程序
    $0 --tui --load build/firmware.elf           # TUI 模式，烧写程序
    $0 --batch --cmd "info registers" firmware.elf  # 批处理模式，查看寄存器

常用 GDB 命令:
    target extended-remote <host>:<port>  # 连接到远程目标
    load                                  # 加载程序到目标设备
    monitor reset halt                    # 复位并暂停目标
    monitor reset init                    # 复位并初始化目标
    continue                              # 继续执行
    break main                            # 在 main 设置断点
    info breakpoints                      # 查看断点
    info registers                        # 查看寄存器
    backtrace                             # 查看调用栈

环境变量:
    GDB_BIN             GDB 可执行文件路径（默认: arm-none-eabi-gdb）

EOF
}

#######################################
# 检查 GDB 是否安装
#######################################
check_gdb() {
    if [[ -n "${GDB_BIN:-}" ]]; then
        GDB_BIN="${GDB_BIN:-arm-none-eabi-gdb}"
    fi

    if ! command_exists "$GDB_BIN"; then
        log_error "未找到 ARM GDB: $GDB_BIN"
        log_error "请确保已安装 ARM 工具链"
        log_error "或设置 GDB_BIN 环境变量指向正确的 GDB 路径"
        return 1
    fi

    local version
    version=$("$GDB_BIN" --version 2>&1 | head -n1 || true)
    log_info "GDB 版本: $version"
}

#######################################
# 检查 ELF 文件
#######################################
check_elf_file() {
    if [[ -z "$ELF_FILE" ]]; then
        log_error "未指定 ELF 文件"
        log_error "使用方式: $0 [选项] <elf文件>"
        return 1
    fi

    if [[ ! -f "$ELF_FILE" ]]; then
        log_error "ELF 文件不存在: $ELF_FILE"
        return 1
    fi

    # 验证是否为 ELF 文件
    if ! file "$ELF_FILE" | grep -q "ELF"; then
        log_warn "文件可能不是有效的 ELF 格式: $ELF_FILE"
    fi

    log_info "ELF 文件: $ELF_FILE"
}

#######################################
# 生成 GDB 初始化文件
#######################################
generate_gdb_init() {
    log_info "生成 GDB 初始化文件: $GDB_INIT_FILE"

    mkdir -p "$(dirname "$GDB_INIT_FILE")"

    cat > "$GDB_INIT_FILE" << EOF
# GDB 初始化文件（自动生成）
# 连接到 OpenOCD 调试服务器

# 连接到远程目标
target extended-remote $HOST:$PORT

# 美化输出
set print pretty on
set pagination off

# 复位并暂停目标
monitor reset halt

EOF

    # 如果需要加载程序
    if [[ $LOAD_PROGRAM -eq 1 ]]; then
        cat >> "$GDB_INIT_FILE" << EOF
# 加载程序到目标设备
load

# 复位并初始化
monitor reset init

EOF
    fi

    # 添加自定义命令
    for cmd in "${CUSTOM_COMMANDS[@]}"; do
        echo "$cmd" >> "$GDB_INIT_FILE"
    done

    # 显示初始信息
    cat >> "$GDB_INIT_FILE" << EOF

# 显示初始信息
echo \\n
echo ========================================\\n
echo GDB 已连接到 $HOST:$PORT\\n
echo ELF 文件: $ELF_FILE\\n
echo ========================================\\n
echo \\n
echo 常用命令:\\n
echo   continue (c)     - 继续执行\\n
echo   break <func>     - 设置断点\\n
echo   next (n)         - 单步执行（跳过函数）\\n
echo   step (s)         - 单步执行（进入函数）\\n
echo   info registers   - 查看寄存器\\n
echo   backtrace (bt)   - 查看调用栈\\n
echo   monitor reset    - 复位目标\\n
echo   quit (q)         - 退出 GDB\\n
echo \\n

EOF
}

#######################################
# 启动 GDB
#######################################
start_gdb() {
    log_info "启动 GDB..."
    log_info "  目标: $HOST:$PORT"
    log_info "  程序: $ELF_FILE"
    [[ $LOAD_PROGRAM -eq 1 ]] && log_info "  操作: 加载程序到设备"
    [[ $TUI_MODE -eq 1 ]] && log_info "  模式: TUI"
    [[ $BATCH_MODE -eq 1 ]] && log_info "  模式: 批处理"

    # 生成初始化文件
    generate_gdb_init

    # 构建 GDB 命令
    local gdb_args=(
        -q  # 安静模式
        -x "$GDB_INIT_FILE"  # 执行初始化文件
        "$ELF_FILE"  # ELF 文件
    )

    # TUI 模式
    if [[ $TUI_MODE -eq 1 ]]; then
        gdb_args+=(-tui)
    fi

    # 批处理模式
    if [[ $BATCH_MODE -eq 1 ]]; then
        gdb_args+=(--batch)
    fi

    # 启动 GDB
    log_info ""
    "$GDB_BIN" "${gdb_args[@]}"
}

#######################################
# 主函数
#######################################
main() {
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            --host)
                HOST="$2"
                shift 2
                ;;
            --port)
                PORT="$2"
                shift 2
                ;;
            --load)
                LOAD_PROGRAM=1
                shift
                ;;
            --tui)
                TUI_MODE=1
                shift
                ;;
            --batch)
                BATCH_MODE=1
                shift
                ;;
            --cmd)
                CUSTOM_COMMANDS+=("$2")
                shift 2
                ;;
            --help)
                show_help
                exit 0
                ;;
            -*)
                log_error "未知选项: $1"
                echo ""
                show_help
                exit 1
                ;;
            *)
                ELF_FILE="$1"
                shift
                ;;
        esac
    done

    # 检查 GDB
    check_gdb || exit 1

    # 检查 ELF 文件
    check_elf_file || exit 1

    # 启动 GDB
    start_gdb
}

# 运行主函数
main "$@"
