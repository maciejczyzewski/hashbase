# Define the package version numbers and the bug reporting address
m4_define([HB_MAJOR], 0)
m4_define([HB_MINOR], 0)
m4_define([HB_PATCH], 1)
m4_define([HB_BUGS], [maciejanthonyczyzewski@gmail.com])

# Include m4 macros
m4_include([m4/ax_pthread.m4])

# Initialize autoconf
AC_PREREQ([2.64])
AC_INIT([hashbase], [HB_MAJOR.HB_MINOR.HB_PATCH], [HB_BUGS])
AC_CONFIG_SRCDIR([src/hb.c])
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_SYSTEM

# Initialize automake
AM_INIT_AUTOMAKE(1.9 foreign)
AM_SILENT_RULES([yes])

# Define macro variables for the package version numbers
AC_DEFINE(HB_VERSION_MAJOR, HB_MAJOR, [Define the major version number])
AC_DEFINE(HB_VERSION_MINOR, HB_MINOR, [Define the minor version number])
AC_DEFINE(HB_VERSION_PATCH, HB_PATCH, [Define the patch version number])
AC_DEFINE(HB_VERSION_STRING, "HB_MAJOR.HB_MINOR.HB_PATCH", [Define the version string])

# Checks for language
AC_LANG([C])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_CC_C_O

# Define Makefiles
AC_CONFIG_FILES([Makefile src/Makefile])

# Pthread
AX_PTHREAD([AC_DEFINE(HAVE_PTHREAD, 1, [Define if you have POSIX threads libraries and header files.])
            CLIBS="$PTHREAD_LIBS $LIBS"
            CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
            LDFLAGS="$LDFLAGS $PTHREAD_CFLAGS"
            CC="$PTHREAD_CC"], [])
            
# Generate the "configure" script
AC_OUTPUT
