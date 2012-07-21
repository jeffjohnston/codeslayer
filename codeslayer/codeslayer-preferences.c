/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-preferences-editor.h>
#include <codeslayer/codeslayer-preferences-theme.h>
#include <codeslayer/codeslayer-preferences-projects.h>
#include <codeslayer/codeslayer-preferences-misc.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-preferences
 * @short_description: The group preferences.
 * @title: CodeSlayerPreferences
 * @include: codeslayer/codeslayer-preferences.h
 *
 * Saves the group preferences using the GKeyFile. The file is saved in the 
 * users home directory under the .codeslayer/groups/"active group" folder.
 */

static void codeslayer_preferences_class_init  (CodeSlayerPreferencesClass *klass);
static void codeslayer_preferences_init        (CodeSlayerPreferences      *preferences);
static void codeslayer_preferences_finalize    (CodeSlayerPreferences      *preferences);

static gboolean verify_conf_exists             (CodeSlayerPreferences      *preferences);
static void set_defaults                       (CodeSlayerPreferences      *preferences);
static gchar *get_conf_path                    (CodeSlayerPreferences      *preferences);

#define CODESLAYER_PREFERENCES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PREFERENCES_TYPE, CodeSlayerPreferencesPrivate))

#define MAIN "main"
#define GROUPS "groups"

typedef struct _CodeSlayerPreferencesPrivate CodeSlayerPreferencesPrivate;

struct _CodeSlayerPreferencesPrivate
{
  GtkWidget       *window;
  GKeyFile        *keyfile;
  CodeSlayerGroup *group;
};

enum
{
  EDITOR_PREFERENCES_CHANGED,
  NOTEBOOK_PREFERENCES_CHANGED,
  SIDE_PANE_PREFERENCES_CHANGED,
  BOTTOM_PANE_PREFERENCES_CHANGED,
  INITIALIZE_PREFERENCES,
  LAST_SIGNAL
};

static guint codeslayer_preferences_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerPreferences, codeslayer_preferences, G_TYPE_OBJECT)

