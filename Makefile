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

CFLAGS ?= -MMD -MP 
CFLAGS += -Ixpt $(shell pkg-config --cflags libarchive)

LDFLAGS ?=
LDFLAGS += -larchive -lgdbm

TARGET  = xpt/xpt
TARGETS = $(COMMON_O) $(DATABASE_O) $(MANAGER_O) $(PACKAGES_O) $(MANIFESTS_O) $(MAIN_O) 

%.o: %.c
	@echo "CC $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(TARGETS)
	@echo "LD $@"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(TARGETS)

all: $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f $(TARGETS)
	rm -f $(TARGETS:.o=.d)

debug: CFLAGS += -g -O0 -Wall -Wextra -Wpedantic
debug: clean all

-include $(TARGETS:.o=.d)
.PHONY: all clean debug
