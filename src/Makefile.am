bin_PROGRAMS = mikado

headers = \
	gui.h

mikado_SOURCES = \
	gui.c \
	main.c \
	$(headers)

mikado_CFLAGS = \
	$(CLUTTERGTK_CFLAGS) \
	$(LIBXML_CFLAGS)

mikado_LDADD = \
	$(CLUTTERGTK_LIBS) \
	$(LIBXML_LIBS)

