lib_LTLIBRARIES = libinterflop_sqrt.la

if ENABLE_LTO
LTO_FLAGS = -flto
else
LTO_FLAGS =
endif

if ENABLE_WARNINGS
WARNING_FLAGS = -Wall -Wextra -Wno-varargs
else
WARNING_FLAGS = 
endif

if HAVE_QUADMATH
QUADMATH_FLAGS = -DHAS_QUADMATH
else
QUADMATH_FLAGS =
endif


libinterflop_sqrt_la_SOURCES = \
    interflop_sqrt.c
libinterflop_sqrt_la_CFLAGS = \
    $(LTO_FLAGS) -O3 \
    -fno-stack-protector \
    -I../$(top_srcdir) \
    $(WARNING_FLAGS) \
    $(QUADMATH_FLAGS) 
libinterflop_sqrt_la_LDFLAGS = \
    $(LTO_FLAGS) -O3 -lm $(QUADMATH_LIBS)


library_includedir = $(includedir)/
