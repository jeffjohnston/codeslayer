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
#include <codeslayer/codeslayer-menubar-editor.h>
#include <codeslayer/codeslayer-menubar-search.h>
#include <codeslayer/codeslayer-menubar-view.h>
#include <codeslayer/codeslayer-menubar-groups.h>
#include <codeslayer/codeslayer-menubar-projects.h>
#include <codeslayer/codeslayer-menubar-tools.h>
#include <codeslayer/codeslayer-menubar-help.h>

/**
 * SECTION:codeslayer-menubar
 * @short_description: The application menu.
 * @title: CodeSlayerMenuBar
 * @include: codeslayer/codeslayer-menubar.h
 */

static void codeslayer_menubar_class_init  (CodeSlayerMenuBarClass *klass);
static void codeslayer_menubar_init        (CodeSlayerMenuBar      *menubar);
static void codeslayer_menubar_finalize    (CodeSlayerMenuBar      *menubar);
                            
#define CODESLAYER_MENUBAR_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENUBAR_TYPE, CodeSlayerMenuBarPrivate))

typedef struct _CodeSlayerMenuBarPrivate CodeSlayerMenuBarPrivate;

struct _CodeSlayerMenuBarPrivate
{
  CodeSlayerGroups *groups;
  GtkAccelGroup    *accel_group;
  GSList           *radio_group;
  GtkWidget        *window;
  GtkWidget        *menubar_editor;
  GtkWidget        *menubar_search;
  GtkWidget        *menubar_view;
  GtkWidget        *menubar_groups;
  GtkWidget        *menubar_projects;
  GtkWidget        *menubar_tools;
};

enum
{
  NEW_GROUP,
  RENAME_GROUP,
  REMOVE_GROUP,
  GROUP_CHANGED,
  ADD_PROJECT,
  SAVE_EDITOR,
  SAVE_ALL_EDITORS,
  CLOSE_EDITOR,
  QUIT_APPLICATION,
  FIND_GROUP,
  FULLSCREEN_WINDOW,
  SHOW_SIDE_PANE,
  SHOW_BOTTOM_PANE,
  DRAW_SPACES,
  REPLACE,
  FIND,
  FIND_NEXT,
  FIND_PREVIOUS,
  FIND_INCREMENTAL,
  SHOW_PREFERENCES,
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
  SYNC_PROJECTS_WITH_EDITOR,  
  LAST_SIGNAL
};

static guint codeslayer_menubar_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerMenuBar, codeslayer_menubar, GTK_TYPE_MENU_BAR)

