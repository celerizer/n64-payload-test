#ifndef CL64_USB_EVERDRIVE_H
#define CL64_USB_EVERDRIVE_H

#include "usb.h"

cl_error cl64_everdrive_usb_init(void);

cl_error cl64_everdrive_usb_receive(void *dst, unsigned length);

cl_error cl64_everdrive_usb_transfer(const void* data, cl64_datatype type,
  unsigned length);

#endif
