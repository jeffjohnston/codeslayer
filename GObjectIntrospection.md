This is the configuration that I used to compile the project using gobject-introspection. In the end I could not figure it out but I wanted to document what I had started. One important thing that I learned to compile the code was to run the commands 'autoreconf' and 'aclocal' if I changed the configuration in a way that broke the build.


configure.ac
```
AC_CONFIG_MACRO_DIR([m4])
GOBJECT_INTROSPECTION_CHECK([1.30.0])
```

Makefile.am

```
DISTCHECK_CONFIGURE_FLAGS = \
  --enable-gtk-doc \
  --enable-introspection
```

codeslayer/Makefile.am

```

#introspection

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = CodeSlayer-1.0.gir
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all --symbol-prefix=codeslayer --verbose
#INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
#introspection_sources = $(pkginclude_HEADERS) $(libcodeslayer_la_SOURCES)
introspection_sources = \
  codeslayer.h \
  codeslayer.c

CodeSlayer-1.0.gir: libcodeslayer.la
CodeSlayer_1_0_gir_INCLUDES = Gtk-3.0 Gdk-3.0 GtkSource-3.0
CodeSlayer_1_0_gir_CFLAGS = $(CODESLAYER_CFLAGS)
CodeSlayer_1_0_gir_PACKAGES = gtk+-3.0 gdk-3.0 gtksourceview-3.0
CodeSlayer_1_0_gir_LIBS = libcodeslayer.la
CodeSlayer_1_0_gir_FILES = $(introspection_sources)
CodeSlayer_1_0_gir_EXPORT_PACKAGES = codeslayer

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelib_DATA)
endif
```