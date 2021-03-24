CFLAGS += -std=c99 -Wall -Wextra -pedantic -Wold-style-declaration
CFLAGS += -Wmissing-prototypes -Wno-unused-parameter
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
CC     ?= gcc

all: config.h samu

config.h:
	cp config.def.h config.h

samu: 
	$(CC) -O3 $(CFLAGS) -o samu samu.c -lX11 -lXext -lXinerama $(LDFLAGS)

install: all
	install -Dm755 samu $(DESTDIR)$(BINDIR)/samu

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/samu

clean:
	rm -f samu *.o
