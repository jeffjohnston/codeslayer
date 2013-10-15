/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.remove_group_item
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-menubar-edit.h>
#include <codeslayer/codeslayer-menubar-file.h>
#include <codeslayer/codeslayer-menubar-search.h>
#include <codeslayer/codeslayer-menubar-view.h>
#include <codeslayer/codeslayer-menubar-projects.h>
#include <codeslayer/codeslayer-menubar-tools.h>
#include <codeslayer/codeslayer-menubar-help.h>
#include <codeslayer/codeslayer-marshaller.h>

/**
 * SECTION:codeslayer-menubar
 * @short_description: The application menu.
 * @title: CodeSlayerMenuBar
 * @include: codeslayer/codeslayer-menubar.h
 */

static void codeslayer_menu_bar_class_init  (CodeSlayerMenuBarClass *klass);
static void codeslayer_menu_bar_init        (CodeSlayerMenuBar      *menu_bar);
static void codeslayer_menu_bar_finalize    (CodeSlayerMenuBar      *menu_bar);
                            
#define CODESLAYER_MENU_BAR_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_TYPE, CodeSlayerMenuBarPrivate))

typedef struct _CodeSlayerMenuBarPrivate CodeSlayerMenuBarPrivate;

struct _CodeSlayerMenuBarPrivate
{
  GtkAccelGroup *accel_group;
  GSList        *radio_group;
  GtkWidget     *window;
  GtkWidget     *menu_bar_edit;
  GtkWidget     *menu_bar_file;
  GtkWidget     *menu_bar_search;
  GtkWidget     *menu_bar_view;
  GtkWidget     *menu_bar_projects;
  GtkWidget     *menu_bar_tools;
};

enum
{
  NEW_GROUP,
  RENAME_GROUP,
  REMOVE_GROUP,
  GROUP_CHANGED,
  ADD_PROJECTS,
  NEW_EDITOR,
  OPEN_EDITOR,
  SAVE_EDITOR,
  SAVE_ALL_EDITORS,
  CLOSE_EDITOR,
  QUIT_APPLICATION,
  FIND_GROUP,
  FULLSCREEN_WINDOW,
  SHOW_SIDE_PANE,
  SHOW_BOTTOM_PANE,
  DRAW_SPACES,
  WORD_WRAP,
  REPLACE,
  FIND,
  FIND_NEXT,
  FIND_PREVIOUS,
  GO_TO_LINE,
  FIND_INCREMENTAL,
  SHOW_PREFERENCES,
  SHOW_PROFILES,
  SCAN_EXTERNAL_CHANGES,
  SHOW_PLUGINS,
  UNDO,
  REDO,
  CUT,
  COPY,
  PASTE,
  DELETE,
  SELECT_ALL,
  TO_UPPERCASE,
  TO_LOWERCASE,
  COPY_LINES,  
  SYNC_WITH_EDITOR,  
  SYNC_ENGINE,  
  LAST_SIGNAL
};

static guint codeslayer_menu_bar_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerMenuBar, codeslayer_menu_bar, GTK_TYPE_MENU_BAR)

