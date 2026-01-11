#include "usb.h"
#include "usb_internal.h"
#include "usb_summercart.h"
#include "usb_everdrivex7.h"

#include "classicslive-integration/cl_types.h"

typedef enum
{
  CL64_CART_INVALID = 0,

  CL64_CART_SUMMERCART64 = 1,
  CL64_CART_EVERDRIVE64_X7 = 2,
  CL64_CART_EVERDRIVE64_V3 = 3,

  CL64_CART_SIZE
} cl64_cart_type;

static cl64_cart_type cl64_cart = CL64_CART_INVALID;

cl_error cl64_usb_init(void)
{
  if (cl64_summercart_init() == CL_OK)
    cl64_cart = CL64_CART_SUMMERCART64;
#if 0
  else if (cl64_everdrivex7_init() == CL_OK)
    cl64_cart = CL64_CART_EVERDRIVE64_X7; 
#endif
  else
  {
    cl64_cart = CL64_CART_INVALID;
    return CL_ERR_CLIENT_RUNTIME;
  }

  return CL_OK;
}

cl_error cl64_usb_transmit(const void* data, cl64_datatype type, unsigned length)
{
  switch (cl64_cart)
  {
  case CL64_CART_SUMMERCART64:
    return cl64_summercart_transmit(data, type, length);
  default:
    return CL_ERR_CLIENT_RUNTIME;
  }
}

cl_error cl64_usb_receive(void *dst, unsigned length)
{
  switch (cl64_cart)
  {
  case CL64_CART_SUMMERCART64:
    return cl64_summercart_receive(dst, length);
  default:
    return CL_ERR_CLIENT_RUNTIME;
  }
}
