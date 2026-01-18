/* linker.ld - STM32F407VGT6 链接器脚本
 * 定义内存布局和段分配
 *
 * STM32F407VGT6 内存配置:
 *   Flash: 1024 KB (0x08000000 - 0x080FFFFF)
 *   RAM:   128 KB  (0x20000000 - 0x2001FFFF)
 *   CCM:   64 KB   (0x10000000 - 0x1000FFFF) - 仅 CPU 可访问
 *
 * 编码: UTF-8
 */

/* 入口点 */
ENTRY(Reset_Handler)

/* 栈大小定义（可根据需要调整） */
_Min_Heap_Size = 0x200;   /* 512 字节 */
_Min_Stack_Size = 0x400;  /* 1024 字节 */

/* 内存区域定义 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 128K
    CCM (rwx)   : ORIGIN = 0x10000000, LENGTH = 64K
}

/* 段定义 */
SECTIONS
{
    /* 中断向量表（必须位于 Flash 起始位置） */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))   /* 保留向量表 */
        . = ALIGN(4);
    } >FLASH

    /* 程序代码段 */
    .text :
    {
        . = ALIGN(4);
        *(.text)               /* 代码 */
        *(.text*)              /* 所有 .text.* 段 */
        *(.glue_7)             /* ARM/Thumb 互操作（ARM 代码） */
        *(.glue_7t)            /* ARM/Thumb 互操作（Thumb 代码） */
        *(.eh_frame)           /* 异常处理帧 */

        KEEP (*(.init))        /* 初始化代码 */
        KEEP (*(.fini))        /* 终止代码 */

        . = ALIGN(4);
        _etext = .;            /* 代码段结束地址 */
    } >FLASH

    /* 只读数据段 */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)             /* 只读数据 */
        *(.rodata*)            /* 所有 .rodata.* 段 */
        . = ALIGN(4);
    } >FLASH

    /* ARM 异常索引（用于展开堆栈） */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH

    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    /* 初始化数组（构造函数） */
    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH

    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

    /* 数据段初始化（Flash 中） */
    _sidata = LOADADDR(.data);

    /* 已初始化数据段（RAM 中，从 Flash 复制） */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;            /* 数据段起始地址 */
        *(.data)               /* 数据 */
        *(.data*)              /* 所有 .data.* 段 */

        . = ALIGN(4);
        _edata = .;            /* 数据段结束地址 */
    } >RAM AT> FLASH

    /* 未初始化数据段（BSS） */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;             /* BSS 起始地址 */
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;             /* BSS 结束地址 */
        __bss_end__ = _ebss;
    } >RAM

    /* 用户堆栈段（用于动态内存分配） */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >RAM

    /* 栈顶（RAM 最高地址） */
    _estack = ORIGIN(RAM) + LENGTH(RAM);

    /* 移除调试信息（可选，减小文件大小） */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    /* 调试符号（不占用 Flash 空间） */
    .ARM.attributes 0 : { *(.ARM.attributes) }
}
