#include "usb.h"

#include "classicslive-integration/cl_abi.h"
#include "classicslive-integration/cl_json.h"
#include "classicslive-integration/cl_main.h"
#include "classicslive-integration/cl_memory.h"
#include "classicslive-integration/cl_network.h"

#include <stdio.h>
#include <string.h>

#define CLN_STACK ((volatile signed char*)0x80600000)

/** @todo remove */
#define SM64_COINS ((volatile unsigned short*)0x8033B218)
#define SM64_LIVES ((volatile signed char*)0x8033B21D)

typedef enum {
  CLN_STATE_UNINITIALIZED = 0,
  CLN_STATE_INITIALIZED   = 1,
  CLN_STATE_ERROR         = 2,
} cln_state_t;

static volatile cln_state_t cln_state = CLN_STATE_UNINITIALIZED;
static volatile unsigned int cl_stack_backup = 0;
static volatile unsigned int cl_gp_backup = 0;
static volatile unsigned int cln_boot_frames = 0;

static cl_error cln_abi_display_message(unsigned level, const char *msg)
{
  char buffer[512];
  const char *level_str;

  switch (level)
  {
    case CL_MSG_DEBUG: level_str = "[DEBUG] "; break;
    case CL_MSG_INFO: level_str = "[INFO ] "; break;
    case CL_MSG_WARN: level_str = "[WARN ] "; break;
    case CL_MSG_ERROR: level_str = "[ERROR] "; break;
    default: level_str = "[UNKWN] "; break;
  };

  snprintf(buffer, sizeof(buffer), "%s%s\n", level_str, msg);
  cl64_usb_transmit(buffer, CL64_DATATYPE_MESSAGE_DEBUG + level,
    strlen(buffer) + 1);

  return CL_OK;
}

static cl_memory_region_t cln_region;
static cl_error result = 0;

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
  snprintf(cln_region.title, sizeof(cln_region.title), "N64 RDRAM");
  *region_count = 1;

  return CL_OK;
}

static cl_error cln_abi_library_name(const char **name)
{
  *name = "Classics Live 64";
  return CL_OK;
}

static cl_error cln_abi_network_post(const char *url, char *post_data,
  cl_network_cb_t callback, void *userdata)
{
  cl_network_response_t response;
  char send_buffer[512];
  char recv_buffer[512];

  response.error_code = 0;
  response.error_msg = NULL;

  snprintf(send_buffer, sizeof(send_buffer), "%s\n%s",
    url, post_data ? post_data : "");
  cl64_usb_transmit(send_buffer, CL64_DATATYPE_NETWORK_POST,
    strlen(send_buffer) + 1);

  cl64_usb_receive(recv_buffer, sizeof(recv_buffer));
  response.data = recv_buffer;

  if (callback)
    callback(response, userdata);

  return CL_OK;
}

static cl_error cln_abi_set_pause(unsigned mode)
{
  return CL_OK;
}

static cl_error cln_abi_thread(cl_task_t *task)
{
  /* Run tasks immediately */
  task->error = NULL;
  task->handler(task);
  if (task->callback)
    task->callback(task->state);
  if (task->state)
    free(task->state);
  free(task);

  return CL_OK;
}

static cl_error cln_abi_user_data(cl_user_t *user, unsigned index)
{
  char buffer[256];

  cl64_usb_transmit("", CL64_DATATYPE_REQUEST_USER_INFO, 0);
  cl64_usb_receive(buffer, sizeof(buffer));

  user->password[0] = '\0';
  if (!cl_json_get(user->username, buffer, CL_JSON_KEY_USERNAME,
        CL_JSON_TYPE_STRING, sizeof(user->username)) ||
      !cl_json_get(user->token, buffer, CL_JSON_KEY_TOKEN_CLINT,
        CL_JSON_TYPE_STRING, sizeof(user->token)))
    return CL_ERR_USER_CONFIG;

  return CL_OK;
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
      cln_abi_set_pause,
      cln_abi_thread,
      cln_abi_user_data
    },
    { NULL, NULL, NULL, NULL }
  },
};

void cln_run(void)
{
  if (cln_state == CLN_STATE_UNINITIALIZED)
  {
    if (cln_boot_frames < 120)
    {
      /* Wait for some frames to pass to allow USB initialization */
      cln_boot_frames++;
      return;
    }
    int error = 0;

    result += 10;

    /* Try USB initialize */
    error = cl64_usb_init();
    if (error != CL_OK)
    {
      cln_state = CLN_STATE_ERROR;
      return;
    }
    result += 10;

    /* Try ABI register */
    error = cl_abi_register(&cln_abi);
    if (error != CL_OK)
    {
      cln_state = CLN_STATE_ERROR;
      result += error;
      return;
    }
    result += 10;

    /* Try login */
    cl_game_identifier_t identifier;
    identifier.type = CL_GAMEIDENTIFIER_FILE_HASH;
    snprintf(identifier.filename, sizeof(identifier.filename),
      "SummerCart64.z64");
    snprintf(identifier.checksum, sizeof(identifier.checksum),
      "20b854b239203baf6c961b850a4a51a2");
    error = cl_login_and_start(identifier);
    if (error != CL_OK)
    {
      cln_state = CLN_STATE_ERROR;
      result += error;
      return;
    }
    result += 10;
    result += session.state;

    cln_state = CLN_STATE_INITIALIZED;
  }
  else if (cln_state == CLN_STATE_ERROR)
    *SM64_COINS = result;
  else if (cln_state == CLN_STATE_INITIALIZED)
    cl_run();
}

__attribute__((used, noinline, optimize("O0")))
void _start(void)
{
  asm volatile (
    ".set noreorder\n"

    /* Save the stack/global pointer of the calling thread in static memory */
    "sw     $sp, cl_stack_backup\n"
    "sw     $gp, cl_gp_backup\n"

    /* Switch to our own stack/global pointer */
    "lui    $sp, 0x8060\n"
    "la     $gp, _gp\n"

    /* Save every register */
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

    /* Run our C function */
    "jal    cln_run\n"
    "nop\n"

    /* Restore every register */
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

    /* Restore the original stack pointer */
    "lw     $sp, cl_stack_backup\n"
    "lw     $gp, cl_gp_backup\n"

    /* Return to game -- The compiler will output a JR RA here */
    ".set at\n"
    ".set reorder\n"
  );
}
