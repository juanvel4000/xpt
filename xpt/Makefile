.DEFAULT_GOAL := all

CC ?= gcc

CFLAGS ?= -Wall -Wextra
CFLAGS += -I../include -I../libfetch $(shell pkg-config --cflags libarchive) -MMD -MP

LDFLAGS ?=
LDFLAGS += -L../libfetch -larchive -lgdbm -lz -lssl -lcrypto -lfetch

TARGET  = xpt
SOURCES = \
	common.c \
	lockfile.c \
	database.c \
	manager.c \
	packages.c \
	manifests.c \
	resolver.c \
	repo.c \
	main.c

TARGETS = $(SOURCES:.c=.o)

VERSION ?= $(shell git describe --tags --abbrev=0 2>/dev/null || echo "0.4.0")
CFLAGS += -DXPT_VERSION=\"$(VERSION)\"

DISTDIR = xpt-$(VERSION)
DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

%.o: %.c
	@echo " CC $@"
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(TARGETS)
	@mkdir -p $(dir $@)
	@echo " LD $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	

all: $(TARGET)
	@if [ "$(DOCS)" = "1" ]; then \
		$(MAKE) -C docs all; \
	fi

clean:
	@echo " RM $(TARGET)"
	@rm -f $(TARGET)
	@echo " RM $(TARGETS)"
	@rm -f $(TARGETS)
	@echo " RM $(TARGETS:.o=.d)"
	@rm -f $(TARGETS:.o=.d)

debug: CFLAGS += -g -O0 -Wpedantic
debug: clean all

static: CFLAGS += -Os
static: LDFLAGS += -static
static: all

install: all
	@echo " INSTALL $(TARGET)"
	@install -dm755 $(DESTDIR)$(BINDIR)
	@install -m755 $(TARGET) $(DESTDIR)$(BINDIR)/xpt
	@install -dm644 $(DESTDIR)/var/lib/xpt
	@install -dm644 $(DESTDIR)/var/cache/xpt.repositories
	@install -dm644 $(DESTDIR)/var/cache/xpt.packages

-include $(TARGETS:.o=.d)
.PHONY: all clean debug install static

