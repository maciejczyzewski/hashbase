bin_PROGRAMS = hashbase

AM_CPPFLAGS = -D_GNU_SOURCE

AM_CFLAGS = -O2 -Wall
AM_CFLAGS += -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls
AM_CFLAGS += -fno-strict-aliasing

LDFLAGS += -rdynamic -lpthread

hashbase_SOURCES =              \
    hb_core.c hb_core.h         \
    hb_net.c hb_net.h           \
    hb_map.c hb_map.h           \
    hb_pipe.c hb_pipe.h         \
    hb_util.c hb_util.h         \
    hb.c