static void
codeslayer_menubar_class_init (CodeSlayerMenuBarClass *klass)
{

  /**
	 * CodeSlayerMenuBar::group-changed
	 * @menu: the menu that received the signal
	 *
	 * Note: for internal use only.
	 *
	 * The ::group-changed signal is a request for the active group to be changed.
	 */
  codeslayer_menubar_signals[GROUP_CHANGED] =
    g_signal_new ("group-changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, group_changed),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
	 * CodeSlayerMenuBar::new-group
	 * @menu: the menu that received the signal
	 *
	 * Note: for internal use only.
	 *
	 * The ::new-group signal is a request to create a new group. 
	 */
  codeslayer_menubar_signals[NEW_GROUP] =
    g_signal_new ("new-group", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, new_group),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
	 * CodeSlayerMenuBar::rename-group
	 * @menu: the menu that received the signal
	 *
	 * Note: for internal use only.
	 *
	 * The ::rename-group signal is a request to rename the active group. 
	 */
  codeslayer_menubar_signals[RENAME_GROUP] =
    g_signal_new ("rename-group", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, rename_group),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  /**
	 * CodeSlayerMenuBar::remove-group
	 * @menu: the menu that received the signal
	 *
	 * Note: for internal use only.
	 *
	 * The ::remove-group signal is a request to remove the active group. 
	 */
  codeslayer_menubar_signals[REMOVE_GROUP] =
    g_signal_new ("remove-group", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, remove_group),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
	 * CodeSlayerMenuBar::add-project
	 * @menu: the menu that received the signal
	 *
	 * Note: for internal use only.
	 *
	 * The ::add-project signal is a request to add a new project to the active group. 
	 */
  codeslayer_menubar_signals[ADD_PROJECT] =
    g_signal_new ("add-project", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, add_project),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
	 * CodeSlayerMenuBar::save-editor
	 * @menu: the menu that received the signal
	 *
	 * Note: for internal use only.
	 *
	 * The ::save-editor signal is a request to save the active editor. 
	 */
  codeslayer_menubar_signals[SAVE_EDITOR] =
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
  codeslayer_menubar_signals[SAVE_ALL_EDITORS] =
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
  codeslayer_menubar_signals[CLOSE_EDITOR] =
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
  codeslayer_menubar_signals[QUIT_APPLICATION] =
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
  codeslayer_menubar_signals[FIND_GROUP] =
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
  codeslayer_menubar_signals[FULLSCREEN_WINDOW] =
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
  codeslayer_menubar_signals[SHOW_SIDE_PANE] =
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
  codeslayer_menubar_signals[SHOW_BOTTOM_PANE] =
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
	 * The ::draw-spaces signal is a request to open up the search dialog.
	 */
  codeslayer_menubar_signals[DRAW_SPACES] =
    g_signal_new ("draw-spaces", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, draw_spaces),
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
  codeslayer_menubar_signals[FIND] =
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
  codeslayer_menubar_signals[REPLACE] =
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
  codeslayer_menubar_signals[FIND_NEXT] =
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
  codeslayer_menubar_signals[FIND_PREVIOUS] =
    g_signal_new ("find-previous", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, find_previous), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
	 * CodeSlayerMenuBar::find-incremental
	 * @menu: the menu that received the signal
	 *
	 * Note: for internal use only.
	 *
	 * The ::find-incremental signal is a request to find the value using 
	 * the notebook search with the incremental turned on.
	 */
  codeslayer_menubar_signals[FIND_INCREMENTAL] =
    g_signal_new ("find-incremental", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, find_incremental), 
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
  codeslayer_menubar_signals[SHOW_PREFERENCES] =
    g_signal_new ("show-preferences", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, change_preferences),
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
  codeslayer_menubar_signals[SHOW_PLUGINS] =
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
  codeslayer_menubar_signals[UNDO] =
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
  codeslayer_menubar_signals[REDO] =
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
  codeslayer_menubar_signals[CUT] =
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
  codeslayer_menubar_signals[COPY] =
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
  codeslayer_menubar_signals[PASTE] =
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
  codeslayer_menubar_signals[DELETE] =
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
  codeslayer_menubar_signals[SELECT_ALL] =
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
  codeslayer_menubar_signals[TO_UPPERCASE] =
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
  codeslayer_menubar_signals[TO_LOWERCASE] =
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
  codeslayer_menubar_signals[COPY_LINES] =
    g_signal_new ("copy-lines", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, copy_lines),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
	 * CodeSlayerMenuBar::sync-projects-with-editor 
	 * @menu: the menu that received the signal
	 */
  codeslayer_menubar_signals[SYNC_PROJECTS_WITH_EDITOR] =
    g_signal_new ("sync-projects-with-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuBarClass, sync_projects_with_editor),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menubar_finalize;

  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarPrivate));
}

static void
codeslayer_menubar_init (CodeSlayerMenuBar *menu) {}

static void
codeslayer_menubar_finalize (CodeSlayerMenuBar *menu)
{
  G_OBJECT_CLASS (codeslayer_menubar_parent_class)->finalize (G_OBJECT (menu));
}

/**
 * codeslayer_menubar_new:
 * @window: the main application window.
 * @groups: a #CodeSlayerGroups.
 * @preferences: a #CodeSlayerPreferences.
 *
 * Creates a new #CodeSlayerMenuBar.
 *
 * Returns: a new #CodeSlayerMenuBar. 
 */
GtkWidget*
codeslayer_menubar_new (GtkWidget             *window,
                        CodeSlayerGroups      *groups, 
                        CodeSlayerPreferences *preferences)
{
  CodeSlayerMenuBarPrivate *priv;
  GtkWidget *menu;
  
  GtkWidget *menubar_editor;
  GtkWidget *menubar_search;
  GtkWidget *menubar_view;
  GtkWidget *menubar_groups;
  GtkWidget *menubar_projects;
  GtkWidget *menubar_tools;
  GtkWidget *menu_help;
  
  GtkAccelGroup *accel_group;
  
  menu = g_object_new (codeslayer_menubar_get_type (), NULL);
  priv = CODESLAYER_MENUBAR_GET_PRIVATE (menu);
  priv->window = window;
  priv->groups = groups;

  accel_group = gtk_accel_group_new ();
  priv->accel_group = accel_group;
  gtk_window_add_accel_group (GTK_WINDOW (window), priv->accel_group);

  menubar_editor = codeslayer_menubar_editor_new (menu, accel_group);
  priv->menubar_editor = menubar_editor;

  menubar_search = codeslayer_menubar_search_new (menu, accel_group);
  priv->menubar_search = menubar_search;

  menubar_view = codeslayer_menubar_view_new (menu, accel_group);
  priv->menubar_view = menubar_view;

  menubar_groups = codeslayer_menubar_groups_new (menu, accel_group, groups);
  priv->menubar_groups = menubar_groups;

  menubar_projects = codeslayer_menubar_projects_new (menu, accel_group, preferences);
  priv->menubar_projects = menubar_projects;

  menubar_tools = codeslayer_menubar_tools_new (menu, accel_group);
  priv->menubar_tools = menubar_tools;

  menu_help = codeslayer_menubar_help_new (menu, accel_group);

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menubar_editor);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menubar_search);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menubar_view);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menubar_groups);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menubar_projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menubar_tools);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_help);

  return menu;
}

