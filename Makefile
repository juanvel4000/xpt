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
LDFLAGS += -larchive -lgdbm

TARGET  = xpt/xpt
TARGETS = $(COMMON_O) $(DATABASE_O) $(MANAGER_O) $(PACKAGES_O) $(MANIFESTS_O) $(MAIN_O) 

VERSION ?= $(shell git describe --tags --abbrev=0 2>/dev/null || echo "0.1.0")
CFLAGS += -DXPT_VERSION=\"$(VERSION)\"

%.o: %.c
	@echo " CC $@"
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(TARGETS)
	@echo " LD $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(TARGETS)

all: $(TARGET)

clean:
	@echo " RM $(TARGET)"
	@rm -f $(TARGET)
	@echo " RM $(TARGETS)"
	@rm -f $(TARGETS)
	@echo " RM $(TARGETS:.o=.d)"
	@rm -f $(TARGETS:.o=.d)

debug: CFLAGS += -g -O0 -Wall -Wextra -Wpedantic
debug: clean all

-include $(TARGETS:.o=.d)
.PHONY: all clean debug
