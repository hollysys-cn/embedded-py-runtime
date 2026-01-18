# PLCopen 基础功能块库 - 根 CMake 配置
# 项目: embedded-py-runtime
# 功能: PLCopen 标准功能块实现

cmake_minimum_required(VERSION 3.20)

# 项目定义
project(embedded-py-runtime
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "PLCopen 基础功能块库，用于 ARM Cortex-M4 嵌入式控制系统"
)

# C11 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 编译选项
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -fno-common
    -ffunction-sections
    -fdata-sections
)

# 数学库链接
link_libraries(m)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/.toolchain/unity/src
)

# PLCopen 功能块库源文件
set(PLCOPEN_SOURCES
    src/plcopen/common.c
    src/plcopen/fb_pid.c
    src/plcopen/fb_pt1.c
    src/plcopen/fb_ramp.c
    src/plcopen/fb_limit.c
    src/plcopen/fb_deadband.c
    src/plcopen/fb_integrator.c
    src/plcopen/fb_derivative.c
)

# Unity 测试框架源文件
set(UNITY_SOURCES
    .toolchain/unity/src/unity.c
)

# PLCopen 静态库
add_library(plcopen STATIC ${PLCOPEN_SOURCES})
target_include_directories(plcopen PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# 启用测试
enable_testing()

# 添加测试子目录
add_subdirectory(tests/plcopen)

# 添加示例子目录
add_subdirectory(examples/plcopen)

# 安装规则
install(TARGETS plcopen
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/plcopen
    DESTINATION include
)
