OUTPUT_ARCH("riscv")
OUTPUT_FORMAT("elf64-littleriscv","elf64-littleriscv","elf64-littleriscv")
MEMORY
{
    PSRAM (rwx) : ORIGIN = 0x0C6AE000, LENGTH = 0x1952000
}
__PSRAM_BASE = ORIGIN(PSRAM);
__PSRAM_LENGTH = LENGTH(PSRAM);
__MSP_STACK_LENGTH = 0x100;
__RAM_LENGTH = LENGTH(PSRAM);
ENTRY(_start)
SECTIONS
{
    .psram_text :
    {
        . = ALIGN(4);
        __text_start__ = .;
        KEEP(*(.start))
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
        . = ALIGN(4);
        __text_end__ = .;
    } > PSRAM
    . = ALIGN(4);
    __etext = .;
    _sidata = .;
    .psram_data :
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
        . = ALIGN(16);
        __init_process_stack_start__ = .;
        . += 16384;
        __init_process_stack_end__ = .;
        . = ALIGN(8);
        __freertos_irq_stack_bottom = .;
        . += 16384;
        __freertos_irq_stack_top = .;
        PROVIDE( __global_pointer$ = . + 0x400 );
        . = ALIGN(4);
        __data_end__ = .;
        _edata = .;
    } > PSRAM
   .FSymTab : {
        _syscall_table_begin = .;
 KEEP(*(FSymTab))
 _syscall_table_end = .;
   } > PSRAM
   .VSymTab : {
        __vsymtab_start = .;
 KEEP(*(VSymTab))
 __vsymtab_end = .;
   } > PSRAM
   .ttcall : {
        _tt_begin = .;
 KEEP(*(ttcall))
 _tt_end = .;
   } > PSRAM
    .heap (COPY):
    {
        __end__ = .;
        __heap_start__ = .;
        _heap_start = .;
        end = __end__;
        *(.heap*)
        __HeapLimit = .;
    } > PSRAM
    .stack_dummy (COPY):
    {
        *(.stack*)
    } > PSRAM
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
    } > PSRAM
    . = ALIGN(64);
    __StackTop = ORIGIN(PSRAM) + LENGTH(PSRAM);
    _estack = __StackTop;
    __heap_end__ = _estack - __MSP_STACK_LENGTH;
    _heap_end = __heap_end__;
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);
    __FLASH_BASE = ORIGIN(FLASH);
    PROVIDE(__XIP_Base = __FLASH_BASE);
    PROVIDE(__XIP_End = __XIP_Base);
    ASSERT(__StackLimit >= __HeapLimit, "region PSRAM overflowed with stack")
}
