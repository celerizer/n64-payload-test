#include "usb_everdrivex7.h"

#include "usb_internal.h"

/**
 * @todo EverDrive X7 could be supported with a similar method, using
 * REG_USB_DATA.
 * https://n64brew.dev/wiki/EverDrive-64_X7
 */

#define ED64_REG_USB_CFG 0x1F800004
#define ED64_REG_USB_DATA 0x1F800400
#define ED64_SYS_CFG 0x1F808000
#define ED64_REG_KEY 0x1F808004

#define ED64_KEY_UNLOCK 0x0000AA55

cl_error cl64_everdrive_usb_init(void)
{
  /*
  cl64_usb_io_write(ED64_REG_KEY, ED64_KEY_UNLOCK);
  cl64_usb_io_write(ED64_SYS_CFG, 0);
  */
  return CL_ERR_PARAMETER_INVALID;
}

cl_error cl64_everdrive_usb_receive(void *dst, unsigned length)
{
  return CL_ERR_PARAMETER_INVALID;
}

cl_error cl64_everdrive_usb_transfer(const void* data, cl64_datatype type,
  unsigned length)
{
  return CL_ERR_PARAMETER_INVALID;
}
