# CMakeLists.txt for hello-c11 Example
# C11 语言特性演示项目
#
# 使用方式:
#   1. 复制此目录到工作区
#   2. 构建:
#      cmake -B build -DCMAKE_TOOLCHAIN_FILE=../../templates/cmake/toolchain-arm-cortex-m4.cmake
#      cmake --build build
#
# 编码: UTF-8
# 换行符: LF

cmake_minimum_required(VERSION 3.20)

# 项目信息
project(hello-c11
    VERSION 1.0.0
    DESCRIPTION "C11 Language Features Demo"
    LANGUAGES C
)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# 源文件
set(SOURCE_FILES
    main.c
)

# 创建可执行文件
add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

# C11 标准（应该从工具链文件继承）
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# 编译定义
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)

# 警告选项（更严格）
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    -Werror  # 将警告视为错误
)

# 简单的链接选项（无需链接脚本的示例）
# 注意：实际硬件需要正确的链接脚本
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
)

# 生成 HEX 文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMENT "生成 HEX 文件"
)

# 生成 BIN 文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "生成 BIN 文件"
)

# 显示代码大小
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} --format=berkeley
        $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "代码大小统计:"
)

# 打印配置
message(STATUS "===================================")
message(STATUS "hello-c11 项目配置")
message(STATUS "===================================")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "C 编译器: ${CMAKE_C_COMPILER}")
message(STATUS "C 标准: C11")
message(STATUS "===================================")
