#include <libdragon.h>

#define N64_PAYLOAD_LOCATION ((volatile void*)0x80400000)

#define REDIR_INTR_ENTRY (0x80000120)
#define REDIR_INTR_ENTRY_CACHED (REDIR_INTR_ENTRY | 0x20000000)

#define INTR_ENTRY (0x80000180)
#define WATCH_ENTRY (0x80000190)

#define CODE_HANDLER (0x80400000)

#define INTR_HANDLER (0x800002C0)
#define WATCH_HANDLER (0x807FF000)

#define MI_INTR ((uint32_t)MI_INTERRUPT)

#define MI_INTR ((uint32_t)MI_INTERRUPT)

void setup_cheats(void)
{
  const uint32_t watch_entry[] = {
        0x3C1A0000 | ((WATCH_HANDLER >> 16) & 0xFFFF), // lui $k0, %HI(WATCH_HANDLER)
        0x375A0000 | (WATCH_HANDLER & 0xFFFF),         // ori $k0, $k0, %LO(WATCH_HANDLER)
        0x03400008,                                    // jr  $k0
        0x00000000,                                    //  nop
    };

  memcpy((void*)WATCH_ENTRY, watch_entry, sizeof(watch_entry));

  const uint32_t intr_handler[] = {
        0x401A6800,                                      //  mfc0 $k0, Cause
        0x341B0000 | (EXCEPTION_CODE_WATCH << 2),        //  ori  $k1, $zero, (EXCEPTION_CODE_WATCH << 2)
        0x335A007C,                                      //  andi $k0, $k0,   0x007C
        0x135B0009,                                      //  beq  $k0, $k1,   2f
        0x00000000,                                      //   nop
        0x17400005,                                      //  bnez $k0,        1f
        0x3C1A0000 | ((MI_INTR >> 16) & 0xFFFF),         //   lui $k0, %HI(MI_INTERRUPT)
        0x8F5A0000 | (MI_INTR & 0xFFFF),                 //  lw   $k0, %LO(MI_INTERRUPT)($k0)
        0x335B0000 | (MI_INTERRUPT_SI),                  //  andi $k1, $k0,   MI_INTERRUPT_SI
        0x17600001,                                      //  bnez $k1,        1f
        0x00000000,                                      //   nop
                                                         // 1:
        0x08000000 | ((CODE_HANDLER & 0x0FFFFFFF) >> 2), //  j CODE_HANDLER
        0x00000000,                                      //   nop
                                                         // 2:
        0x08000000 | ((WATCH_ENTRY & 0x0FFFFFFF) >> 2),  //  j WATCH_ENTRY
        0x00000000,                                      //   nop
  };

  memcpy((void*)UncachedAddr(INTR_HANDLER), intr_handler, sizeof(intr_handler));

  const uint32_t watch_handler[] =
  {
    0x0c100000, // jal _start (payload entry point)
    0x00000000, // nop
    0x42000018, // eret
    0x00000000, // nop
  };

  memcpy((void*)UncachedAddr(WATCH_HANDLER), watch_handler, sizeof(watch_handler));

  // set watchpoint on INTR_ENTRY write
  __asm__ volatile("mtc0 %0, $18\n" : : "r"((0x80000080 & ~0x80000000) | ((1 << 0))));
  __asm__ volatile("mtc0 %0, $19\n" : : "r"((1 << 30)));
}

#define SM64_COINS ((volatile unsigned short*)0x8033B218)

int main(void)
{
  FILE *payload_file = NULL;
  size_t payload_size = 0;
  size_t bytes_read = 0;
  joypad_buttons_t buttons;

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

  printf("Invalidating cache...\n");
  data_cache_writeback_invalidate_all();
  inst_cache_invalidate_all();

  printf("Payload read successfully.\n\n");
  printf("Press L to test the payload, or RESET to exit.\n");

  joypad_init();
  while (1)
  {
    joypad_poll();
    buttons = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if (buttons.l)
      break;
  }

  printf("Jumping to payload...\n");
  void (*payload_entry)(void) = (void*)0x80400000;
  payload_entry();
  printf("\nPayload tested successfully! Result: %u\n", *SM64_COINS);

  return 0;
}
