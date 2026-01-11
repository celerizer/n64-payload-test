#include "usb_summercart.h"
#include "usb_internal.h"

/* SummerCart 64 defines */
#define SC64_BRAM 0x1FFE0000
#define SC64_BRAM_SIZE CL_KB(8)

#define SC64_REG_SCR 0x1FFF0000
#define SC64_REG_DATA_0 0x1FFF0004
#define SC64_REG_DATA_1 0x1FFF0008
#define SC64_REG_IDENTIFIER 0x1FFF000C
#define SC64_REG_KEY 0x1FFF0010

#define SC64_CMD_USB_WRITE 'M'
#define SC64_SCR_CMD_BUSY (1 << 31)

#define SC64_V2_IDENTIFIER 0x53437632

#define SC64_KEY_RESET 0x00000000
#define SC64_KEY_UNLOCK_1 0x5F554E4C
#define SC64_KEY_UNLOCK_2 0x4F434B5F

/**
 * Reads data from the SummerCart 64 data buffer.
 */
static cl_error cl64_bram_read(void* data, unsigned length)
{
  if (length > SC64_BRAM_SIZE)
    return CL_ERR_PARAMETER_INVALID;
  else
  {
    unsigned packet_length;
    unsigned i;

    if (cl64_usb_io_read(SC64_BRAM) != CL64_MAGIC_PC_TO_N64)
      return CL_ERR_CLIENT_RUNTIME;
    packet_length = cl64_usb_io_read(SC64_BRAM + 4);
    if (length > packet_length)
      length = packet_length;

    for (i = 0; i < length; i += 4)
      *((unsigned*)((char*)data + i)) = cl64_usb_io_read(SC64_BRAM + i + 8);
  }

  return CL_OK;
}

/**
 * Writes data to the SummerCart 64 data buffer.
 */
static cl_error cl64_bram_write(const void* data, unsigned length)
{
  if (length > SC64_BRAM_SIZE)
    return CL_ERR_PARAMETER_INVALID;
  else
  {
    unsigned i;

    cl64_usb_io_write(SC64_BRAM, CL64_MAGIC_N64_TO_PC);
    cl64_usb_io_write(SC64_BRAM + 4, length);
    for (i = 0; i < length; i += 4)
      cl64_usb_io_write(SC64_BRAM + i + 8, *((unsigned*)((const char*)data + i)));
  }

  return CL_OK;
}

/**
 * Checks if the SummerCart 64 data buffer is empty.
 */
static int cl64_bram_is_empty(void)
{
  volatile unsigned scr;

  scr = cl64_usb_io_read(SC64_BRAM);

  return scr != CL64_MAGIC_PC_TO_N64;
}

/**
 * Waits for the SummerCart 64 to be ready for the next command.
 */
static inline void cl64_summercart_wait(void)
{
  volatile unsigned scr;

  do scr = cl64_usb_io_read(SC64_REG_SCR);
  while (scr & SC64_SCR_CMD_BUSY);
}

cl_error cl64_summercart_init(void)
{
  /* Try to unlock SummerCart 64 registers and confirm device */
  cl64_usb_io_write(SC64_REG_KEY, SC64_KEY_RESET);
  cl64_usb_io_write(SC64_REG_KEY, SC64_KEY_UNLOCK_1);
  cl64_usb_io_write(SC64_REG_KEY, SC64_KEY_UNLOCK_2);
  if (cl64_usb_io_read(SC64_REG_IDENTIFIER) != SC64_V2_IDENTIFIER)
    return CL_ERR_PARAMETER_INVALID;

  return CL_OK;
}

cl_error cl64_summercart_receive(void *dst, unsigned length)
{
  /* Block until data is available */
  while (cl64_bram_is_empty());

  /* Wait for the USB transfer to complete */
  cl64_summercart_wait();

  /* Read data from the data buffer */
  return cl64_bram_read(dst, length);
}

cl_error cl64_summercart_transmit(const void* data, cl64_datatype type,
  unsigned length)
{
  cl_error err;

  cl64_summercart_wait();

  err = cl64_bram_write(data, length);
  if (err != CL_OK)
    return err;

  /* Start transmission */
  cl64_usb_io_write(SC64_REG_DATA_0, SC64_BRAM);
  cl64_usb_io_write(SC64_REG_DATA_1, (type << 24) | (length + 8));
  cl64_usb_io_write(SC64_REG_SCR, SC64_CMD_USB_WRITE);

  return CL_OK;
}
