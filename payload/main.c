#include "classicslive-integration/cl_abi.h"
#include "classicslive-integration/cl_main.h"
#include "N64-UNFLoader/usb.h"

#include <string.h>

#define SM64_COINS ((volatile unsigned short*)0x8033B218)
#define SM64_LIVES ((volatile signed char*)0x8033B21D)

typedef enum {
  CLN_STATE_UNINITIALIZED = 0,
  CLN_STATE_INITIALIZED   = 1,
  CLN_STATE_ERROR         = 2,
} cln_state_t;

static volatile cln_state_t cln_state = CLN_STATE_UNINITIALIZED;

static cl_error cln_abi_display_message(unsigned level, const char *msg)
{
  /* Ignore messages for now */
  (void)level;
  (void)msg;

  return 0;
}

cl_error cln_abi_install_memory_regions(cl_memory_region_t **regions,
  unsigned *region_count)
{
  *regions = malloc(sizeof(cl_memory_region_t));
  regions[0]->base_alloc = (void*)0x80000000;
  regions[0]->base_guest = 0x80000000;
  regions[0]->base_host = (void*)0x80000000;
  regions[0]->size = 0x00400000;
  regions[0]->pointer_length = 4;
  regions[0]->endianness = CL_ENDIAN_BIG;
  *region_count = 1;

  return 0;
}

static int cln_abi_network_post(const char *url,
                               const char *post_data,
                               char **response_data)
{
  if (!usb_write(DATATYPE_TEXT, url, strlen(url) + 1) ||
      !usb_write(DATATYPE_TEXT, post_data, strlen(post_data) + 1))
  {
    return -1;
  }
  else
    return 0;
}

static cl_abi_t cln_abi =
{
  CL_ABI_VERSION,
  {
    {
      cln_abi_display_message,
      cln_abi_install_memory_regions,
    },
    { NULL, NULL, NULL, NULL }
  },
};

__attribute__((used, noinline, optimize("O0")))
void _start(void)
{
  if (cln_state == CLN_STATE_ERROR)
    return;
  else
  {
    *SM64_COINS = cl_abi_register(&cln_abi);
  }
}
