SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

OBJS = $(patsubst %.c, %.o, $(SOURCES))

CFLAGS ?= -g
CC ?= gcc
LIBS = -lnotmuch -ltcl

mailfile: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<
