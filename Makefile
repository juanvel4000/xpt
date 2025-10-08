CC ?= gcc

MAIN_C     = xpt/main.c
COMMON_C   = xpt/common.c
MANAGER_C  = xpt/manager.c
PACKAGES_C = xpt/packages.c
MANIFESTS_C= xpt/manifests.o

MAIN_O     = xpt/main.o
COMMON_O   = xpt/common.o
MANAGER_O  = xpt/manager.o
PACKAGES_O = xpt/packages.o
MANIFESTS_O= xpt/manifests.o

CFLAGS ?=
CFLAGS += -Ixpt $(shell pkg-config --cflags libarchive)

LDFLAGS ?=
LDFLAGS += -larchive

TARGET  = xpt/xpt
TARGETS = $(COMMON_O) $(MANAGER_O) $(PACKAGES_O) $(MANIFESTS_O) $(MAIN_O)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(TARGETS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(TARGETS)

all: $(TARGET)

clean:
	rm $(TARGET)
	rm $(TARGETS)
.PHONY: all
