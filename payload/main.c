#define SM64_COINS ((volatile unsigned short*)0x8033B218)
#define SM64_LIVES ((volatile signed char*)0x8033B21D)

__attribute__((used, noinline, optimize("O0")))
void _start(void)
{
  *SM64_COINS += 1;
  if (*SM64_COINS >= 100)
    *SM64_COINS = 0;
  *SM64_LIVES += 1;
  if (*SM64_LIVES >= 100)
    *SM64_LIVES = 0;
}
