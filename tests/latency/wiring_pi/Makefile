CC = gcc
SKIN = alchemy
CFLAGS = -std=gnu99 $(shell arm-linux-gnueabihf-xeno-config --skin=$(SKIN) --cflags)
LDFLAGS = -lwiringPi $(shell arm-linux-gnueabihf-xeno-config --skin=$(SKIN) --ldflags)

all:
	$(CC) $(CFLAGS) $(LDFLAGS) wiring_latency.c -o wiring_latency

clean:
	rm wiring_latency
