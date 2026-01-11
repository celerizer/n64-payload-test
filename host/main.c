#include <libusb-1.0/libusb.h>
#include <curl/curl.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define EP_IN 0x81
#define EP_OUT 0x02
#define USB_TIMEOUT 0

#define CL64_MAGIC_N64_TO_PC 0x36343E50
#define CL64_MAGIC_PC_TO_N64 0x503E3634

#define SC64_BRAM 0x1FFE0000
#define SC64_BRAM_SIZE CL_KB(8)

typedef enum
{
  CL64_DATATYPE_INVALID = 0,

  CL64_DATATYPE_LIBDRAGON_TEXT = 1,
  CL64_DATATYPE_LIBDRAGON_HEARTBEAT = 5,

  CL64_DATATYPE_MESSAGE_DEBUG = 0xc0,
  CL64_DATATYPE_MESSAGE_INFO = 0xc1,
  CL64_DATATYPE_MESSAGE_WARN = 0xc2,
  CL64_DATATYPE_MESSAGE_ERROR = 0xc3,

  CL64_DATATYPE_NETWORK_POST = 0xc4,
  CL64_DATATYPE_REQUEST_USER_INFO = 0xc5,

  CL64_DATATYPE_SIZE
} cl64_datatype;

typedef struct
{
  char identifier[3];
  char command;
  unsigned arg0;
  unsigned arg1;
  char data[496];
} cmd_packet_t;

int aux_write(libusb_device_handle *handle, const void *data, int length)
{
  cmd_packet_t packet;
  int transferred;
  int rc;

  packet.identifier[0] = 'C';
  packet.identifier[1] = 'M';
  packet.identifier[2] = 'D';
  packet.command = 'M';
  packet.arg0 = __builtin_bswap32(0x05000000);
  packet.arg1 = __builtin_bswap32(length + 8);
  *(unsigned*)(&packet.data[0]) = __builtin_bswap32(CL64_MAGIC_PC_TO_N64);
  *(unsigned*)(&packet.data[4]) = __builtin_bswap32(length);
  memcpy(&packet.data[8], data, length);

  rc = libusb_bulk_transfer(
    handle,
    EP_OUT,
    (unsigned char*)&packet,
    20 + length,
    &transferred,
    USB_TIMEOUT
  );

  if (rc != 0)
  {
    printf("aux_write: libusb_bulk_transfer error %d\n", rc);
    return rc;
  }

  if (transferred != 20 + length)
  {
    printf("aux_write: transferred %d bytes, expected %d\n",
      transferred, 20 + length);
    return LIBUSB_ERROR_IO;
  }

  return 0;
}

struct string
{
  char *ptr;
  size_t len;
};

static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp)
{
  size_t total = size * nmemb;
  struct string *s = (struct string *)userp;
  char *tmp = realloc(s->ptr, s->len + total + 1);

  if (!tmp)
    return 0;
  s->ptr = tmp;
  memcpy(s->ptr + s->len, data, total);
  s->len += total;
  s->ptr[s->len] = '\0';

  return total;
}

int network_post(libusb_device_handle *handle, char *data, int length)
{
  char *newline = memchr(data, '\n', length);

  if (newline)
    *newline = '\0';

  char *url = data;
  char *post_data = newline ? newline + 1 : "";

  CURL *curl = curl_easy_init();
  if (!curl)
    return 1;

  struct string response;
  response.ptr = malloc(1);
  response.len = 0;
  response.ptr[0] = '\0';

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  /* Set timeout (example: 10 seconds) */
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

  CURLcode res;

  for (;;)
  {
    /* reset response buffer */
    response.len = 0;
    response.ptr[0] = '\0';

    res = curl_easy_perform(curl);

    if (res == CURLE_OK)
      break;

    if (res == CURLE_OPERATION_TIMEDOUT ||
        res == CURLE_COULDNT_CONNECT ||
        res == CURLE_COULDNT_RESOLVE_HOST)
    {
      /* timeout or transient error -- retry */
      fprintf(stderr, "curl timeout, retrying...\n");
      sleep(1); /* avoid hammering */
      continue;
    }

    /* non-timeout error -- fail */
    fprintf(stderr, "curl error: %s\n", curl_easy_strerror(res));
    break;
  }

  if (res == CURLE_OK)
  {
    printf("%s\n", response.ptr);
    aux_write(handle, response.ptr, response.len + 1);
  }

  free(response.ptr);
  curl_easy_cleanup(curl);

  return (res == CURLE_OK) ? 0 : 1;
}

