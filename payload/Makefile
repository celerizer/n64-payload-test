N64_TARGET := mips64-elf
N64_PREFIX := $(N64_INST)/bin/$(N64_TARGET)-

CC      := $(N64_PREFIX)gcc
LD      := $(N64_PREFIX)ld
OBJCOPY := $(N64_PREFIX)objcopy

CFLAGS := \
    -march=vr4300 -mtune=vr4300 -mabi=32 \
    -ffreestanding -fno-builtin -fno-stack-protector \
    -fno-pic \
    -mno-gpopt -G0 \
    -Os -Wall

LDFLAGS := \
    -T link.ld \
    -nostdlib \
    -G0

SRCS := main.c
OBJS := $(SRCS:.c=.o)

all: payload.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

payload.elf: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

payload.bin: payload.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(OBJS) payload.elf payload.bin

.PHONY: all clean
