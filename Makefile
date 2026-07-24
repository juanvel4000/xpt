CC ?= gcc

MAIN_C     = xpt/main.c
COMMON_C   = xpt/common.c
MANAGER_C  = xpt/manager.c
PACKAGES_C = xpt/packages.c
DATABASE_C = xpt/database.c
MANIFESTS_C= xpt/manifests.c

MAIN_O     = xpt/main.o
COMMON_O   = xpt/common.o
MANAGER_O  = xpt/manager.o
PACKAGES_O = xpt/packages.o
DATABASE_O = xpt/database.o
MANIFESTS_O= xpt/manifests.o

CFLAGS ?=
CFLAGS += -Ixpt $(shell pkg-config --cflags libarchive) -MMD -MP

LDFLAGS ?=
LDFLAGS += -larchive -lgdbm -lz

TARGET  = xpt/xpt
TARGETS = $(COMMON_O) $(DATABASE_O) $(MANAGER_O) $(PACKAGES_O) $(MANIFESTS_O) $(MAIN_O)

VERSION ?= $(shell git describe --tags --abbrev=0 2>/dev/null || echo "0.1.0")
CFLAGS += -DXPT_VERSION=\"$(VERSION)\"

DISTDIR = xpt-$(VERSION)
DISTFILE = $(DISTDIR).tar.gz

XPTMAKE = xpt-make/xpt-make.sh
XPTMAKE_TARGET = xpt-make/xpt-make

DESTDIR ?=
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin

%.o: %.c
	@echo " CC $@"
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(TARGETS)
	@echo " LD $@"
	@$(CC) $(CFLAGS) -o $(TARGET) $(TARGETS) $(LDFLAGS)

all: $(TARGET)

$(XPTMAKE_TARGET): $(XPTMAKE)
	@echo " CP $(XPTMAKE)"
	@cp $(XPTMAKE) $(XPTMAKE_TARGET)
	@echo " CHMOD $(XPTMAKE_TARGET)"
	@chmod +x  $(XPTMAKE_TARGET)

clean:
	@echo " RM $(TARGET)"
	@rm -f $(TARGET)
	@echo " RM $(TARGETS)"
	@rm -f $(TARGETS)
	@echo " RM $(TARGETS:.o=.d)"
	@rm -f $(TARGETS:.o=.d)
	@echo " RM $(XPTMAKE_TARGET)"
	@rm -f $(XPTMAKE_TARGET)

debug: CFLAGS += -g -O0 -Wall -Wextra -Wpedantic
debug: clean all

static: CFLAGS += -Os
static: LDFLAGS += -static
static: all

xpt-make: $(XPTMAKE_TARGET)
dist: clean
	@echo " RM $(DISTDIR)"
	@rm -rf $(DISTDIR)
	@echo " MKDIR $(DISTDIR)"
	@mkdir -p $(DISTDIR)
	@echo " CP ./xpt ./xpt-make Makefile LICENSE"
	@cp -a ./xpt ./xpt-make Makefile LICENSE $(DISTDIR)
	@echo " SED $(DISTDIR)/Makefile"
	@sed -i 's/^VERSION .*/VERSION ?= $(VERSION)/' $(DISTDIR)/Makefile
	@echo " TAR $(DISTFILE)"
	@tar -czf $(DISTFILE) $(DISTDIR)
	@echo " RM $(DISTDIR)"
	@rm -rf $(DISTDIR)

install: all xpt-make
	@install -dm755 $(DESTDIR)$(BINDIR)
	@install -m755 $(TARGET) $(DESTDIR)$(BINDIR)/xpt
	@install -m755 $(XPTMAKE_TARGET) $(DESTDIR)$(BINDIR)/xpt-make
-include $(TARGETS:.o=.d)
.PHONY: all clean debug xpt-make install dist static
