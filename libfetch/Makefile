CC ?= cc
AR ?= ar

CFLAGS ?= -O2

CPPFLAGS ?=
CPPFLAGS += \
	-D_XOPEN_SOURCE=700 \
	-D_GNU_SOURCE -MMD \
	-MP

LDFLAGS ?=
LDLIBS += -lssl

LIB = libfetch.a

SRCS = \
	common.c \
	fetch.c \
	http.c \
	openssl-compat.c

OBJS = $(SRCS:.c=.o)

all: $(LIB)

$(LIB): $(OBJS)
	@echo " AR $@"
	@$(AR) rcs $@ $^

%.o: %.c
	@echo " CC $@"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	@echo " RM $(OBJS)"
	@rm -f $(OBJS)
	@echo " RM $(LIB)"
	@rm -f $(LIB)
	@echo " RM $(OBJS:.o=.d)"
	@rm -f $(OBJS:.o=.d)

.PHONY: all clean
-include $(OBJS:.o=.d)
