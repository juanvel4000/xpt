.DEFAULT_GOAL := all

VERSION ?= $(shell git describe --tags --abbrev=0 2>/dev/null || echo "0.4.1")
DISTDIR = xpt-$(VERSION)
DISTFILE = $(DISTDIR).tar.gz

DOCS ?= 1

libfetch/libfetch.a:
	$(MAKE) -C libfetch all

xpt/xpt:
	$(MAKE) -C xpt all

all: libfetch/libfetch.a xpt/xpt
	@if [ "$(DOCS)" = "1" ]; then \
		$(MAKE) -C docs all; \
	fi

clean:
	$(MAKE) -C xpt clean
	$(MAKE) -C libfetch clean
	$(MAKE) -C docs clean

debug:
	$(MAKE) -C xpt debug

static:
	$(MAKE) -C xpt static

dist: clean
	@echo " RM $(DISTDIR)"
	@rm -rf $(DISTDIR)
	@echo " MKDIR $(DISTDIR)"
	@mkdir -p $(DISTDIR)
	@echo " CP ./xpt ./include ./docs ./libfetch Makefile README.md LICENSE THIRD-PARTY-LICENSES"
	@cp -a ./xpt ./include ./docs ./libfetch Makefile README.md LICENSE THIRD-PARTY-LICENSES $(DISTDIR)
	@echo " SED $(DISTDIR)/Makefile"
	sed 's/^VERSION .*/VERSION ?= $(VERSION)/' \
		$(DISTDIR)/Makefile > $(DISTDIR)/Makefile.tmp
	mv $(DISTDIR)/Makefile.tmp $(DISTDIR)/Makefile
	@echo " TAR $(DISTFILE)"
	@tar -czf $(DISTFILE) $(DISTDIR)
	@echo " RM $(DISTDIR)"
	@rm -rf $(DISTDIR)

install: all
	$(MAKE) -C xpt install
	@if [ "$(DOCS)" = "1" ]; then \
		$(MAKE) -C docs install; \
	fi

install-docs:
	$(MAKE) -C docs install

.PHONY: all clean debug install dist static install-docs xpt/xpt libfetch/libfetch.a
