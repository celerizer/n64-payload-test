#include "usb_internal.h"

#include <libdragon.h>

/* libultra defines */
#define	PHYS_TO_K1(x)	((unsigned)(x)|0xA0000000)
#define IO_READ(addr) (*(volatile unsigned*)PHYS_TO_K1(addr))
#define WAIT_ON_IOBUSY(stat) \
    { \
        stat = IO_READ(0x04600010); \
        while (stat & (3)) \
            stat = IO_READ(0x04600010); \
    } (void)0

/**
 * Reads a 32-bit register value
 */
unsigned cl64_usb_io_read(unsigned pi_address)
{
  unsigned value;
  register unsigned stat;

  WAIT_ON_IOBUSY(stat);
  MEMORY_BARRIER();

  value = *(volatile unsigned*)(pi_address | 0xA0000000);

  return value;
}

/**
 * Writes a 32-bit register value
 */
void cl64_usb_io_write(unsigned pi_address, unsigned value)
{
  register unsigned stat;

  WAIT_ON_IOBUSY(stat);
  MEMORY_BARRIER();

  *(volatile unsigned*)(pi_address | 0xA0000000) = value;
}
