.DEFAULT_GOAL := all

CC ?= gcc

CFLAGS ?= -Wall -Wextra
CFLAGS += -Iinclude -Ilibfetch $(shell pkg-config --cflags libarchive) -MMD -MP

LDFLAGS ?=
LDFLAGS += -Llibfetch -larchive -lgdbm -lz -lssl -lcrypto -lfetch

TARGET  = xpt/xpt
SOURCES = \
	xpt/common.c \
	xpt/database.c \
	xpt/manager.c \
	xpt/packages.c \
	xpt/manifests.c \
	xpt/resolver.c \
	xpt/repo.c \
	xpt/main.c

TARGETS = $(SOURCES:.c=.o)

VERSION ?= $(shell git describe --tags --abbrev=0 2>/dev/null || echo "0.3.0")
CFLAGS += -DXPT_VERSION=\"$(VERSION)\"

DISTDIR = xpt-$(VERSION)
DISTFILE = $(DISTDIR).tar.gz
DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

DOCS ?= 1

%.o: %.c
	@echo " CC $@"
	@$(CC) $(CFLAGS) -c $< -o $@

libfetch/libfetch.a:
	$(MAKE) -C libfetch all

$(TARGET): $(TARGETS) libfetch/libfetch.a
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
	$(MAKE) -C libfetch clean
	$(MAKE) -C docs clean

debug: CFLAGS += -g -O0 -Wpedantic
debug: clean all

static: CFLAGS += -Os
static: LDFLAGS += -static
static: all

dist: clean
	@echo " RM $(DISTDIR)"
	@rm -rf $(DISTDIR)
	@echo " MKDIR $(DISTDIR)"
	@mkdir -p $(DISTDIR)
	@echo " CP ./xpt Makefile LICENSE"
	@cp -a ./xpt Makefile LICENSE $(DISTDIR)
	@echo " SED $(DISTDIR)/Makefile"
	sed 's/^VERSION .*/VERSION ?= $(VERSION)/' \
		$(DISTDIR)/Makefile > $(DISTDIR)/Makefile.tmp
	mv $(DISTDIR)/Makefile.tmp $(DISTDIR)/Makefile
	@echo " TAR $(DISTFILE)"
	@tar -czf $(DISTFILE) $(DISTDIR)
	@echo " RM $(DISTDIR)"
	@rm -rf $(DISTDIR)

install: all
	@echo " INSTALL $(TARGET)"
	@install -dm755 $(DESTDIR)$(BINDIR)
	@install -m755 $(TARGET) $(DESTDIR)$(BINDIR)/xpt
	@if [ "$(DOCS)" = "1" ]; then \
		$(MAKE) -C docs install; \
	fi

install-docs:
	$(MAKE) -C docs install

-include $(TARGETS:.o=.d)
.PHONY: all clean debug install dist static install-docs