static void 
codeslayer_preferences_class_init (CodeSlayerPreferencesClass *klass)
{
  /**
   * CodeSlayerPreferences::editor-preferences-changed
  * @codeslayerpreferences: the preference that received the signal
   *
   * The ::editor-preferences-changed signal lets all observers know that 
   * something in the preferences, related to the 
   * #CodeSlayerEditor, changed.
   */
  codeslayer_preferences_signals[EDITOR_PREFERENCES_CHANGED] =
    g_signal_new ("editor-preferences-changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerPreferencesClass, editor_preferences_changed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerPreferences::notebook-preferences-changed
   * @codeslayerpreferences: the preference that received the signal
   *
   * The ::notebook-preferences-changed signal lets all observers know that 
   * something in the preferences, related to the 
   * #CodeSlayerNotebook, changed.
   */
  codeslayer_preferences_signals[NOTEBOOK_PREFERENCES_CHANGED] =
    g_signal_new ("notebook-preferences-changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerPreferencesClass, notebook_preferences_changed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerPreferences::side-pane-preferences-changed
   * @codeslayerpreferences: the preference that received the signal
   *
   * The ::side-pane-preferences-changed signal lets all observers know that 
   * something in the preferences, related to the 
   * #CodeSlayerSidePane, changed.
   */
  codeslayer_preferences_signals[SIDE_PANE_PREFERENCES_CHANGED] =
    g_signal_new ("side-pane-preferences-changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerPreferencesClass, side_pane_preferences_changed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerPreferences::bottom-pane-preferences-changed
   * @codeslayerpreferences: the preference that received the signal
   *
   * The ::bottom-pane-preferences-changed signal lets all observers know that 
   * something in the preferences, related to the 
   * #CodeSlayerSidePane, changed.
   */
  codeslayer_preferences_signals[BOTTOM_PANE_PREFERENCES_CHANGED] =
    g_signal_new ("bottom-pane-preferences-changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerPreferencesClass, bottom_pane_preferences_changed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerPreferences::initialize-preferences
   * @codeslayerpreferences: the preference that received the signal
   *
   * The ::initialize-preferences signal lets all observers know that 
   * the preferences need to be applied.
   * #CodeSlayerSidePane, changed.
   */
  codeslayer_preferences_signals[INITIALIZE_PREFERENCES] =
    g_signal_new ("initialize-preferences", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerPreferencesClass, initialize_preferences), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_preferences_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPreferencesPrivate));
}

static void
codeslayer_preferences_init (CodeSlayerPreferences *preferences)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  priv->keyfile = NULL;
}

static void
codeslayer_preferences_finalize (CodeSlayerPreferences *preferences)
{
  CodeSlayerPreferencesPrivate *priv;
  
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  if (priv->keyfile)
    {
      g_key_file_free (priv->keyfile);
      priv->keyfile = NULL;
    }
    
  G_OBJECT_CLASS (codeslayer_preferences_parent_class)->finalize (G_OBJECT (preferences));
}

/**
 * codeslayer_preferences_new:
 * @window: a #GtkWindow.
 *
 * Creates a new #CodeSlayerPreferences.
 *
 * Returns: a new #CodeSlayerPreferences. 
 */
CodeSlayerPreferences*
codeslayer_preferences_new (GtkWidget *window)
{
  CodeSlayerPreferencesPrivate *priv;
  CodeSlayerPreferences *preferences;

  preferences = g_object_new (codeslayer_preferences_get_type (), NULL);
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  priv->window = window;
  
  return preferences;
}

/**
 * codeslayer_preferences_get_integer:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property name.
 *
 * Returns: the value as an integer for the given key.
 */
gint
codeslayer_preferences_get_integer (CodeSlayerPreferences *preferences,
                                    gchar                 *key)
{
  CodeSlayerPreferencesPrivate *priv;
  
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  if (g_key_file_has_key (priv->keyfile, MAIN, key, NULL))
    return g_key_file_get_integer (priv->keyfile, MAIN, key, NULL);
  
  return -1;
}

/**
 * codeslayer_preferences_set_integer:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property name.
 * @value: a property value as a gint.
 */
void
codeslayer_preferences_set_integer (CodeSlayerPreferences *preferences,
                                    gchar                 *key, 
                                    gint                   value)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  g_key_file_set_integer (priv->keyfile, MAIN, key, value);
}

/**
 * codeslayer_preferences_get_double:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property name.
 *
 * Returns: the value as a double for the given key.
 */
gdouble
codeslayer_preferences_get_double (CodeSlayerPreferences *preferences,
                                   gchar                 *key)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  if (g_key_file_has_key (priv->keyfile, MAIN, key, NULL))
    return g_key_file_get_double (priv->keyfile, MAIN, key, NULL);

  return -1;
}

/**
 * codeslayer_preferences_set_double:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property name.
 * @value: a property value as a gdouble.
 */
void
codeslayer_preferences_set_double (CodeSlayerPreferences *preferences,
                                   gchar                 *key, 
                                   gdouble                value)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  g_key_file_set_double (priv->keyfile, MAIN, key, value);
}

/**
 * codeslayer_preferences_get_boolean:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property name.
 *
 * Returns: the value as a boolean for the given key.
 */
gboolean
codeslayer_preferences_get_boolean (CodeSlayerPreferences *preferences,
                                    gchar                 *key)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  if (g_key_file_has_key (priv->keyfile, MAIN, key, NULL))
    return g_key_file_get_boolean (priv->keyfile, MAIN, key, NULL);
  
  return -1;
}

/**
 * codeslayer_preferences_set_boolean:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property name.
 * @value: a property value as a gboolean.
 */
void
codeslayer_preferences_set_boolean (CodeSlayerPreferences *preferences,
                                    gchar                 *key, 
                                    gboolean               value)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  g_key_file_set_boolean (priv->keyfile, MAIN, key, value);
}

