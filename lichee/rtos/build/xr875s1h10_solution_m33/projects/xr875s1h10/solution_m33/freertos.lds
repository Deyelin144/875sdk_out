MEMORY
{
    RAM (rwx) : ORIGIN = 0x0C004000, LENGTH = 0x1BD2000
    HPSRAM (rwx) : ORIGIN = 0x0C004000, LENGTH = 680K
}
__RAM_BASE = ORIGIN(RAM);
__RAM_LENGTH = LENGTH(RAM);
__HPSRAM_BASE = ORIGIN(HPSRAM);
__HPSRAM_LENGTH = LENGTH(HPSRAM);
__MSP_STACK_LENGTH = 0x1000;
__FLASH_LENGTH = 0;
ENTRY(Reset_Handler)
SECTIONS
{
    .head : {
       __head_start = .;
       KEEP(*(.head))
       __head_end = .;
    } > RAM
    .text :
    {
        . = ALIGN(4);
        __text_start__ = .;
        KEEP(*(.text.Reset_Handler))
        KEEP(*(.cpu_text))
        *(*.text)
        *(.text*)
        *(.nonxip_text*)
        *(.sram_text*)
        KEEP(*(.init))
        KEEP(*(.fini))
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)
        *(.rodata*)
        *(.nonxip_rodata*)
        *(.sram_rodata*)
        KEEP(*(.eh_frame*))
        . = ALIGN(4);
        __text_end__ = .;
    } > RAM
    .isr_vector : ALIGN(0x400)
    {
        __VECTOR_BASE = .;
        irq_vector = .;
        KEEP(*(.isr_vector))
    } > RAM
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > RAM
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > RAM
    __exidx_end = .;
    . = ALIGN(4);
    __etext = .;
    _sidata = .;
    .data :
    {
        . = ALIGN(4);
        __data_start__ = .;
        _sdata = .;
        *(vtable)
        *(.data*)
        *(.nonxip_data*)
        *(.sram_data*)
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        PROVIDE(__ctors_start__ = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE(__ctors_end__ = .);
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);
        KEEP(*(.jcr*))
        . = ALIGN(4);
        __data_end__ = .;
        _edata = .;
    } > RAM
   .FSymTab : {
        _syscall_table_begin = .;
 KEEP(*(FSymTab))
 _syscall_table_end = .;
   } > RAM
   .VSymTab : {
        __vsymtab_start = .;
 KEEP(*(VSymTab))
 __vsymtab_end = .;
   } > RAM
   .ttcall : {
        _tt_begin = .;
 KEEP(*(ttcall))
 _tt_end = .;
   } > RAM
    .heap (COPY):
    {
        __end__ = .;
        __heap_start__ = .;
        _heap_start = .;
        end = __end__;
        *(.heap*)
        __HeapLimit = .;
    } > RAM
    .stack_dummy (COPY):
    {
        *(.stack*)
    } > RAM
    .bss (NOLOAD):
    {
        . = ALIGN(4);
        __bss_start__ = .;
        _sbss = .;
        *(.bss*)
        *(COMMON)
        *(.nonxip_bss*)
        *(.sram_bss*)
        . = ALIGN(4);
        __bss_end__ = .;
        _ebss = .;
    } > RAM
    . = ALIGN(8);
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    _estack = __StackTop;
    __heap_end__ = _estack - __MSP_STACK_LENGTH;
    _heap_end = __heap_end__;
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