#define CL64_SUMMERCART_VENDOR_ID 0x0403
#define CL64_SUMMERCART_PRODUCT_ID 0x6014

typedef struct
{
  char username[256];
  char password[256];
  char token_clint[256];
  char language[16];
} cl64_user_config_t;

static cl64_user_config_t cl64_user_config;

static int cl64_get_user_config(void)
{
  FILE *f = fopen("login.txt", "r");
  char line[512];

  if (!f)
  {
    fprintf(stderr, "Failed to open login.txt\n");
    return -1;
  }

  while (fgets(line, sizeof(line), f))
  {
    if (sscanf(line, "username=%255s", cl64_user_config.username) == 1)
      continue;
    if (sscanf(line, "password=%255s", cl64_user_config.password) == 1)
      continue;
    if (sscanf(line, "token_clint=%255s", cl64_user_config.token_clint) == 1)
      continue;
    if (sscanf(line, "language=%15s", cl64_user_config.language) == 1)
      continue;
  }
  fclose(f);

  if (cl64_user_config.username[0] == '\0')
  {
    fprintf(stderr, "Username not found in login.txt\n");
    return -1;
  }

  if (cl64_user_config.password[0] == '\0' &&
      cl64_user_config.token_clint[0] == '\0')
  {
    fprintf(stderr, "Password / token not found in login.txt\n");
    return -1;
  }

  return 0;
}

int main(void)
{
  libusb_context *ctx;
  libusb_device_handle *handle;
  unsigned char buffer[512];
  int transferred;

  libusb_init(&ctx);

  handle = libusb_open_device_with_vid_pid(ctx,
    CL64_SUMMERCART_VENDOR_ID, CL64_SUMMERCART_PRODUCT_ID);

  if (!handle)
  {
    fprintf(stderr, "Device not found\n");
    return 1;
  }

  if (cl64_get_user_config() != 0)
  {
    fprintf(stderr, "Failed to get user config.\n"
      "Please provide your Classics Live username and password in login.txt.\n");
    return 1;
  }

  libusb_reset_device(handle);
  libusb_claim_interface(handle, 0);

  while (1)
  {
    int rc = libusb_bulk_transfer(
      handle,
      EP_IN,
      buffer,
      sizeof(buffer),
      &transferred,
      USB_TIMEOUT
    );

    if (rc == 0 && transferred > 10)
    {
      cl64_datatype datatype = (cl64_datatype)buffer[10]; 
      unsigned length = (buffer[11] << 16) | (buffer[12] << 8) | buffer[13];

      for (int i = 0; i < transferred; i++)
        printf("%02X ", buffer[i]);
      printf("\n");

      switch (datatype)
      {
      case CL64_DATATYPE_LIBDRAGON_TEXT:
        printf("libdragon: %.*s", length, &buffer[14]);
        break;
      case CL64_DATATYPE_LIBDRAGON_HEARTBEAT:
        printf("libdragon heartbeat\n");
        break;
      case CL64_DATATYPE_MESSAGE_DEBUG:
      case CL64_DATATYPE_MESSAGE_INFO:
      case CL64_DATATYPE_MESSAGE_WARN:
      case CL64_DATATYPE_MESSAGE_ERROR:
        printf("CL: %.*s\n", length, &buffer[22]);
        break;
      case CL64_DATATYPE_NETWORK_POST:
        printf("CL: Network Post (%d bytes): %.*s\n", length, length, &buffer[22]);
        network_post(handle, (char*)&buffer[22], length);
        break;
      case CL64_DATATYPE_REQUEST_USER_INFO:
      {
        char response[1024];
        int length;

        printf("CL: Request User Info\n");
        if (cl64_user_config.password[0] == '\0')
        {
          length = snprintf(response, sizeof(response),
            "{\"username\":\"%s\",\"token_clint\":\"%s\",\"language\":\"%s\"}",
            cl64_user_config.username,
            cl64_user_config.token_clint,
            cl64_user_config.language);
        }
        else
        {
          length = snprintf(response, sizeof(response),
            "{\"username\":\"%s\",\"password\":\"%s\",\"language\":\"%s\"}",
            cl64_user_config.username,
            cl64_user_config.password,
            cl64_user_config.language);
        }
        aux_write(handle, response, length + 1);
        break;
      }
      default:
        printf("Unknown datatype %02X (%d bytes)\n", datatype, length);
        break;
      }
    }
  }
  libusb_release_interface(handle, 0);
  libusb_close(handle);
  libusb_exit(ctx);

  return 0;
}
