CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -O2
LIBS    ?= -lusb-1.0 -lcurl

TARGET  = host
SRCS    = main.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
