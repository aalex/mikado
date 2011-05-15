# Process this file with autoconf to produce a configure script.

# mikado package version number, (as distinct from shared library version)
# An odd micro number indicates in-progress development from Git
# An even micro number indicates a released version
#
# Making a point release:
# - increase mikado_micro_version to the next even number
# - increase mikado_interface_age to the next even number
#   UNLESS there was an API addition/deprecation, in which case
#   - set mikado_interface_age to 0
# After the release:
# - increase mikado_micro_version to the next odd number
# - increase mikado_interface_version to the next odd number
m4_define([mikado_major_version], [0])
m4_define([mikado_minor_version], [0])
m4_define([mikado_micro_version], [9])

m4_define([mikado_release_status],
          [m4_if(m4_eval(mikado_micro_version % 2), [1], [git],
                 [m4_if(m4_eval(mikado_minor_version % 2), [1], [snapshot],
                                                                 [release])])])

m4_define([mikado_version], [mikado_major_version.mikado_minor_version.mikado_micro_version])

# change this only when breaking the API
m4_define([mikado_api_version], [0.1])

# increase the interface age by 1 for each release; if the API changes,
# set to 0. interface_age and binary_age are used to create the soname
# of the shared object:
#
#  (<minor> * 100 + <micro>) - <interface_age>
#
# this allows using the same soname for different micro-releases in case
# no API was added or deprecated. for instance:
#
#   mikado 1.2.0  -> 100 * 2 + 0  = 200, interface age = 0 -> 200
#   mikado 1.2.2  -> 100 * 2 + 2  = 202, interface age = 2 -> 200
#   mikado 1.2.4  -> 100 * 2 + 4  = 204, interface age = 4 -> 200
#   [ API addition, deprecation ]
#   mikado 1.2.6  -> 100 * 2 + 6  = 206, interface age = 0 -> 206
#   mikado 1.2.8  -> 100 * 2 + 8  = 208, interface age = 2 -> 206
#   mikado 1.2.10 -> 100 * 2 + 10 = 210, interface age = 4 -> 206
#   ...
#
m4_define([mikado_interface_age], [0])
m4_define([mikado_binary_age], [m4_eval(100 * mikado_minor_version + mikado_micro_version)])

AC_PREREQ([2.63])
AC_INIT([mikado],
        [mikado_version],
        [http://github.com/aalex/mikado/issues],
        [mikado],
        [http://github.com/aalex/mikado])
AC_CONFIG_MACRO_DIR([m4])
AC_MSG_NOTICE([Configuring mikado])
AM_INIT_AUTOMAKE([subdir-objects -Wno-portability])
AC_CONFIG_HEADERS([mikado/config.h])
AC_CANONICAL_HOST # fills $host variable

AC_SUBST([MIKADO_MAJOR_VERSION],  [mikado_major_version])
AC_SUBST([MIKADO_MINOR_VERSION],  [mikado_minor_version])
AC_SUBST([MIKADO_MICRO_VERSION],  [mikado_micro_version])
AC_SUBST([MIKADO_VERSION],        [mikado_version])
AC_SUBST([MIKADO_API_VERSION],    [mikado_api_version])
AC_SUBST([MIKADO_API_VERSION_AM], [$MIKADO_MAJOR_VERSION\_0])
AC_SUBST([MIKADO_RELEASE_STATUS], [mikado_release_status])

m4_define([lt_current],  [m4_eval(100 * mikado_minor_version + mikado_micro_version - mikado_interface_age)])
m4_define([lt_revision], [mikado_interface_age])
m4_define([lt_age],      [m4_eval(mikado_binary_age - mikado_interface_age)])
MIKADO_LT_CURRENT=lt_current
MIKADO_LT_REV=lt_revision
MIKADO_LT_AGE=lt_age
MIKADO_LT_VERSION="$MIKADO_LT_CURRENT:$MIKADO_LT_REV:$MIKADO_LT_AGE"
AC_SUBST([MIKADO_LT_VERSION], "$MIKADO_LT_VERSION")
MIKADO_LT_LDFLAGS="-version-info $MIKADO_LT_VERSION"

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for libraries
AC_CHECK_LIB([m],[sqrt])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stddef.h stdlib.h string.h sys/time.h])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([gettimeofday memmove mkdir setenv sqrt strchr strpbrk uname])

AX_PTHREAD

# Clutter-GTK
PKG_CHECK_MODULES([CLUTTERGTK], [clutter-gtk-0.10 >= 0.10], have_clutter_gtk=true, have_clutter_gtk=false)
if test "x${have_clutter_gtk}" = "xfalse" ; then
    AC_MSG_ERROR([missing package: libclutter-gtk-0.10-dev])
fi
AC_SUBST([CLUTTERGTK_LIBS])
AC_SUBST([CLUTTERGTK_CFLAGS])

# libxml2
PKG_CHECK_MODULES([LIBXML], [libxml-2.0], have_libxml=true, have_libxml=false)
if test "x${have_libxml}" = "xfalse" ; then
    AC_MSG_ERROR([missing package: libxml2-dev])
fi
AC_SUBST([LIBXML_LIBS])
AC_SUBST([LIBXML_CFLAGS])

CFLAGS+=" -Wall -Wextra -Wfatal-errors -Werror "

# GNU help2man creates man pages from --help output; in many cases, this
# is sufficient, and obviates the need to maintain man pages separately.
# However, this means invoking executables, which we generally cannot do
# when cross-compiling, so we test to avoid that (the variable
# "cross_compiling" is set by AC_PROG_CC).
if test $cross_compiling = no; then
    AM_MISSING_PROG(HELP2MAN, help2man)
else
    HELP2MAN=:
fi

# Override the template file name of the generated .pc file, so that there
# is no need to rename the template file when the API version changes.
AC_CONFIG_FILES([
        Makefile 
        mikado/mikado-version.h
        mikado/Makefile 
        src/Makefile
        tests/Makefile
        doc/Makefile
        mikado-${MIKADO_API_VERSION}.pc:mikado.pc.in
        ])

AC_OUTPUT

