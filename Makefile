sources=$(wildcard *.c)
objects=$(patsubst %.c,%.o,$(sources))

CC=gcc
CFLAGS=

LD=gcc
LDFLAGS=-lm


all: $(objects)
	$(LD) -o cmdcalc $(objects) $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf *.o

.PHONY: all clean
