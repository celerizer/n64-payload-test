#ifndef CL64_USB_SUMMERCART_H
#define CL64_USB_SUMMERCART_H

#include "usb.h"

cl_error cl64_summercart_init(void);

cl_error cl64_summercart_receive(void *dst, unsigned length);

cl_error cl64_summercart_transmit(const void* data, cl64_datatype type,
  unsigned length);

#endif
