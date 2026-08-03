.DEFAULT_GOAL := all
GZIP ?= gzip

SOURCE = xpt.8
TARGET = xpt.8.gz

DESTDIR ?= /
PREFIX ?= /usr/local
MANDIR ?= $(PREFIX)/share/man

$(TARGET): $(SOURCE)
	@echo " GZIP $(TARGET)"
	@$(GZIP)	-k $(SOURCE)

all: $(TARGET)

install: all
	@echo " INSTALL $(TARGET)"
	@install -d $(DESTDIR)$(MANDIR)/man8
	@install -m644 $(TARGET) $(DESTDIR)$(MANDIR)/man8

clean:
	@echo " RM $(TARGET)"
	@rm -f $(TARGET)

.PHONY: all install clean