static void
codeslayer_menu_bar_class_init (CodeSlayerMenuBarClass *klass)
{
  /**
   * CodeSlayerMenuBar::add-project
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::add-project signal is a request to add a new project. 
   */
  codeslayer_menu_bar_signals[ADD_PROJECTS] =
    g_signal_new ("add-projects", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, add_projects),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayerMenuBar::new-editor
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::new-editor signal is a request to save the active editor. 
   */
  codeslayer_menu_bar_signals[NEW_EDITOR] =
    g_signal_new ("new-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, new_editor),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
                  
  /**
   * CodeSlayerMenuBar::open-editor
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::open-editor signal is a request to save the active editor. 
   */
  codeslayer_menu_bar_signals[OPEN_EDITOR] =
    g_signal_new ("open-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, open_editor),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
                  
  /**
   * CodeSlayerMenuBar::save-editor
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::save-editor signal is a request to save the active editor. 
   */
  codeslayer_menu_bar_signals[SAVE_EDITOR] =
    g_signal_new ("save-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, save_editor),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::save-all-editors
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::save-all-editors signal is a request to save all the open editors.
   */
  codeslayer_menu_bar_signals[SAVE_ALL_EDITORS] =
    g_signal_new ("save-all-editors", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, save_all_editors),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::close-editor
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::close-editor signal is a request to close the active editor.
   */
  codeslayer_menu_bar_signals[CLOSE_EDITOR] =
    g_signal_new ("close-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, close_editor),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::quit-application
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::quit-application signal is a request to close the application.
   */
  codeslayer_menu_bar_signals[QUIT_APPLICATION] =
    g_signal_new ("quit-application", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, quit_application),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::find-projects
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::find-projects signal is a request to open up the search dialog.
   */
  codeslayer_menu_bar_signals[FIND_GROUP] =
    g_signal_new ("find-projects", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, find_projects),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
   * CodeSlayerMenuBar::fullscreen-window
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::fullscreen-window signal is a request to open up the search dialog.
   */
  codeslayer_menu_bar_signals[FULLSCREEN_WINDOW] =
    g_signal_new ("fullscreen-window", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, fullscreen_window),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::show-side-pane
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::show-side-pane signal is a request to open up the side pane.
   */
  codeslayer_menu_bar_signals[SHOW_SIDE_PANE] =
    g_signal_new ("show-side-pane", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, show_side_pane),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::show-bottom-pane
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::show-side-pane signal is a request to open up the bottom pane.
   */
  codeslayer_menu_bar_signals[SHOW_BOTTOM_PANE] =
    g_signal_new ("show-bottom-pane", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, show_bottom_pane),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::draw-spaces
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::draw-spaces signal is a request to show the invisible characters in the editors.
   */
  codeslayer_menu_bar_signals[DRAW_SPACES] =
    g_signal_new ("draw-spaces", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, draw_spaces),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::word_wrap
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::word-wrap signal is a request to show the invisible characters in the editors.
   */
  codeslayer_menu_bar_signals[WORD_WRAP] =
    g_signal_new ("word-wrap", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, word_wrap),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::find
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::find signal is a request to open up the notebook search.
   */
  codeslayer_menu_bar_signals[FIND] =
    g_signal_new ("find", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, find),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::replace
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::replace signal is a request to open up the notebook search.
   */
  codeslayer_menu_bar_signals[REPLACE] =
    g_signal_new ("replace", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, replace),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::find-next
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::find-next signal is a request to find the next 
   * value using the notebook search.
   */
  codeslayer_menu_bar_signals[FIND_NEXT] =
    g_signal_new ("find-next", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, find_next),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::find-previous
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::find-previous signal is a request to find the previous 
   * value using the notebook search.
   */
  codeslayer_menu_bar_signals[FIND_PREVIOUS] =
    g_signal_new ("find-previous", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, find_previous), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::go-to-line
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::go-to-line signal is a request to jump to the line number.
   */
  codeslayer_menu_bar_signals[GO_TO_LINE] =
    g_signal_new ("go-to-line", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, go_to_line), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::show-preferences
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::show-preferences signal is a request to open up the preferences dialog.
   */
  codeslayer_menu_bar_signals[SHOW_PREFERENCES] =
    g_signal_new ("show-preferences", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, show_preferences),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::show-profiles
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::show-profiles signal is a request to open up the profiles dialog.
   */
  codeslayer_menu_bar_signals[SHOW_PROFILES] =
    g_signal_new ("show-profiles", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, show_profiles),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::scan-external-changes
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::scan-external-changes signal is a request to scan for changes outside editor.
   */
  codeslayer_menu_bar_signals[SCAN_EXTERNAL_CHANGES] =
    g_signal_new ("scan-external-changes", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, scan_external_changes),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::show-plugins
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::show-plugins signal is a request to open up the plugins dialog.
   */
  codeslayer_menu_bar_signals[SHOW_PLUGINS] =
    g_signal_new ("show-plugins", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, show_plugins),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::undo
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::undo signal is a request to undo the last change.
   */
  codeslayer_menu_bar_signals[UNDO] =
    g_signal_new ("undo", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, undo),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::redo
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::redo signal is a request to redo the last change.
   */
  codeslayer_menu_bar_signals[REDO] =
    g_signal_new ("redo", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, redo),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::cut
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::cut signal is a request to cut the selected text.
   */
  codeslayer_menu_bar_signals[CUT] =
    g_signal_new ("cut", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, cut),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::copy
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::copy signal is a request to copy the selected text.
   */
  codeslayer_menu_bar_signals[COPY] =
    g_signal_new ("copy", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, copy),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::paste
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::paste signal is a request to paste the selected text.
   */
  codeslayer_menu_bar_signals[PASTE] =
    g_signal_new ("paste", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, paste),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::del
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::del signal is a request to delete the selected text.
   */
  codeslayer_menu_bar_signals[DELETE] =
    g_signal_new ("del", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, del),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::select-all
   * @menu: the menu that received the signal
   *
   * Note: for internal use only.
   *
   * The ::select-all signal is a request to select all the text.
   */
  codeslayer_menu_bar_signals[SELECT_ALL] =
    g_signal_new ("select-all", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, select_all),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
                  
  /**
   * CodeSlayerMenuBar::to-uppercase
   * @menu: the menu that received the signal
   *
   * The ::to-uppercase signal enables the (Ctrl + U) keystroke to uppercase 
   * the selected text.
   */
  codeslayer_menu_bar_signals[TO_UPPERCASE] =
    g_signal_new ("to-uppercase", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, to_uppercase),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::to-lowercase
   * @menu: the menu that received the signal
   *
   * The ::to-lowercase signal enables the (Ctrl + L) keystroke to lowercase the 
   * selected text.
   */
  codeslayer_menu_bar_signals[TO_LOWERCASE] =
    g_signal_new ("to-lowercase", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, to_lowercase),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::copy-lines
   * @menu: the menu that received the signal
   *
   * The ::copy-lines signal enables the (Ctrl + Shift + Down) keystroke to copy 
   * the currently selected lines.
   */
  codeslayer_menu_bar_signals[COPY_LINES] =
    g_signal_new ("copy-lines", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, copy_lines),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerMenuBar::sync-with-editor 
   * @menu: the menu that received the signal
   */
  codeslayer_menu_bar_signals[SYNC_WITH_EDITOR] =
    g_signal_new ("sync-with-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, sync_with_editor),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
                  
  /**
   * CodeSlayerMenuBar::sync-engine 
   * @menu: the menu that received the signal
   */
  codeslayer_menu_bar_signals[SYNC_ENGINE] =
    g_signal_new ("sync-engine", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, sync_engine), 
                  NULL, NULL,
                  _codeslayer_marshal_VOID__BOOLEAN_BOOLEAN, G_TYPE_NONE, 2, 
                  G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);                  

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_finalize;

  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarPrivate));
}

static void
codeslayer_menu_bar_init (CodeSlayerMenuBar *menu) {}

static void
codeslayer_menu_bar_finalize (CodeSlayerMenuBar *menu)
{
  G_OBJECT_CLASS (codeslayer_menu_bar_parent_class)->finalize (G_OBJECT (menu));
}

/**
 * codeslayer_menu_bar_new:
 * @window: the main application window.
 * @registry: a #CodeSlayerRegistry.
 *
 * Creates a new #CodeSlayerMenuBar.
 *
 * Returns: a new #CodeSlayerMenuBar. 
 */
GtkWidget*
codeslayer_menu_bar_new (GtkWidget          *window, 
                         CodeSlayerProfiles *profiles)
{
  CodeSlayerMenuBarPrivate *priv;
  GtkWidget *menu;
  
  GtkWidget *menu_bar_edit;
  GtkWidget *menu_bar_file;
  GtkWidget *menu_bar_search;
  GtkWidget *menu_bar_view;
  GtkWidget *menu_bar_projects;
  GtkWidget *menu_bar_tools;
  GtkWidget *menu_help;
  
  GtkAccelGroup *accel_group;
  
  menu = g_object_new (codeslayer_menu_bar_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_GET_PRIVATE (menu);
  priv->window = window;

  accel_group = gtk_accel_group_new ();
  priv->accel_group = accel_group;
  gtk_window_add_accel_group (GTK_WINDOW (window), priv->accel_group);

  menu_bar_edit = codeslayer_menu_bar_edit_new (menu, accel_group);
  priv->menu_bar_edit = menu_bar_edit;

  menu_bar_file = codeslayer_menu_bar_file_new (menu, accel_group);
  priv->menu_bar_file = menu_bar_file;

  menu_bar_search = codeslayer_menu_bar_search_new (menu, accel_group);
  priv->menu_bar_search = menu_bar_search;

  menu_bar_view = codeslayer_menu_bar_view_new (menu, accel_group, profiles);
  priv->menu_bar_view = menu_bar_view;

  menu_bar_projects = codeslayer_menu_bar_projects_new (window, menu, accel_group, profiles);
  priv->menu_bar_projects = menu_bar_projects;

  menu_bar_tools = codeslayer_menu_bar_tools_new (menu, accel_group);
  priv->menu_bar_tools = menu_bar_tools;

  menu_help = codeslayer_menu_bar_help_new (window, menu, accel_group);

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_bar_file);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_bar_edit);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_bar_view);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_bar_search);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_bar_projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_bar_tools);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_help);

  return menu;
}

