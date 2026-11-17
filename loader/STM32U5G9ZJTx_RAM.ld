/*
 * External loader linker script — execute from SRAM1 (milestone 4.5).
 *
 * CubeProgrammer loads the .stldr into RAM and calls Init/Write/… through
 * the debug probe. Layout follows ST U5 loaders:
 *
 *   - code/data at 0x20000004
 *     CubeProgrammer writes a tiny stub at 0x20000000 (typically BKPT) and
 *     uses it as the return address (LR) when calling Init/Write/…. The
 *     loader image must start at 0x20000004 so it does not overwrite that
 *     stub. This is an ST tooling convention (H7 uses 0x24000004 the same
 *     way), not an MCU limit — SRAM1 still begins at 0x20000000.
 *
 *   - StorageInfo in a separate ELF segment (SgInfo) for the host tool
 *
 * Only the first ~256 KB of SRAM1 is claimed so CubeProgrammer still has
 * room for its program buffer in the rest of the 768 KB SRAM1.
 *
 * See doc/system_concept.md (memory map, polled time base).
 */
ENTRY(Init)

PHDRS
{
  Loader PT_LOAD;
  SgInfo PT_LOAD;
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

_Min_Heap_Size  = 0x200;
_Min_Stack_Size = 0x400;

MEMORY
{
  /* 0x20000004 .. 0x2003FFFF — same window as ST U5 reference loaders */
  RAM (xrw) : ORIGIN = 0x20000004, LENGTH = 256K - 4
}

SECTIONS
{
  /* Host-readable description block (not part of the Loader segment). */
  .Dev_info 0 :
  {
    KEEP(*(.Dev_info))
  } :SgInfo

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >RAM :Loader

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >RAM :Loader

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >RAM :Loader
  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >RAM :Loader

  .preinit_array :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } >RAM :Loader

  .init_array :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } >RAM :Loader

  .fini_array :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } >RAM :Loader

  /* VMA == LMA: image is downloaded straight into RAM (no flash copy). */
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM :Loader

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM :Loader

  ._user_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE(end = .);
    PROVIDE(_end = .);
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >RAM :Loader

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
