/* startup.s - STM32F407 启动文件（ARM Cortex-M4）
 * 功能：初始化栈、复制数据段、清零 BSS、调用 main
 *
 * 编码: UTF-8
 * 换行符: LF
 */

    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

/* 全局符号 */
.global Reset_Handler
.global Default_Handler

/* 外部符号（来自链接脚本） */
.word _estack           /* 栈顶地址 */
.word _sidata           /* 初始化数据段起始地址（Flash） */
.word _sdata            /* 数据段起始地址（RAM） */
.word _edata            /* 数据段结束地址（RAM） */
.word _sbss             /* BSS 段起始地址（RAM） */
.word _ebss             /* BSS 段结束地址（RAM） */

/**
 * 向量表（中断向量表）
 * 位于 Flash 起始位置（0x08000000）
 */
    .section .isr_vector,"a",%progbits
    .type g_pfnVectors, %object
    .size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
    .word _estack                   /* 栈顶指针 */
    .word Reset_Handler             /* Reset 处理函数 */
    .word NMI_Handler               /* NMI */
    .word HardFault_Handler         /* Hard Fault */
    .word MemManage_Handler         /* Memory Management */
    .word BusFault_Handler          /* Bus Fault */
    .word UsageFault_Handler        /* Usage Fault */
    .word 0                         /* 保留 */
    .word 0                         /* 保留 */
    .word 0                         /* 保留 */
    .word 0                         /* 保留 */
    .word SVC_Handler               /* SVCall */
    .word DebugMon_Handler          /* Debug Monitor */
    .word 0                         /* 保留 */
    .word PendSV_Handler            /* PendSV */
    .word SysTick_Handler           /* SysTick */

    /* 外部中断（STM32F407 有 82 个外部中断） */
    /* 这里简化处理，仅列出前 10 个 */
    .word Default_Handler           /* WWDG */
    .word Default_Handler           /* PVD */
    .word Default_Handler           /* TAMP_STAMP */
    .word Default_Handler           /* RTC_WKUP */
    .word Default_Handler           /* FLASH */
    .word Default_Handler           /* RCC */
    .word Default_Handler           /* EXTI0 */
    .word Default_Handler           /* EXTI1 */
    .word Default_Handler           /* EXTI2 */
    .word Default_Handler           /* EXTI3 */
    /* ... 省略其他中断向量 ... */

/**
 * Reset_Handler - 复位处理函数
 * 系统启动时执行的第一段代码
 */
    .section .text.Reset_Handler
    .weak Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    /* 1. 复制数据段从 Flash 到 RAM */
    ldr r0, =_sdata         /* 目标地址（RAM） */
    ldr r1, =_edata         /* 结束地址 */
    ldr r2, =_sidata        /* 源地址（Flash） */
    movs r3, #0
    b LoopCopyDataInit

CopyDataInit:
    ldr r4, [r2, r3]        /* 从 Flash 读取 */
    str r4, [r0, r3]        /* 写入 RAM */
    adds r3, r3, #4

LoopCopyDataInit:
    adds r4, r0, r3
    cmp r4, r1
    bcc CopyDataInit        /* 如果未到结束地址，继续复制 */

    /* 2. 清零 BSS 段 */
    ldr r2, =_sbss          /* BSS 起始地址 */
    ldr r4, =_ebss          /* BSS 结束地址 */
    movs r3, #0
    b LoopFillZerobss

FillZerobss:
    str r3, [r2]            /* 写入 0 */
    adds r2, r2, #4

LoopFillZerobss:
    cmp r2, r4
    bcc FillZerobss         /* 如果未到结束地址，继续清零 */

    /* 3. 调用 main 函数 */
    bl main

    /* 4. 如果 main 返回，进入死循环 */
    b .

    .size Reset_Handler, .-Reset_Handler

/**
 * Default_Handler - 默认中断处理函数
 * 捕获所有未实现的中断
 */
    .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
    b Infinite_Loop
    .size Default_Handler, .-Default_Handler

/**
 * 弱符号定义
 * 如果用户未实现这些处理函数，则使用 Default_Handler
 */
    .weak NMI_Handler
    .thumb_set NMI_Handler,Default_Handler

    .weak HardFault_Handler
    .thumb_set HardFault_Handler,Default_Handler

    .weak MemManage_Handler
    .thumb_set MemManage_Handler,Default_Handler

    .weak BusFault_Handler
    .thumb_set BusFault_Handler,Default_Handler

    .weak UsageFault_Handler
    .thumb_set UsageFault_Handler,Default_Handler

    .weak SVC_Handler
    .thumb_set SVC_Handler,Default_Handler

    .weak DebugMon_Handler
    .thumb_set DebugMon_Handler,Default_Handler

    .weak PendSV_Handler
    .thumb_set PendSV_Handler,Default_Handler

    .weak SysTick_Handler
    .thumb_set SysTick_Handler,Default_Handler
