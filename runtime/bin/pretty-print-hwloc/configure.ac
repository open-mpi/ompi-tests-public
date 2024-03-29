#
#
#

######################
# Project Information
######################
AC_INIT([HWLOC Tools],
        [0.1])
AC_PREREQ(2.63)


######################
# Utilities
######################
_show_title() {
  cat <<EOF

============================================================================
== ${1}
============================================================================
EOF
}


######################
# Setup Makefile
######################
_show_title "Setup Basic Information"

# Initialize automake
AM_INIT_AUTOMAKE([foreign dist-bzip2 subdir-objects no-define 1.10.1 -Wall -Werror])

# Make becomes a bit more quiet
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Set the language
CFLAGS_save="$CFLAGS"
AC_LANG([C])
CFLAGS="$CFLAGS_save"


######################
# Create a configure header file
######################
AH_TOP([/*
 * This file is automatically generated by configure.  Edits will be lost
 * the next time you run configure!
 */
#ifndef HWLOC_TOOLS_H
#define HWLOC_TOOLS_H

#include <autogen/hwloc_tools_config_top.h>

])
AH_BOTTOM([

#include <autogen/hwloc_tools_config_bottom.h>
#endif /* HWLOC_TOOLS_CONFIG_H */
])

AC_CONFIG_HEADERS([src/include/autogen/config.h])


######################
# Make automake clean emacs ~ files for "make clean"
######################
CLEANFILES="*~ .\#*"
AC_SUBST(CLEANFILES)


######################
# C Compiler
######################
_show_title "Setup C Compiler"

CFLAGS_save="$CFLAGS"
AC_PROG_CC
CFLAGS="$CFLAGS_save"

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LIBS)


######################
# HWLOC Install
######################
_show_title "Setup HWLOC"

#
# --with-hwloc=DIR
# --with-hwloc-libdir=DIR
#
AC_ARG_WITH([hwloc],
            [AC_HELP_STRING([--with-hwloc=DIR],
            [Search for hwloc headers and libraries in DIR ])])

AC_ARG_WITH([hwloc-libdir],
            [AC_HELP_STRING([--with-hwloc-libdir=DIR],
            [Search for hwloc libraries in DIR ])])
# HWLOC is required
AS_IF([test "$with_hwloc" = "no"],
      [AC_MSG_WARN([HWLOC is required. --without-hwloc is not supported.])
       AC_MSG_ERROR([Cannot continue])])
AS_IF([test -z "$with_hwloc" || test "$with_hwloc" == "yes"],
      [AC_MSG_WARN([HWLOC is required. Default search functionality not supported.])
       AC_MSG_ERROR([Cannot continue])])

AC_MSG_CHECKING([HWLOC Location])
AC_MSG_RESULT([$with_hwloc])

CFLAGS="-I$with_hwloc/include $CFLAGS"

# Do we really need '-ludev'?
#LDFLAGS="-ludev $LDFLAGS"

AC_MSG_CHECKING([If static HWLOC library is available])
AS_IF([test -f "$with_hwloc/lib/libhwloc.a" ],
      [AC_MSG_RESULT([yes])
       LIBS="$with_hwloc/lib/libhwloc.a $LIBS"],
      [LDFLAGS="-L$with_hwloc/lib $LDFLAGS"
       LIBS="-lhwloc $LIBS"])

AC_MSG_CHECKING([Final CFLAGS])
AC_MSG_RESULT([$CFLAGS])

AC_MSG_CHECKING([Final LDFLAGS])
AC_MSG_RESULT([$LDFLAGS])

AC_MSG_CHECKING([Final LIBS])
AC_MSG_RESULT([$LIBS])

######################
# Makefile
######################
AC_CONFIG_FILES([Makefile src/Makefile])

######################
# Done
######################
_show_title "All Done"
AC_OUTPUT

