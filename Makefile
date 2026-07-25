CC ?= clang
CFLAGS ?= -Wall -Wextra -O2
LIBS = -framework CoreFoundation -framework IOKit
PREFIX ?= $(HOME)/.local

.PHONY: all install clean

all: trackpad-haptic

trackpad-haptic: trackpad-haptic.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

install: trackpad-haptic
	install -d $(PREFIX)/bin
	install -m 755 trackpad-haptic $(PREFIX)/bin/trackpad-haptic

clean:
	rm -f trackpad-haptic
