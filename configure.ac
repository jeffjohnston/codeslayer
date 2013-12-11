AC_PREREQ(2.59)
AC_INIT([CodeSlayer], [4.0.0], [jeff.johnston.mn@gmail.com], [codeslayer], [http://codeslayer.org])
AM_INIT_AUTOMAKE

AC_ARG_ENABLE(codeslayer-dev,
    [  --enable-codeslayer-dev  work out of the development directory],
    [AC_DEFINE(CODESLAYER_HOME, ".codeslayer-dev")], [AC_DEFINE(CODESLAYER_HOME, ".codeslayer")])

AC_DEFINE(CODESLAYER_DB, "codeslayer.db")
AC_DEFINE(CODESLAYER_CONF, "codeslayer.conf")

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])

#Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([dlopen])

# Check for programs
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Localization
ALL_LINGUAS="es"
GETTEXT_PACKAGE=codeslayer
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AM_GLIB_GNU_GETTEXT
AC_PROG_INTLTOOL

# Dependencies
GTK_REQUIRED_VERSION=3.6.0

AC_SUBST(GTK_REQUIRED_VERSION)

PKG_CHECK_MODULES(CODESLAYER, [
    glib-2.0 >= 2.28.0
    gtk+-3.0 >= $GTK_REQUIRED_VERSION
    gtksourceview-3.0 >= 3.0.0
])

AC_CONFIG_FILES([
    codeslayer.pc
    Makefile
    codeslayer/Makefile
    pixmaps/Makefile
    pixmaps/16x16/Makefile
    pixmaps/48x48/Makefile    
    po/Makefile.in
    docs/Makefile
    docs/reference/Makefile
    docs/reference/version.xml
])

AC_OUTPUT

echo "

Configuration:

	Source code location:   ${srcdir}
	Compiler:               ${CC}
	Prefix:			${prefix}
"
