.PHONY: all clean

all: injector.z64

CFLAGS += \
	-O2 -funroll-loops \
	-std=c89 -Wall -Wextra

BUILD_DIR = build
SRC_DIR = .

include $(N64_INST)/include/n64.mk

src = \
	$(SRC_DIR)/main.c

$(BUILD_DIR)/injector.elf: $(src:%.c=$(BUILD_DIR)/%.o)

# Get the current git version
GIT_VERSION := $(shell git rev-parse --short=8 HEAD)

# Define the N64 ROM title with the git version
N64_ROM_TITLE_WITH_VERSION := "Injector $(GIT_VERSION)"

injector.z64: N64_ROM_TITLE = $(N64_ROM_TITLE_WITH_VERSION)

clean:
	rm -rf $(BUILD_DIR) *.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: clean
