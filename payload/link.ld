OUTPUT_ARCH(mips)
ENTRY(_start)

BASE_ADDR = 0x80400000;

SECTIONS
{
  . = BASE_ADDR;

  .text : {
    *(.text._start)
    *(.text*)
  }

  .rodata : {
    *(.rodata*)
    *(.lit4*)
    *(.lit8*)
  }

  .data : {
    *(.data*)
  }

  /* Small data — GP-relative */
  .sdata : {
    __sdata_start = .;
    *(.sdata*)
    __sdata_end = .;
  }

  .sbss (NOLOAD) : {
    __sbss_start = .;
    *(.sbss*)
    __sbss_end = .;
  }

  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  }

  /* Place GP near the middle of small data */
  _gp = (__sdata_start + __sbss_end) / 2;
}
