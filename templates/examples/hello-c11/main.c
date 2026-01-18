// hello-c11 - C11 语言特性演示程序
// 用于验证 ARM Cortex-M4 C11 编译环境
//
// 此程序演示以下 C11 特性:
// - _Static_assert: 编译时断言
// - _Generic: 泛型宏
// - _Alignas: 内存对齐
// - 匿名结构体/联合体
// - 复合字面量
//
// 编码: UTF-8

#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>

//===========================================
// 1. _Static_assert - 编译时断言
//===========================================

// 验证基本类型大小
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");
_Static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes");

// 验证结构体对齐
struct sensor_data {
    uint32_t timestamp;
    float value;
};
_Static_assert(sizeof(struct sensor_data) == 8, "unexpected padding");

//===========================================
// 2. _Generic - 泛型宏
//===========================================

// 泛型的绝对值函数
#define abs_value(x) _Generic((x), \
    int: abs_int,                  \
    float: abs_float,              \
    double: abs_double             \
)(x)

static inline int abs_int(int x) {
    return x < 0 ? -x : x;
}

static inline float abs_float(float x) {
    return x < 0.0f ? -x : x;
}

static inline double abs_double(double x) {
    return x < 0.0 ? -x : x;
}

//===========================================
// 3. _Alignas - 内存对齐
//===========================================

// 确保结构体 16 字节对齐（DMA 传输优化）
typedef struct {
    uint32_t data[4];
} _Alignas(16) dma_buffer_t;

_Static_assert(alignof(dma_buffer_t) == 16, "DMA buffer must be 16-byte aligned");

//===========================================
// 4. 匿名结构体/联合体
//===========================================

// 寄存器位字段定义（常用于嵌入式开发）
typedef union {
    uint32_t value;
    struct {
        uint32_t enable    : 1;  // bit 0
        uint32_t ready     : 1;  // bit 1
        uint32_t error     : 1;  // bit 2
        uint32_t reserved  : 29; // bits 3-31
    };
} control_register_t;

//===========================================
// 5. 复合字面量
//===========================================

// 初始化传感器数据（无需中间变量）
static inline struct sensor_data create_sensor_data(uint32_t ts, float val) {
    return (struct sensor_data){.timestamp = ts, .value = val};
}

//===========================================
// 全局变量
//===========================================

// DMA 缓冲区（16 字节对齐）
static dma_buffer_t g_dma_buffer = {
    .data = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x87654321}
};

// 控制寄存器
static volatile control_register_t g_control = {.value = 0};

//===========================================
// 测试函数
//===========================================

// 测试 _Generic 泛型宏
static void test_generic_abs(void) {
    int i = -42;
    float f = -3.14f;
    double d = -2.718;

    // 使用泛型宏，根据类型自动选择函数
    volatile int abs_i = abs_value(i);     // 调用 abs_int
    volatile float abs_f = abs_value(f);   // 调用 abs_float
    volatile double abs_d = abs_value(d);  // 调用 abs_double

    (void)abs_i;
    (void)abs_f;
    (void)abs_d;
}

// 测试位字段和匿名结构体
static void test_bitfield(void) {
    // 设置控制寄存器
    g_control.enable = 1;
    g_control.ready = 0;
    g_control.error = 0;

    // 也可以直接操作整个寄存器值
    g_control.value |= 0x2;  // 设置 ready 位
}

// 测试复合字面量
static void test_compound_literal(void) {
    // 直接创建结构体
    struct sensor_data sensor = create_sensor_data(1000, 25.5f);

    // 使用复合字面量作为函数参数（假设有这样的函数）
    // process_sensor_data((struct sensor_data){.timestamp = 2000, .value = 26.0f});

    (void)sensor;
}

// 测试对齐
static void test_alignment(void) {
    // 检查对齐（编译时已通过 _Static_assert 验证）
    uintptr_t addr = (uintptr_t)&g_dma_buffer;

    // 运行时验证（可选）
    if ((addr & 0xF) != 0) {
        // 对齐错误（不应发生）
        while(1);
    }
}

//===========================================
// 主函数
//===========================================

int main(void) {
    // 初始化：演示 C11 特性
    test_generic_abs();
    test_bitfield();
    test_compound_literal();
    test_alignment();

    // 嵌入式主循环
    while (1) {
        // 在实际应用中，这里会有任务处理
        // 例如：读取传感器、处理数据、控制执行器等

        // 演示：访问 DMA 缓冲区
        volatile uint32_t data = g_dma_buffer.data[0];
        (void)data;

        // 演示：检查控制寄存器
        if (g_control.enable && !g_control.error) {
            // 执行某些操作
        }
    }

    return 0;  // 嵌入式系统通常不会返回
}
