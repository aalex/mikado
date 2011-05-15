bin_PROGRAMS = mikado

AM_CFLAGS = \
	$(CLUTTERGTK_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(GEGL_CFLAGS) \
	-I$(top_srcdir)/mikado

LDADD = \
	$(CLUTTERGTK_LIBS) \
	$(LIBXML_LIBS) \
	$(GEGL_LIBS) \
	$(top_builddir)/mikado/libmikado-@MIKADO_API_VERSION@.la

headers = \
	gui.h

mikado_SOURCES = \
	gui.c \
	main.c \
	$(headers)


