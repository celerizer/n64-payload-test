#include <libdragon.h>

#define N64_PAYLOAD_LOCATION ((volatile void*)0x80400000)

int main(void)
{
  FILE *payload_file = NULL;
  size_t payload_size = 0;
  size_t bytes_read = 0;

  console_init();
  console_set_render_mode(RENDER_AUTOMATIC);
  console_clear();

  printf("N64 Payload Injector\n\n");

  printf("Opening SD Card...\n");
  debug_init_sdfs("sd:/", -1);

  printf("Opening payload file...\n");
  payload_file = fopen("sd:/payload.bin", "rb");
  if (!payload_file)
  {
    printf("Error: Could not open payload file.\n");
    return -1;
  }

  /* Determine file size */
  fseek(payload_file, 0, SEEK_END);
  payload_size = ftell(payload_file);
  rewind(payload_file);

  if (payload_size == 0)
  {
    printf("Error: Payload file is empty.\n");
    fclose(payload_file);
    return -1;
  }

  printf("Payload size: %lu bytes\n", (unsigned long)payload_size);
  printf("Reading payload into memory...\n");

  bytes_read = fread((void*)N64_PAYLOAD_LOCATION, 1, payload_size, payload_file);
  fclose(payload_file);

  if (bytes_read != payload_size)
  {
    printf("Error: Read %lu of %lu bytes.\n",
           (unsigned long)bytes_read,
           (unsigned long)payload_size);
    return -1;
  }

  printf("\nPayload loaded successfully!\n");

  return 0;
}
