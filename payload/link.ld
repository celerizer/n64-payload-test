OUTPUT_ARCH(mips)
ENTRY(_start)

/* Base address in N64 RAM (Expansion Pak region) */
BASE_ADDR = 0x80400000;

SECTIONS
{
  . = BASE_ADDR;

  /* Code */
  .text ALIGN(16) :
  {
    _text_start = .;
    KEEP(*(.start))
    *(.text .text.*)
    *(.init)
    *(.fini)
    _text_end = .;
  }

  /* Read-only data */
  .rodata ALIGN(16) :
  {
    _rodata_start = .;
    *(.rodata .rodata.*)
    _rodata_end = .;
  }

  /* Initialized data */
  .data ALIGN(16) :
  {
    _data_start = .;
    *(.data .data.*)
    _data_end = .;
  }

  /* Small data (optional, for -G usage) */
  .sdata ALIGN(16) :
  {
    _sdata_start = .;
    *(.sdata .sdata.*)
    _sdata_end = .;
  }

  /* Uninitialized data */
  .bss ALIGN(16) (NOLOAD) :
  {
    _bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    _bss_end = .;
  }

  /DISCARD/ :
  {
    *(.sdata*)
    *(.sbss*)
  }

  /* End of library */
  _end = .;
}
