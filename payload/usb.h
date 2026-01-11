#ifndef CL64_USB_H
#define CL64_USB_H

#include "classicslive-integration/cl_types.h"

typedef enum
{
  CL64_DATATYPE_INVALID = 0,

  CL64_DATATYPE_MESSAGE_DEBUG = 0xc0,
  CL64_DATATYPE_MESSAGE_INFO = 0xc1,
  CL64_DATATYPE_MESSAGE_WARN = 0xc2,
  CL64_DATATYPE_MESSAGE_ERROR = 0xc3,

  CL64_DATATYPE_NETWORK_POST = 0xc4,
  CL64_DATATYPE_REQUEST_USER_INFO = 0xc5,

  CL64_DATATYPE_SIZE
} cl64_datatype;

/**
 * Initializes the USB communication with the user's flashcart.
 * @return CL_OK on success, or an error code on failure.
 */
cl_error cl64_usb_init(void);

/**
 * A blocking call to receive data from the PC to the N64 over USB.
 * @param dst Pointer to the buffer to receive data into.
 * @param length The length of the data to receive in bytes.
 * @return CL_OK on success, or an error code on failure.
 */
cl_error cl64_usb_receive(void *dst, unsigned length);

/**
 * A blocking call to send data from the N64 to the PC over USB.
 * @param data Pointer to the data to send.
 * @param type The datatype of the data being sent.
 * @param length The length of the data in bytes.
 * @return CL_OK on success, or an error code on failure.
 */
cl_error cl64_usb_transmit(const void* data, cl64_datatype type, unsigned length);

#endif