GtkAccelGroup *
codeslayer_menu_bar_get_accel_group (CodeSlayerMenuBar *menu_bar)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENU_BAR_GET_PRIVATE (menu_bar);
  return priv->accel_group;
}

/**
 * codeslayer_menu_bar_new_editor:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_new_editor (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "new-editor");
}

/**
 * codeslayer_menu_bar_open_editor:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_open_editor (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "open-editor");
}

/**
 * codeslayer_menu_bar_save_editor:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_save_editor (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "save-editor");
}

/**
 * codeslayer_menu_bar_save_all_editors:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_save_all_editors (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "save-all-editors");
}

/**
 * codeslayer_menu_bar_close_editor:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_close_editor (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "close-editor");
}

/**
 * codeslayer_menu_bar_quit_editor:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_quit_application (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "quit-application");
}

/**
 * codeslayer_menu_bar_undo:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_undo (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "undo");
}

/**
 * codeslayer_menu_bar_redo:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_redo (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "redo");
}

/**
 * codeslayer_menu_bar_cut:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_cut (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "cut");
}

/**
 * codeslayer_menu_bar_copy:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_copy (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "copy");
}

/**
 * codeslayer_menu_bar_paste:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_paste (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "paste");
}

/**
 * codeslayer_menu_bar_delete:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_delete (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "del");
}

/**
 * codeslayer_menu_bar_select_all:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_select_all (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "select-all");
}

/**
 * codeslayer_menu_bar_to_uppercas:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_to_uppercase (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "to-uppercase");
}

/**
 * codeslayer_menu_bar_to_lowercase:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_to_lowercase (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "to-lowercase");
}

/**
 * codeslayer_menu_bar_copy_lines:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_copy_lines (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "copy-lines");
}

/**
 * codeslayer_menu_bar_show_preferences:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_show_preferences (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "show-preferences");
}

/**
 * codeslayer_menu_bar_show_profiles:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_show_profiles (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "show-profiles");
}

/**
 * codeslayer_menu_bar_scan_external_changes:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_scan_external_changes (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "scan-external-changes");
}

/**
 * codeslayer_menu_bar_find:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_find (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "find");
}

/**
 * codeslayer_menu_bar_replace:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_replace (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "replace");
}

/**
 * codeslayer_menu_bar_find_next:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_find_next (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "find-next");
}

/**
 * codeslayer_menu_bar_find_previous:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_find_previous (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "find-previous");
}

/**
 * codeslayer_menu_bar_find_projects:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_find_projects (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "find-projects", NULL);
}

/**
 * codeslayer_menu_bar_go_to_line:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menu_bar_go_to_line (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "go-to-line", NULL);
}

/**
 * codeslayer_menu_bar_fullscreen_window:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_fullscreen_window (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "fullscreen-window");
}

/**
 * codeslayer_menu_bar_show_side_pane:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_show_side_pane (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "show-side-pane");
}

/**
 * codeslayer_menu_bar_show_bottom_pane:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_show_bottom_pane (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "show-bottom-pane");
}

/**
 * codeslayer_menu_bar_draw_spaces:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_draw_spaces (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "draw-spaces");
}

/**
 * codeslayer_menu_bar_word_wrap:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_word_wrap (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "word-wrap");
}

/**
 * codeslayer_menu_bar_open_projects:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @file: a #GFile.
 */
