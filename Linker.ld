MEMORY
{
    flash (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
    ram   (rwx) : ORIGIN = 0x20000000, LENGTH = 36K
}

ENTRY(Reset_Handler)

_estack = ORIGIN(ram) + LENGTH(ram);

_Min_Heap_Size  = 0x200;  /* 512 bytes minimum heap  */
_Min_Stack_Size = 0x400;  /* 1024 bytes minimum stack */

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)

        /* Static constructor support (C++ / libc init) */
        . = ALIGN(4);
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);

        . = ALIGN(4);
        _etext = .;
    } > flash

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > ram AT > flash

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > ram

    /* Heap and stack layout with MPU guard between them */
    ._user_heap_stack (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        PROVIDE(__end__ = .);

        /* Heap grows UPWARD from here */
        . = . + _Min_Heap_Size;

        /* MPU guard — sits between heap and stack */
        . = ALIGN(32);
        __mpu_guard_start = .;
        . = . + 32;
        __mpu_guard_end = .;

        /* Mark where paintable stack begins */
        __stack_bottom = .;

        /* Stack grows DOWNWARD from _estack toward here */
        . = . + _Min_Stack_Size;

        . = ALIGN(8);
    } > ram

    /* ══════════════════════════════════════════════════
     * ASSERT: Catch heap/stack collision at LINK TIME
     * +40 accounts for worst-case alignment padding
     * between sections (ALIGN(8), ALIGN(32), ALIGN(8))
     * ══════════════════════════════════════════════════ */
    ASSERT(
        (ORIGIN(ram) + LENGTH(ram)) >=
            (_ebss + _Min_Heap_Size + 32 + _Min_Stack_Size + 40),
        "ERROR: Heap + Stack + Guard + BSS exceed available RAM (36KB)!"
    )

    /* Discard unused ARM exception handling sections */
    /DISCARD/ :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    }
}