GtkAccelGroup *
codeslayer_menubar_get_accel_group (CodeSlayerMenuBar *menubar)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENUBAR_GET_PRIVATE (menubar);
  return priv->accel_group;
}

/**
 * codeslayer_menubar_sync_with_notebook:
 * @menubar: a #CodeSlayerMenuBar.
 * @notebook: a #CodeSlayerNotebook.
 * 
 * Update the sensitivity of editor related menu items based on the current 
 * state of the notebook #CodeSlayerEditor widgets.
 */
void
codeslayer_menubar_sync_with_notebook (CodeSlayerMenuBar *menubar,
                                       GtkWidget         *notebook)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENUBAR_GET_PRIVATE (menubar);
  codeslayer_menubar_editor_sync_with_notebook (CODESLAYER_MENUBAR_EDITOR (priv->menubar_editor),
                                                notebook);                                               
  codeslayer_menubar_search_sync_with_notebook (CODESLAYER_MENUBAR_SEARCH (priv->menubar_search),
                                                notebook);  
}

/**
 * codeslayer_menubar_sync_with_panes:
 * @menubar: a #CodeSlayerMenuBar.
 * @show_side_pane: if TRUE then the side pane is shown
 * @show_bottom_pane: if TRUE then the bottom pane is shown
 * 
 * Update the sensitivity of view related menu items based on the current 
 * state of the bottom and side pane.
 */
void            
codeslayer_menubar_sync_with_panes (CodeSlayerMenuBar *menubar, 
                                    gboolean           show_side_pane, 
                                    gboolean           show_bottom_pane)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENUBAR_GET_PRIVATE (menubar);
  codeslayer_menubar_view_sync_with_panes (CODESLAYER_MENUBAR_VIEW (priv->menubar_view),
                                           show_side_pane, show_bottom_pane);  
}                                         

/**
 * codeslayer_menubar_save_editor:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_save_editor (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "save-editor");
}

/**
 * codeslayer_menubar_save_all_editors:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_save_all_editors (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "save-all-editors");
}

/**
 * codeslayer_menubar_close_editor:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_close_editor (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "close-editor");
}

/**
 * codeslayer_menubar_quit_editor:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_quit_application (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "quit-application");
}

/**
 * codeslayer_menubar_undo:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_undo (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "undo");
}

/**
 * codeslayer_menubar_redo:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_redo (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "redo");
}

/**
 * codeslayer_menubar_cut:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_cut (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "cut");
}

/**
 * codeslayer_menubar_copy:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_copy (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "copy");
}

/**
 * codeslayer_menubar_paste:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_paste (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "paste");
}

/**
 * codeslayer_menubar_delete:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_delete (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "del");
}

/**
 * codeslayer_menubar_select_all:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_select_all (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "select-all");
}

/**
 * codeslayer_menubar_to_uppercas:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_to_uppercase (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "to-uppercase");
}

/**
 * codeslayer_menubar_to_lowercase:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_to_lowercase (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "to-lowercase");
}

/**
 * codeslayer_menubar_copy_lines:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_copy_lines (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "copy-lines");
}

/**
 * codeslayer_menubar_show_preferences:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_show_preferences (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "show-preferences");
}

/**
 * codeslayer_menubar_find:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_find (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "find");
}

/**
 * codeslayer_menubar_replace:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_replace (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "replace");
}

/**
 * codeslayer_menubar_find_next:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_find_next (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "find-next");
}

/**
 * codeslayer_menubar_find_previous:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_find_previous (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "find-previous");
}

/**
 * codeslayer_menubar_find_incremental:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_find_incremental (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "find-incremental");
}

/**
 * codeslayer_menubar_find_projects:
 * @menubar: a #CodeSlayerMenuBar.
 */
void            
codeslayer_menubar_find_projects (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "find-projects", NULL);
}

