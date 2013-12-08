AUTOMAKE_OPTIONS=1.6

DOC_MODULE=codeslayer

DOC_MAIN_SGML_FILE=codeslayer-docs.sgml

SCAN_OPTIONS=--deprecated-guards="GTK_ENABLE_BROKEN|GTK_DISABLE_DEPRECATED"

DOC_SOURCE_DIR=$(top_srcdir)/codeslayer

HFILE_GLOB = $(top_srcdir)/codeslayer/*.h
CFILE_GLOB = $(top_srcdir)/codeslayer/*.c

IGNORE_HFILES = \
    encoding.h \
    codeslayer-utils.h \
    codeslayer-preferences-utils.h \
    codeslayer-tearoff.h \
    codeslayer-marshaller.h \
    codeslayer-xml.h    

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/codeslayer \
	-I$(top_builddir) \
	-I$(top_builddir)/codeslayer \
	$(CODESLAYER_CFLAGS)

GTKDOC_LIBS = \
    $(top_builddir)/codeslayer/.libs/libcodeslayer.so \
    $(CODESLAYER_LIBS)
    
MKDB_OPTIONS = --sgml-mode --output-format=xml --name-space=codeslayer

include $(top_srcdir)/gtk-doc.make

content_files= version.xml
EXTRA_DIST += version.xml.in