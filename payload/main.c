#include "classicslive-integration/cl_abi.h"
#include "classicslive-integration/cl_main.h"
#include "N64-UNFLoader/usb.h"

#include <string.h>

#define CLN_STACK ((volatile signed char*)0x80600000)

#define SM64_COINS ((volatile unsigned short*)0x8033B218)
#define SM64_LIVES ((volatile signed char*)0x8033B21D)

typedef enum {
  CLN_STATE_UNINITIALIZED = 0,
  CLN_STATE_INITIALIZED   = 1,
  CLN_STATE_ERROR         = 2,
} cln_state_t;

static volatile cln_state_t cln_state = CLN_STATE_UNINITIALIZED;
static volatile unsigned int cl_stack_backup = 0;

static cl_error cln_abi_display_message(unsigned level, const char *msg)
{
  /* Ignore messages for now */
  (void)level;
  (void)msg;

  return 0;
}

static cl_memory_region_t cln_region;
static cl_error result = 123;

static cl_error cln_abi_install_memory_regions(cl_memory_region_t **regions,
  unsigned *region_count)
{
  /** 
   * As of now, we take full reign of the Expansion Pak to run CL,
   * meaning games that require it will not be compatible.
   * We report only the base console memory here.
   */
  *regions = &cln_region;
  regions[0]->base_alloc = (void*)0x80000000;
  regions[0]->base_guest = 0x80000000;
  regions[0]->base_host = (void*)0x80000000;
  regions[0]->size = CL_MB(4);
  regions[0]->pointer_length = 4;
  regions[0]->endianness = CL_ENDIAN_BIG;
  *region_count = 1;

  return 0;
}

static cl_error cln_abi_library_name(const char **name)
{
  *name = "Classics Live 64";
  return CL_OK;
}

static cl_error cln_abi_network_post(const char *url, char *post_data,
  cl_network_cb_t callback, void *userdata)
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
      cln_abi_library_name,
      cln_abi_network_post,
      NULL,
    },
    { NULL, NULL, NULL, NULL }
  },
};

void cln_run(void)
{
  if (cln_state == CLN_STATE_UNINITIALIZED)
  {
    result = cl_abi_register(&cln_abi);
    cln_state = CLN_STATE_INITIALIZED;
  }
  else if (cln_state == CLN_STATE_INITIALIZED)
    *SM64_COINS = result;
}

__attribute__((used, noinline, optimize("O0")))
void _start(void)
{
  asm volatile (
    /* --------------------------------------------------------- */
    /* No compiler prologue / epilogue allowed                   */
    /* We own EVERYTHING from here on                            */
    /* --------------------------------------------------------- */
    ".set noreorder\n"

    /* --------------------------------------------------------- */
    /* Save original stack pointer                               */
    /* --------------------------------------------------------- */
    "sw     $sp, cl_stack_backup\n"

    /* --------------------------------------------------------- */
    /* Switch to private stack                                   */
    /* --------------------------------------------------------- */
    "lui   $sp, 0x8060\n"

    /* --------------------------------------------------------- */
    /* Save all GPRs                                             */
    /* --------------------------------------------------------- */
    ".set noat\n"
    "addiu  $sp, $sp, -128\n"
    "sw     $at,   0($sp)\n"
    "sw     $v0,   4($sp)\n"
    "sw     $v1,   8($sp)\n"
    "sw     $a0,  12($sp)\n"
    "sw     $a1,  16($sp)\n"
    "sw     $a2,  20($sp)\n"
    "sw     $a3,  24($sp)\n"
    "sw     $t0,  28($sp)\n"
    "sw     $t1,  32($sp)\n"
    "sw     $t2,  36($sp)\n"
    "sw     $t3,  40($sp)\n"
    "sw     $t4,  44($sp)\n"
    "sw     $t5,  48($sp)\n"
    "sw     $t6,  52($sp)\n"
    "sw     $t7,  56($sp)\n"
    "sw     $s0,  60($sp)\n"
    "sw     $s1,  64($sp)\n"
    "sw     $s2,  68($sp)\n"
    "sw     $s3,  72($sp)\n"
    "sw     $s4,  76($sp)\n"
    "sw     $s5,  80($sp)\n"
    "sw     $s6,  84($sp)\n"
    "sw     $s7,  88($sp)\n"
    "sw     $t8,  92($sp)\n"
    "sw     $t9,  96($sp)\n"
    "sw     $gp, 100($sp)\n"
    "sw     $fp, 104($sp)\n"
    "sw     $ra, 108($sp)\n"

    /* --------------------------------------------------------- */
    /* Call into C                                               */
    /* --------------------------------------------------------- */
    "jal    cln_run\n"
    "nop\n"

    /* --------------------------------------------------------- */
    /* Restore all GPRs                                          */
    /* --------------------------------------------------------- */
    "lw     $at,   0($sp)\n"
    "lw     $v0,   4($sp)\n"
    "lw     $v1,   8($sp)\n"
    "lw     $a0,  12($sp)\n"
    "lw     $a1,  16($sp)\n"
    "lw     $a2,  20($sp)\n"
    "lw     $a3,  24($sp)\n"
    "lw     $t0,  28($sp)\n"
    "lw     $t1,  32($sp)\n"
    "lw     $t2,  36($sp)\n"
    "lw     $t3,  40($sp)\n"
    "lw     $t4,  44($sp)\n"
    "lw     $t5,  48($sp)\n"
    "lw     $t6,  52($sp)\n"
    "lw     $t7,  56($sp)\n"
    "lw     $s0,  60($sp)\n"
    "lw     $s1,  64($sp)\n"
    "lw     $s2,  68($sp)\n"
    "lw     $s3,  72($sp)\n"
    "lw     $s4,  76($sp)\n"
    "lw     $s5,  80($sp)\n"
    "lw     $s6,  84($sp)\n"
    "lw     $s7,  88($sp)\n"
    "lw     $t8,  92($sp)\n"
    "lw     $t9,  96($sp)\n"
    "lw     $gp, 100($sp)\n"
    "lw     $fp, 104($sp)\n"
    "lw     $ra, 108($sp)\n"
    "addiu  $sp, $sp, 128\n"

    /* --------------------------------------------------------- */
    /* Restore original stack pointer                            */
    /* --------------------------------------------------------- */
    "lw     $sp, cl_stack_backup\n"

    /* --------------------------------------------------------- */
    /* Return to game                                            */
    /* --------------------------------------------------------- */
    ".set at\n"
    ".set reorder\n"
  );
}
