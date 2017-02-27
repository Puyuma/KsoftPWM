CC = $(shell arm-linux-gnueabihf-xeno-config --cc)
CFLAGS = -std=gnu99
SKIN = posix

CFLAGS += $(shell arm-linux-gnueabihf-xeno-config --skin=$(SKIN) --cflags)
LDFLAGS := $(shell arm-linux-gnueabihf-xeno-config --skin=$(SKIN) --ldflags)

OBJS = main.c pwm_user.c

all:
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o main

clean:
	@rm main
