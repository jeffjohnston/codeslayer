SUBDIRS = codeslayer docs pixmaps po

ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = codeslayer.pc

desktopdir = $(datadir)/applications
desktop_in_files = codeslayer.desktop
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
  codeslayer.desktop.in \
  codeslayer.pc.in

DISTCLEANFILES = \
	codeslayer.desktop \
	intltool-extract \
	intltool-merge \
	intltool-update
  
DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc
