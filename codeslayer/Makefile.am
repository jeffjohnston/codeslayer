lib_LTLIBRARIES = libcodeslayer.la

pkginclude_HEADERS = \
    codeslayer-engine.h \
    codeslayer-process.h \
    codeslayer-processes.h \
    codeslayer-statusbar.h \
    codeslayer-completion.h \
    codeslayer-completion-provider.h \
    codeslayer-completion-proposal.h \
    codeslayer-abstract-pane.h \
    codeslayer-side-pane.h \
    codeslayer-bottom-pane.h \
    codeslayer-listview.h \
    codeslayer-menubar.h \
    codeslayer-menubar-editor.h \
    codeslayer-menubar-search.h \
    codeslayer-menubar-view.h \
    codeslayer-menubar-groups.h \
    codeslayer-menubar-projects.h \
    codeslayer-menubar-tools.h \
    codeslayer-menubar-help.h \
    codeslayer-project.h \
    codeslayer-group.h \
    codeslayer-group-properties.h \
    codeslayer-groups.h \
    codeslayer-document.h \
    codeslayer-repository.h \
    codeslayer-settings.h \
    codeslayer-preferences.h \
    codeslayer-preferences-misc.h \
    codeslayer-preferences-projects.h \
    codeslayer-preferences-editor.h \
    codeslayer-preferences-theme.h \
    codeslayer-preferences-utils.h \
    codeslayer-preferences-listview.h \
    codeslayer-projects-selection.h \
    codeslayer-projects.h \
    codeslayer-project-properties.h \
    codeslayer-menuitem.h \
    codeslayer-notebook-pane.h \
    codeslayer-notebook-search.h \
    codeslayer-notebook.h \
    codeslayer-notebook-tab.h \
    codeslayer-notebook-page.h \
    codeslayer-editor.h \
    codeslayer-search.h \
    codeslayer-utils.h \
    codeslayer-xml.h \
    codeslayer-marshaller.h \
    codeslayer-plugin.h \
    codeslayer-plugins-selector.h \
    codeslayer-plugins.h \
    codeslayer-tearoff.h \
    codeslayer.h
    
libcodeslayer_la_SOURCES = \
    codeslayer-engine.c \
    codeslayer-process.c \
    codeslayer-processes.c \
    codeslayer-statusbar.c \
    codeslayer-completion.c \
    codeslayer-completion-provider.c \
    codeslayer-completion-proposal.c \
    codeslayer-abstract-pane.c \
    codeslayer-side-pane.c \
    codeslayer-bottom-pane.c \
    codeslayer-listview.c \
    codeslayer-menubar.c \
    codeslayer-menubar-editor.c \
    codeslayer-menubar-search.c \
    codeslayer-menubar-view.c \
    codeslayer-menubar-groups.c \
    codeslayer-menubar-projects.c \
    codeslayer-menubar-tools.c \
    codeslayer-menubar-help.c \
    codeslayer-project.c \
    codeslayer-group.c \
    codeslayer-group-properties.c \
    codeslayer-groups.c \
    codeslayer-document.c \
    codeslayer-repository.c \
    codeslayer-settings.c \
    codeslayer-preferences.c \
    codeslayer-preferences-editor.c \
    codeslayer-preferences-misc.c \
    codeslayer-preferences-projects.c \
    codeslayer-preferences-theme.c \
    codeslayer-preferences-utils.c \
    codeslayer-preferences-listview.c \
    codeslayer-projects.c \
    codeslayer-projects-selection.c \
    codeslayer-project-properties.c \
    codeslayer-menuitem.c \
    codeslayer-notebook-pane.c \
    codeslayer-notebook-search.c \
    codeslayer-notebook.c \
    codeslayer-notebook-tab.c \
    codeslayer-notebook-page.c \
    codeslayer-editor.c \
    codeslayer-search.c \
    codeslayer-utils.c \
    codeslayer-xml.c \
    codeslayer-marshaller.c \
    codeslayer-plugin.c \
    codeslayer-plugins-selector.c \
    codeslayer-plugins.c \
    codeslayer-tearoff.c \
    encoding.c \
    encoding.h \
    codeslayer.c

libcodeslayer_la_CPPFLAGS = $(CODESLAYER_CFLAGS) -I$(top_srcdir) -I$(srcdir)

bin_PROGRAMS = codeslayer
codeslayer_SOURCES = codeslayer-main.c
codeslayer_CPPFLAGS = $(CODESLAYER_CFLAGS) -I$(top_srcdir) -I$(srcdir)
codeslayer_LDADD = libcodeslayer.la $(CODESLAYER_LIBS)

datadir = @datadir@
datarootdir= @datarootdir@
localedir = @localedir@
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
