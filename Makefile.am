# "foreign" means not GNU, and is common for avoiding boring messages
# about files organized differently from what GNU expects

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign

SUBDIRS = \
    mikado \
    src \
    tests \
    doc

EXTRA_DIST = \
    LICENSE \
    NEWS \
    README

# pkg-config file:
DISTCLEANFILES = mikado.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mikado-@MIKADO_API_VERSION@.pc