void
codeslayer_menu_bar_open_projects (CodeSlayerMenuBar *menu_bar, 
                                   GFile             *file)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "open-projects", file);
}

/**
 * codeslayer_menu_bar_new_projects:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @file: a #GFile.
 */
void
codeslayer_menu_bar_new_projects (CodeSlayerMenuBar *menu_bar, 
                                  gchar             *file_name)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "new-projects", file_name);
}

/**
 * codeslayer_menu_bar_add_projects:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @files: a list of #GFile.
 */
void
codeslayer_menu_bar_add_projects (CodeSlayerMenuBar *menu_bar, 
                                 GSList            *files)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "add-projects", files);
}

/**
 * codeslayer_menu_bar_show_plugins:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_show_plugins (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "show-plugins");
}

/**
 * codeslayer_menu_bar_sync_with_editor:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @sync_with_editor: is TRUE if the projects should be synced with the editor.
 */
void            
codeslayer_menu_bar_sync_with_editor (CodeSlayerMenuBar *menu_bar, 
                                      gboolean           sync_with_editor)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "sync-with-editor", sync_with_editor);
}                                               

/**
 * codeslayer_menu_bar_add_tools_item:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @item: a #GtkMenuItem.
 */
void
codeslayer_menu_bar_add_tools_item (CodeSlayerMenuBar *menu_bar, 
                                    GtkWidget         *item)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENU_BAR_GET_PRIVATE (menu_bar);  
  codeslayer_menu_bar_tools_add_item (CODESLAYER_MENU_BAR_TOOLS (priv->menu_bar_tools),
                                     item);
}                                   
                                                             
/**
 * codeslayer_menu_bar_remove_tools_item:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @item: a #GtkMenuItem.
 */
void
codeslayer_menu_bar_remove_tools_item (CodeSlayerMenuBar *menu_bar, 
                                       GtkWidget         *item)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENU_BAR_GET_PRIVATE (menu_bar);  
  codeslayer_menu_bar_tools_remove_item (CODESLAYER_MENU_BAR_TOOLS (priv->menu_bar_tools),
                                        item);
}

/**
 * codeslayer_menu_bar_check_updates:
 * @menu_bar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menu_bar_check_updates (CodeSlayerMenuBar *menu_bar)
{
  g_signal_emit_by_name ((gpointer) menu_bar, "check-updates");
}
