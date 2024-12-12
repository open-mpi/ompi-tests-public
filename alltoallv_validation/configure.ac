# -*- shell-script -*-
#
# Copyright (c) 2024      Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# $HEADER$
#
# modified from ompi-tests/cxx-test-suite's autoconf by Luke Robison 2024.

#
# Init autoconf
#

AC_PREREQ([2.63])
AC_INIT([alltoallv_validation], [1.0], [devel@open-mpi.org], [openmpi-cxx-test-suite])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])

#
# Get the version of ompitest that we are configuring
#

echo "Configuring Open MPI C++ test suite"

AM_INIT_AUTOMAKE([1.10 foreign dist-bzip2 no-define])

# If Automake supports silent rules, enable them.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Setup the reporting/ompitest_config.h file

AH_TOP([/* -*- c -*-
 *
 *	ompitest configuation header file.
 *
 * Function: - OS, CPU and compiler dependent configuration
 */

#ifndef OMPITEST_CONFIG_H
#define OMPITEST_CONFIG_H
])
AH_BOTTOM([#endif /* OMPITEST_CONFIG_H */])

#
# This is useful later
#

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED(OMPITEST_ARCH, "$host",
    [Architecture that we are compiled for])

#
# We always want debugging flags
#
CXXFLAGS="$CXXFLAGS -g"
CFLAGS="$CFLAGS -g"

#
# Get various programs
# C compiler - bias towards mpicc
#

if test "$CC" != ""; then
    BASE="`basename $CC`"
else
    BASE=
fi
if test "$BASE" = "" -o "$BASE" = "." -o "$BASE" = "cc" -o \
    "$BASE" = "gcc" -o "$BASE" = "xlc" -o \
    "$BASE" = "icc" -o "$BASE" = "pgcc"; then
    AC_CHECK_PROG(HAVE_MPICC, mpicc, yes, no)
    if test "$HAVE_MPICC" = "yes"; then
        CC=mpicc
        export CC
    fi
fi

CFLAGS_save="$CFLAGS"
AC_PROG_CC
CFLAGS="$CFLAGS_save"

#
# Get various programs
# C++ compiler - bias towards mpic++, with fallback to mpiCC
#

if test "$CXX" != ""; then
    BASE="`basename $CXX`"
else
    BASE=
fi
if test "$BASE" = "" -o "$BASE" = "." -o "$BASE" = "CC" -o \
    "$BASE" = "g++" -o "$BASE" = "c++" -o "$BASE" = "xlC" -o \
    "$BASE" = "icpc" -o "$BASE" = "pgCC"; then
    AC_CHECK_PROG(HAVE_MPICPP, mpic++, yes, no)
    if test "$HAVE_MPICPP" = "yes"; then
        CXX=mpic++
        export CXX
    else
        AC_CHECK_PROG(HAVE_MPICXX, mpiCC, yes, no)
        if test "$HAVE_MPICXX" = "yes"; then
            CXX=mpiCC
            export CXX
        fi
    fi
fi

CXXFLAGS_save="$CXXFLAGS"
AC_PROG_CXX
CXXFLAGS="$CXXFLAGS_save"

#
# Find ranlib
#

AC_PROG_RANLIB

#
# Ensure that we can compile and link an MPI program
#

# See if we can find <mpi.h>
AC_CHECK_HEADER([mpi.h], [], 
                [AC_MSG_WARN([Cannot find mpi.h])
                    AC_MSG_ERROR([Cannot continue])
                ])

#
# See if we can find the symbol MPI_Init.  Be a little smart and use
# AC CHECK_FUNC if we're using mpicc, or AC CHECK_LIB otherwise.
# Aborts if MPI_Init is not found.
#
base=`basename $CC`
bad=0
AS_IF([test "$base" = "mpicc"],
        [AC_CHECK_FUNC([MPI_Init], [], [bad=1])],
        [AC_CHECK_LIB([mpi], [MPI_Init], [], [bad=1])])

AS_IF([test "$bad" = "1"], 
        [AC_MSG_WARN([Cannot link against MPI_Init])
        AC_MSG_ERROR([Cannot continue])
        ])

#
# Party on
#

AC_CONFIG_HEADERS([src/test_config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
