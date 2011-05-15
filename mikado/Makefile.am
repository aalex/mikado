AM_CFLAGS = $(PTHREAD_CFLAGS)
AM_LIBS = $(PTHREAD_LIBS)

# # use lib_LTLIBRARIES to build a shared lib:
# lib_LTLIBRARIES = libmikado-@MIKADO_API_VERSION@.la
# 
# ## Define the source file list for the "libmikado-@MIKADO_API_VERSION@.la"
# ## target.  Note that @MIKADO_API_VERSION@ is not interpreted by Automake and
# ## will therefore be treated as if it were literally part of the target name,
# ## and the variable name derived from that.
# ## The file extension .cc is recognized by Automake, and makes it produce
# ## rules which invoke the C++ compiler to produce a libtool object file (.lo)
# ## from each source file.  Note that it is not necessary to list header files
# ## which are already listed elsewhere in a _HEADERS variable assignment.
# 
# ## PLEASE KEEP THEM IN ALPHABETICAL ORDER
# libmikado_@MIKADO_API_VERSION@_la_SOURCES = \
#     mikado-version.c \
#     mikado.c
# 
# ## Instruct libtool to include ABI version information in the generated shared
# ## library file (.so).  The library ABI version is defined in configure.ac, so
# ## that all version information is kept in one place.
# libmikado_@MIKADO_API_VERSION@_la_LDFLAGS = -export-dynamic -version-info $(MIKADO_LT_VERSION)
# 
# libmikado_@MIKADO_API_VERSION@_la_includedir = $(includedir)/mikado-$(MIKADO_API_VERSION)/mikado
# 
# ## PLEASE KEEP THEM IN ALPHABETICAL ORDER
# libmikado_@MIKADO_API_VERSION@_la_include_HEADERS = \
#     mikado.h \
#     mikado-version.h

EXTRA_DIST = config.h

