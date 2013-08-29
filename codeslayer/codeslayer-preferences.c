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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-preferences-editor.h>
#include <codeslayer/codeslayer-preferences-theme.h>
#include <codeslayer/codeslayer-preferences-projects.h>
#include <codeslayer/codeslayer-preferences-misc.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-preferences
 * @short_description: The config preferences.
 * @title: CodeSlayerPreferences
 * @include: codeslayer/codeslayer-preferences.h
 *
 * Saves the config preferences using the GKeyFile. The file is saved in the 
 * users home directory under the .codeslayer" folder.
 */

static void codeslayer_preferences_class_init  (CodeSlayerPreferencesClass *klass);
static void codeslayer_preferences_init        (CodeSlayerPreferences      *preferences);
static void codeslayer_preferences_finalize    (CodeSlayerPreferences      *preferences);

#define CODESLAYER_PREFERENCES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PREFERENCES_TYPE, CodeSlayerPreferencesPrivate))

typedef struct _CodeSlayerPreferencesPrivate CodeSlayerPreferencesPrivate;

struct _CodeSlayerPreferencesPrivate
{
  GtkWidget        *window;
  CodeSlayerConfig *config;
};

enum
{
  EDITOR_PREFERENCES_CHANGED,
  NOTEBOOK_PREFERENCES_CHANGED,
  SIDE_PANE_PREFERENCES_CHANGED,
  BOTTOM_PANE_PREFERENCES_CHANGED,
  PROJECTS_PREFERENCES_CHANGED,
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

  /**
   * CodeSlayerPreferences::projects-preferences-changed
   * @codeslayerpreferences: the preference that received the signal
   *
   * The ::projects-preferences-changed signal lets all observers know that 
   * something in the preferences, related to the 
   * #CodeSlayerProjects, changed.
   */
  codeslayer_preferences_signals[PROJECTS_PREFERENCES_CHANGED] =
    g_signal_new ("projects_preferences_changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerPreferencesClass, projects_preferences_changed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_preferences_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPreferencesPrivate));
}

static void
codeslayer_preferences_init (CodeSlayerPreferences *preferences)
{
}

static void
codeslayer_preferences_finalize (CodeSlayerPreferences *preferences)
{
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
 * codeslayer_preferences_get_double:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property key.
 *
 * Returns: the value as a double for the given key.
 */
gdouble
codeslayer_preferences_get_double (CodeSlayerPreferences *preferences,
                                   gchar                 *key)
{
  CodeSlayerPreferencesPrivate *priv;
  const gchar *value;

  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  value = codeslayer_config_get_preference (priv->config, key);
  if (value != NULL)
    return atof (value);

  return -1;
}

/**
 * codeslayer_preferences_set_double:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property key.
 * @value: a property value as a gdouble.
 */
void
codeslayer_preferences_set_double (CodeSlayerPreferences *preferences,
                                   gchar                 *key, 
                                   gdouble                value)
{
  CodeSlayerPreferencesPrivate *priv;
  gchar *val;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  val = g_strdup_printf ("%f", value);
  codeslayer_config_set_preference (priv->config, key, val);
  g_free (val);
}

/**
 * codeslayer_preferences_get_boolean:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property key.
 *
 * Returns: the value as a boolean for the given key.
 */
gboolean
codeslayer_preferences_get_boolean (CodeSlayerPreferences *preferences,
                                    gchar                 *key)
{
  CodeSlayerPreferencesPrivate *priv;
  const gchar *value;

  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  value = codeslayer_config_get_preference (priv->config, key);
  if (value != NULL)
    {
      if (g_strcmp0 (value, "true") == 0)
        return TRUE;
      else
        return FALSE;
    }
  
  return FALSE;
}

/**
 * codeslayer_preferences_set_boolean:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property key.
 * @value: a property value as a gboolean.
 */
void
codeslayer_preferences_set_boolean (CodeSlayerPreferences *preferences,
                                    gchar                 *key, 
                                    gboolean               value)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  
  if (value == TRUE)  
    codeslayer_config_set_preference (priv->config, key, "true");
  else
    codeslayer_config_set_preference (priv->config, key, "false");
}

/**
 * codeslayer_preferences_get_string:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property key.
 *
 * Returns: the value as a string for the given key.
 */
gchar*
codeslayer_preferences_get_string (CodeSlayerPreferences *preferences,
                                   gchar                 *key)
{
  CodeSlayerPreferencesPrivate *priv;
  const gchar *value;

  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  value = codeslayer_config_get_preference (priv->config, key);
  if (value != NULL)
    return g_strdup (value);

  return g_strdup ("");
}

/**
 * codeslayer_preferences_set_string:
 * @preferences: a #CodeSlayerPreferences.
 * @key: a property key.
 * @value: a property value as a gchar pointer.
 */
void
codeslayer_preferences_set_string (CodeSlayerPreferences *preferences,
                                   gchar                 *key, 
                                   gchar                 *value)
{
  CodeSlayerPreferencesPrivate *priv;  
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  codeslayer_config_set_preference (priv->config, key, value);
}

/**
 * codeslayer_preferences_load:
 * @preferences: a #CodeSlayerPreferences.
 * @config: a #CodeSlayerConfig.
 *
 * Load the config preferences.
 */
void
codeslayer_preferences_load (CodeSlayerPreferences *preferences, 
                             CodeSlayerConfig      *config)
{
  CodeSlayerPreferencesPrivate *priv;
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  priv->config = config;
  g_signal_emit_by_name ((gpointer) preferences, "initialize-preferences");
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

  if (codeslayer_config_get_projects_mode (priv->config))
    {
      preferences_projects = codeslayer_preferences_projects_new (preferences);
      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_projects, 
                                gtk_label_new (_("Projects")));    
    }

  preferences_misc = codeslayer_preferences_misc_new (preferences);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_misc, 
                            gtk_label_new (_("Misc")));

  gtk_widget_show_all (notebook);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}
