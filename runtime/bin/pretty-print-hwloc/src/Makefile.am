#
#
#

headers =
sources =
nodist_headers =
EXTRA_DIST =

AM_CPPFLAGS = -I./include/
AM_LDFLAGS  = -lm

# Headers
headers += include/utils.h

# Source
sources += \
	get-pretty-cpu.c \
	support.c

bin_PROGRAMS = get-pretty-cpu

get_pretty_cpu_SOURCES = $(sources) $(headers)
#get_pretty_cpu_CFLAGS = $(CFLAGS_HWLOC)
#get_pretty_cpu_LDADD = $(LIBS_HWLOC)