/**
 * codeslayer_preferences_get_string:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property name.
 *
 * Returns: the value as a string for the given key.
 */
gchar *
codeslayer_preferences_get_string (CodeSlayerPreferences *preferences,
                                   gchar                 *key)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  
  if (g_key_file_has_key (priv->keyfile, MAIN, key, NULL))
    return g_key_file_get_string (priv->keyfile, MAIN, key, NULL);

  return g_strdup ("");
}

/**
 * codeslayer_preferences_set_string:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property name.
 * @value: a property value as a gchar pointer.
 */
void
codeslayer_preferences_set_string (CodeSlayerPreferences *preferences,
                                   gchar                 *key, 
                                   const gchar           *value)
{
  CodeSlayerPreferencesPrivate *priv;  
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  g_key_file_set_string (priv->keyfile, MAIN, key, value);
}

/**
 * codeslayer_preferences_load:
 * @preferences: a #CodeSlayerPreferences.
 * @group: a #CodeSlayerGroup.
 *
 * Load the group preferences.
 */
void
codeslayer_preferences_load (CodeSlayerPreferences *preferences, 
                             CodeSlayerGroup       *group)
{
  CodeSlayerPreferencesPrivate *priv;
  gboolean conf_exists;
  GKeyFile *keyfile;
  gchar *conf;
  
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  priv->group = group;
  
  if (priv->keyfile)
    {
      g_key_file_free (priv->keyfile);
      priv->keyfile = NULL;
    }
  
  conf_exists = verify_conf_exists (preferences);
  keyfile = g_key_file_new ();

  conf = get_conf_path (preferences);
  g_key_file_load_from_file (keyfile, conf, G_KEY_FILE_NONE, NULL);
  priv->keyfile = keyfile;
  g_free (conf);

  if (!conf_exists)
    set_defaults (preferences);
    
  g_signal_emit_by_name ((gpointer) preferences, "initialize-preferences");
}                             

/**
 * codeslayer_preferences_save:
 * @preferences: a #CodeSlayerPreferences.
 *
 * Save the users preference to disk.
 */
void
codeslayer_preferences_save (CodeSlayerPreferences *preferences)
{
  CodeSlayerPreferencesPrivate *priv;
  gchar *data;
  gchar *conf_path;
  gsize size;
  
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  data = g_key_file_to_data (priv->keyfile, &size, NULL);

  conf_path = get_conf_path (preferences);

  g_file_set_contents (conf_path, data, size, NULL);

  g_free (conf_path);
  g_free (data);
}

static void
set_defaults (CodeSlayerPreferences *preferences)
{
  codeslayer_preferences_set_boolean (preferences,
                                      CODESLAYER_PREFERENCES_EDITOR_DISPLAY_LINE_NUMBERS,
                                      TRUE);
  codeslayer_preferences_set_boolean (preferences,
                                      CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_CURRENT_LINE,
                                      TRUE);
  codeslayer_preferences_set_boolean (preferences,
                                      CODESLAYER_PREFERENCES_EDITOR_DISPLAY_RIGHT_MARGIN,
                                      TRUE);
  codeslayer_preferences_set_boolean (preferences,
                                      CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_MATCHING_BRACKET,
                                      TRUE);
  codeslayer_preferences_set_boolean (preferences,
                                      CODESLAYER_PREFERENCES_EDITOR_INSERT_SPACES_INSTEAD_OF_TABS,
                                      TRUE);
  codeslayer_preferences_set_boolean (preferences,
                                      CODESLAYER_PREFERENCES_EDITOR_ENABLE_AUTOMATIC_INDENTATION,
                                      TRUE);
  codeslayer_preferences_set_double (preferences,
                                     CODESLAYER_PREFERENCES_EDITOR_RIGHT_MARGIN_POSITION,
                                     80);
  codeslayer_preferences_set_double (preferences,
                                     CODESLAYER_PREFERENCES_EDITOR_TAB_WIDTH,
                                     2);
  codeslayer_preferences_set_string (preferences,
                                     CODESLAYER_PREFERENCES_EDITOR_FONT,
                                     "Monospace 9");
  codeslayer_preferences_set_string (preferences,
                                     CODESLAYER_PREFERENCES_EDITOR_THEME,
                                     "classic");
  codeslayer_preferences_set_string (preferences,
                                     CODESLAYER_PREFERENCES_EDITOR_TAB_POSITION,
                                     "top");
  codeslayer_preferences_set_string (preferences,
                                     CODESLAYER_PREFERENCES_SIDE_PANE_TAB_POSITION,
                                     "top");
  codeslayer_preferences_set_string (preferences,
                                     CODESLAYER_PREFERENCES_BOTTOM_PANE_TAB_POSITION,
                                     "left");
  codeslayer_preferences_set_string (preferences,
                                     CODESLAYER_PREFERENCES_PROJECTS_EXCLUDE_DIRS,
                                     ".csv,.git,.svn");
  codeslayer_preferences_set_string (preferences,
                                     CODESLAYER_PREFERENCES_EDITOR_WORD_WRAP_TYPES,
                                     ".txt");
  codeslayer_preferences_set_boolean (preferences,
                                      CODESLAYER_PREFERENCES_PROJECTS_SYNC_WITH_EDITOR,
                                      TRUE);
  codeslayer_preferences_save (preferences);
}