/**
 * codeslayer_menubar_fullscreen_window:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_fullscreen_window (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "fullscreen-window");
}

/**
 * codeslayer_menubar_show_side_pane:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_show_side_pane (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "show-side-pane");
}

/**
 * codeslayer_menubar_show_bottom_pane:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_show_bottom_pane (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "show-bottom-pane");
}

/**
 * codeslayer_menubar_draw_spaces:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_draw_spaces (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "draw-spaces");
}

void            
/**
 * codeslayer_menubar_refresh_groups:
 * @menubar: a #CodeSlayerMenuBar.
 * @groups: a #CodeSlayerGroups.
 */
codeslayer_menubar_refresh_groups (CodeSlayerMenuBar       *menubar,
                                   CodeSlayerGroups *groups)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENUBAR_GET_PRIVATE (menubar);
  codeslayer_menubar_groups_refresh_groups (CODESLAYER_MENUBAR_GROUPS (priv->menubar_groups), 
                                            groups);
}                                                          

/**
 * codeslayer_menubar_group_changed:
 * @menubar: a #CodeSlayerMenuBar.
 * @group_name: the group name.
 */
void
codeslayer_menubar_group_changed (CodeSlayerMenuBar *menubar, 
                                  const gchar       *group_name)
{
  g_signal_emit_by_name ((gpointer) menubar, "group-changed", group_name);
}                               

/**
 * codeslayer_menubar_new_group:
 * @menubar: a #CodeSlayerMenuBar.
 * @group_name: the group name.
 */
void
codeslayer_menubar_new_group (CodeSlayerMenuBar *menubar, 
                              const gchar       *group_name)
{
  g_signal_emit_by_name ((gpointer) menubar, "new-group", group_name);
}                               

/**
 * codeslayer_menubar_rename_group:
 * @menubar: a #CodeSlayerMenuBar.
 * @group_name: the group name.
 */
void
codeslayer_menubar_rename_group (CodeSlayerMenuBar *menubar, 
                                 const gchar       *group_name)
{
  g_signal_emit_by_name ((gpointer) menubar, "rename-group", group_name);
}                               

/**
 * codeslayer_menubar_remove_group:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_remove_group (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "remove-group");
}                               

/**
 * codeslayer_menubar_add_project:
 * @menubar: a #CodeSlayerMenuBar.
 * @file: a #GFile.
 */
void
codeslayer_menubar_add_project (CodeSlayerMenuBar *menubar, 
                                GFile             *file)
{
  g_signal_emit_by_name ((gpointer) menubar, "add-project", file);
}

/**
 * codeslayer_menubar_show_plugins:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_show_plugins (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "show-plugins");
}

/**
 * codeslayer_menubar_sync_projects_with_editor:
 * @menubar: a #CodeSlayerMenuBar.
 * @sync_projects_with_editor: is TRUE if the projects should be synced with the editor.
 */
void            
codeslayer_menubar_sync_projects_with_editor (CodeSlayerMenuBar *menubar, 
                                              gboolean           sync_projects_with_editor)
{
  g_signal_emit_by_name ((gpointer) menubar, "sync-projects-with-editor", sync_projects_with_editor);
}                                               

/**
 * codeslayer_menubar_add_tools_item:
 * @menubar: a #CodeSlayerMenuBar.
 * @item: a #GtkMenuItem.
 */
void
codeslayer_menubar_add_tools_item (CodeSlayerMenuBar *menubar, 
                                   GtkWidget         *item)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENUBAR_GET_PRIVATE (menubar);  
  codeslayer_menubar_tools_add_item (CODESLAYER_MENUBAR_TOOLS (priv->menubar_tools),
                                     item);
}                                   
                                                             
/**
 * codeslayer_menubar_remove_tools_item:
 * @menubar: a #CodeSlayerMenuBar.
 * @item: a #GtkMenuItem.
 */
void
codeslayer_menubar_remove_tools_item (CodeSlayerMenuBar *menubar, 
                                      GtkWidget         *item)
{
  CodeSlayerMenuBarPrivate *priv;
  priv = CODESLAYER_MENUBAR_GET_PRIVATE (menubar);  
  codeslayer_menubar_tools_remove_item (CODESLAYER_MENUBAR_TOOLS (priv->menubar_tools),
                                        item);
}

/**
 * codeslayer_menubar_check_updates:
 * @menubar: a #CodeSlayerMenuBar.
 */
void
codeslayer_menubar_check_updates (CodeSlayerMenuBar *menubar)
{
  g_signal_emit_by_name ((gpointer) menubar, "check-updates");
}
