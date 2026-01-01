N64_ROOTDIR = $(N64_INST)
N64_TARGET := mips64-elf
N64_PREFIX := $(N64_INST)/bin/$(N64_TARGET)-
N64_INCLUDEDIR = $(N64_ROOTDIR)/$(N64_TARGET)/include
N64_LIBDIR = $(N64_ROOTDIR)/$(N64_TARGET)/lib

CC      := $(N64_PREFIX)gcc
LD      := $(N64_PREFIX)ld
OBJCOPY := $(N64_PREFIX)objcopy

CFLAGS := \
    -march=vr4300 -mtune=vr4300 -mabi=o64 \
    -G0 \
    -ffunction-sections \
    -fdata-sections \
    -fno-stack-protector \
    -fno-pic \
    -fomit-frame-pointer \
    -Iultralib/include \
    -I. \
    -Os -Wall \
    -DLIBDRAGON=1 \
    -DN64 \
    -Ilibretro-common/include \
    -I$(N64_INCLUDEDIR) \

LDFLAGS := \
    -T link.ld \
    -L$(N64_LIBDIR) \
    -lc -lm -ldragon \
    -Wl,--gc-sections

SRC_DIRS := \
    . \
    N64-UNFLoader \

C_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

# Setup Classics Live integration
CLASSICS_LIVE_DIR = classicslive-integration
CL_OMIT_VFS := 1
include classicslive-integration/classicslive-integration.mk
C_SRCS += $(CLASSICS_LIVE_SOURCES)

OBJS := $(C_SRCS:.c=.o)

all: payload.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

payload.elf: $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

payload.bin: payload.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(OBJS) payload.elf payload.bin

.PHONY: all clean