static gboolean
verify_conf_exists (CodeSlayerPreferences *preferences)
{
  gboolean result = TRUE;
  gchar *conf_path;
  GFile *conf_file;

  conf_path = get_conf_path (preferences);
  conf_file = g_file_new_for_path (conf_path);
  if (!g_file_query_exists (conf_file, NULL))
    {
      GFileIOStream *stream;
      stream = g_file_create_readwrite (conf_file, G_FILE_CREATE_NONE, 
                                        NULL, NULL);
      g_io_stream_close (G_IO_STREAM (stream), NULL, NULL);
      g_object_unref (stream);
      result = FALSE;
    }

  g_free (conf_path);
  g_object_unref (conf_file);

  return result;
}

static gchar*
get_conf_path (CodeSlayerPreferences *preferences)
{
  CodeSlayerPreferencesPrivate *priv;
  const gchar *group_name;

  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  group_name = codeslayer_group_get_name (priv->group);

  return g_build_filename (g_get_home_dir (), CODESLAYER_HOME, GROUPS,
                           group_name, CODESLAYER_CONF, NULL);
}

/**
 * codeslayer_preferences_run_dialog:
 * @preferences: a #CodeSlayerPreferences.
 *
 * Show the preferences dialog.
 */
void
codeslayer_preferences_run_dialog (CodeSlayerPreferences *preferences)
{
  CodeSlayerPreferencesPrivate *priv;  
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *preferences_editor;
  GtkWidget *preferences_theme;
  GtkWidget *preferences_projects;
  GtkWidget *preferences_misc;
  GtkWidget *notebook;
  
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  dialog = gtk_dialog_new_with_buttons (_("Preferences"), 
                                        GTK_WINDOW (priv->window),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CLOSE,
                                        GTK_RESPONSE_OK,
                                        NULL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_widget_set_size_request (content_area, 350, -1);
  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 2);
  gtk_box_pack_start (GTK_BOX (content_area), notebook, TRUE, TRUE, 0);  
  
  preferences_editor = codeslayer_preferences_editor_new (preferences);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_editor, 
                            gtk_label_new (_("Editor")));

  preferences_theme = codeslayer_preferences_theme_new (preferences);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_theme, 
                            gtk_label_new (_("Theme")));

  preferences_projects = codeslayer_preferences_projects_new (preferences);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_projects, 
                            gtk_label_new (_("Projects")));

  preferences_misc = codeslayer_preferences_misc_new (preferences);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_misc, 
                            gtk_label_new (_("Misc")));

  gtk_widget_show_all (notebook);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}
