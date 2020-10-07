# -*- shell-script -*-
#
# Copyright (c) 2012-2020 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2020      IBM Corporatio. All rights reserved.
#
# $COPYRIGHT$
#

dnl
dnl Init autoconf
dnl

AC_PREREQ([2.67])
AC_INIT([packaging-test], [1.0], [http://www.open-mpi.org])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_SRCDIR([.])

echo "Configuring packaging test"

AM_INIT_AUTOMAKE([1.11 foreign -Wall -Werror])

# If Automake supports silent rules, enable them.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AH_TOP([/* -*- c -*-
 *
 *	ompitest test suite configuation header file.
 *      See the top-level LICENSE file for license and copyright
 *      information.
 */

#ifndef OMPITEST_CONFIG_H
#define OMPITEST_CONFIG_H
])
AH_BOTTOM([#endif /* OMPITEST_CONFIG_H */])

dnl
dnl Make automake clean emacs ~ files for "make clean"
dnl

CLEANFILES="*~"
AC_SUBST(CLEANFILES)

dnl
dnl Get various programs
dnl Bias towards mpicc/mpic++/mpif77
dnl C compiler
dnl

if test "$CC" != ""; then
    BASE="`basename $CC`"
else
    BASE=
fi
if test "$BASE" = "" -o "$BASE" = "." -o "$BASE" = "cc" -o \
    "$BASE" = "gcc" -o "$BASE" = "xlc" -o "$BASE" = "pgcc" -o \
    "$BASE" = "icc"; then
    AC_CHECK_PROG(HAVE_MPICC, mpicc, yes, no)
    if test "$HAVE_MPICC" = "yes"; then
        CC=mpicc
        export CC
    fi
fi

CFLAGS_save=$CFLAGS
AC_PROG_CC
CFLAGS=$CFLAGS_save

dnl
dnl Fortran compiler
dnl

if test "$FC" != ""; then
    BASE="`basename $FC`"
else
    BASE=
fi
if test "$BASE" = "" -o "$BASE" = "." -o "$BASE" = "f77" -o \
    "$BASE" = "g77" -o "$BASE" = "f90" -o "$BASE" = "g90" -o \
    "$BASE" = "xlf" -o "$BASE" = "ifc" -o "$BASE" = "pgf77"; then
    AC_CHECK_PROG(HAVE_MPIFORT, mpifort, yes, no)
    AS_IF([test "$HAVE_MPIFORT" = "yes"],
          [FC=mpifort],
          [AC_CHECK_PROG([HAVE_MPIF90], mpif90, yes, no)
           AS_IF([test "$HAVE_MPIF90" = "yes"],
                 [FC=mpif90],
                 [AC_CHECK_PROG([HAVE_MPIF77], mpif77, yes, no)
                  AS_IF([test "$HAVE_MPIF77" = "yes"],
                        [FC=mpif77],
                        [AC_MSG_WARN([Cannot find a suitable MPI compiler])
                         AC_MSG_ERROR([Cannot continue])
                        ])
                 ])
         ])
    export FC
fi

FCFLAGS_save=$FCFLAGS
AC_PROG_FC
FCFLAGS=$FCFLAGS_save

dnl
dnl Because these are meant to be used for debugging, after all
dnl

if test -z "$CFLAGS"; then
    CFLAGS="-g"
fi
if test -z "$FCFLAGS"; then
    FCFLAGS="-g";
fi
AC_SUBST(FCFLAGS)
if test -z "$FFLAGS"; then
    FFLAGS="-g";
fi
AC_SUBST(FFLAGS)

dnl
dnl Ensure that we can compile and link a C MPI program
dnl

AC_LANG_PUSH([C])
AC_CHECK_HEADERS(mpi.h)

AC_MSG_CHECKING([if linking MPI program works])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <mpi.h>
]],
                                [[MPI_Comm a = MPI_COMM_WORLD]])],
               [AC_MSG_RESULT([yes])],
               [AC_MSG_RESULT([no])
                AC_MSG_WARN([Simple MPI program fails to link])
                AC_MSG_ERROR([Cannot continue])
               ])
AC_LANG_POP([C])

dnl
dnl Party on
dnl

AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
