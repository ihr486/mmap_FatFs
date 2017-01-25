CSRCS := $(wildcard *.c)
COBJS := $(CSRCS:%.c=%.o)
EXE := fatfs
CC := gcc
CFLAGS := -std=c99 -g -Wall -Wextra -Wno-switch $(shell pkg-config --cflags libusb-1.0)
LDFLAGS := $(shell pkg-config --libs libusb-1.0)
.PHONY: all clean
all: $(EXE)
clean:
	-@rm -vf $(COBJS) $(EXE)
$(EXE): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
