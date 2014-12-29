SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

OBJS = $(patsubst %.c, %.o, $(SOURCES))

CFLAGS ?= -g
CC ?= gcc
LIBS = -lnotmuch -ltcl

GLIB_CPPFLAGS = $(shell pkg-config --cflags glib-2.0)
GLIB_LDFLAGS = $(shell pkg-config --libs glib-2.0)

mailfile: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(GLIB_LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -std=gnu99 $(GLIB_CPPFLAGS) -c -o $@ $<
