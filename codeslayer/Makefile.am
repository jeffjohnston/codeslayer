lib_LTLIBRARIES = libcodeslayer.la

pkginclude_HEADERS = \
    codeslayer-application.h \
    codeslayer-abstract-engine.h \
    codeslayer-engine.h \
    codeslayer-process.h \
    codeslayer-processbar.h \
    codeslayer-processes.h \
    codeslayer-completion.h \
    codeslayer-completion-provider.h \
    codeslayer-completion-proposal.h \
    codeslayer-abstract-pane.h \
    codeslayer-side-pane.h \
    codeslayer-bottom-pane.h \
    codeslayer-listview.h \
    codeslayer-menubar.h \
    codeslayer-menubar-edit.h \
    codeslayer-menubar-file.h \
    codeslayer-menubar-search.h \
    codeslayer-menubar-view.h \
    codeslayer-menubar-projects.h \
    codeslayer-menubar-tools.h \
    codeslayer-menubar-help.h \
    codeslayer-project.h \
    codeslayer-profile.h \
    codeslayer-profiles.h \
    codeslayer-profiles-manager.h \
    codeslayer-document.h \
    codeslayer-registry.h \
    codeslayer-preferences.h \
    codeslayer-preferences-misc.h \
    codeslayer-preferences-projects.h \
    codeslayer-preferences-editor.h \
    codeslayer-preferences-theme.h \
    codeslayer-preferences-utils.h \
    codeslayer-preferences-listview.h \
    codeslayer-projects.h \
    codeslayer-projects-search.h \
    codeslayer-projects-selection.h \
    codeslayer-projects-engine.h \
    codeslayer-project-properties.h \
    codeslayer-menuitem.h \
    codeslayer-notebook-pane.h \
    codeslayer-notebook-search.h \
    codeslayer-notebook.h \
    codeslayer-notebook-tab.h \
    codeslayer-notebook-page.h \
    codeslayer-editor-linker.h \
    codeslayer-editor.h \
    codeslayer-utils.h \
    codeslayer-xml.h \
    codeslayer-marshaller.h \
    codeslayer-plugin.h \
    codeslayer-plugins-selector.h \
    codeslayer-plugins.h \
    codeslayer-tearoff.h \
    codeslayer.h
    
libcodeslayer_la_SOURCES = \
    codeslayer-application.c \
    codeslayer-abstract-engine.c \
    codeslayer-engine.c \
    codeslayer-process.c \
    codeslayer-processbar.c \
    codeslayer-processes.c \
    codeslayer-completion.c \
    codeslayer-completion-provider.c \
    codeslayer-completion-proposal.c \
    codeslayer-abstract-pane.c \
    codeslayer-side-pane.c \
    codeslayer-bottom-pane.c \
    codeslayer-listview.c \
    codeslayer-menubar.c \
    codeslayer-menubar-edit.c \
    codeslayer-menubar-file.c \
    codeslayer-menubar-search.c \
    codeslayer-menubar-view.c \
    codeslayer-menubar-projects.c \
    codeslayer-menubar-tools.c \
    codeslayer-menubar-help.c \
    codeslayer-project.c \
    codeslayer-profile.c \
    codeslayer-profiles.c \
    codeslayer-profiles-manager.c \
    codeslayer-document.c \
    codeslayer-registry.c \
    codeslayer-preferences.c \
    codeslayer-preferences-editor.c \
    codeslayer-preferences-misc.c \
    codeslayer-preferences-projects.c \
    codeslayer-preferences-theme.c \
    codeslayer-preferences-utils.c \
    codeslayer-preferences-listview.c \
    codeslayer-projects.c \
    codeslayer-projects-search.c \
    codeslayer-projects-selection.c \
    codeslayer-projects-engine.c \
    codeslayer-project-properties.c \
    codeslayer-menuitem.c \
    codeslayer-notebook-pane.c \
    codeslayer-notebook-search.c \
    codeslayer-notebook.c \
    codeslayer-notebook-tab.c \
    codeslayer-notebook-page.c \
    codeslayer-editor-linker.c \
    codeslayer-editor.c \
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
