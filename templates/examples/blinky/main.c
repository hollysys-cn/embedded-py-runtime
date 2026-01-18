// blinky - LED 闪烁示例程序（STM32F407）
// 演示完整的嵌入式启动流程和硬件控制
//
// 硬件：STM32F407VGT6（ARM Cortex-M4）
// LED：PD12（绿色）- STM32F4 Discovery 板载 LED
//
// 编码: UTF-8

#include <stdint.h>

//===========================================
// 硬件寄存器定义
//===========================================

// RCC (Reset and Clock Control)
#define RCC_BASE            0x40023800
#define RCC_AHB1ENR         (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_AHB1ENR_GPIODEN (1U << 3)

// GPIOD
#define GPIOD_BASE          0x40020C00
#define GPIOD_MODER         (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR           (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

// LED 引脚（PD12）
#define LED_PIN             12
#define LED_PIN_MASK        (1U << LED_PIN)

//===========================================
// 延时函数（简单循环延时）
//===========================================

static void delay_ms(uint32_t ms) {
    // 假设系统时钟 168 MHz（实际需要根据时钟配置调整）
    // 粗略估计：每毫秒约 168000 个周期，每次循环约 3-4 个周期
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile uint32_t j = 0; j < 42000; j++) {
            // 空循环
        }
    }
}

//===========================================
// GPIO 初始化
//===========================================

static void gpio_init(void) {
    // 1. 使能 GPIOD 时钟
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    // 2. 配置 PD12 为输出模式（模式寄存器：每引脚 2 位）
    //    00: 输入, 01: 输出, 10: 复用, 11: 模拟
    GPIOD_MODER &= ~(0x3U << (LED_PIN * 2));  // 清除
    GPIOD_MODER |= (0x1U << (LED_PIN * 2));   // 设置为输出
}

//===========================================
// LED 控制
//===========================================

static inline void led_on(void) {
    GPIOD_ODR |= LED_PIN_MASK;
}

static inline void led_off(void) {
    GPIOD_ODR &= ~LED_PIN_MASK;
}

static inline void led_toggle(void) {
    GPIOD_ODR ^= LED_PIN_MASK;
}

//===========================================
// 主函数
//===========================================

int main(void) {
    // 初始化 GPIO
    gpio_init();

    // 主循环：LED 闪烁
    while (1) {
        led_toggle();
        delay_ms(500);  // 500ms 延时
    }

    return 0;
}

//===========================================
// 默认中断处理（弱符号，可被覆盖）
//===========================================

void __attribute__((weak)) Default_Handler(void) {
    // 捕获未处理的中断
    while (1) {
        // 死循环
    }
}

// 中断处理函数别名（指向 Default_Handler）
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
