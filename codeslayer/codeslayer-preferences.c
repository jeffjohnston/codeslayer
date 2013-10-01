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
 * @short_description: The profile preferences.
 * @title: CodeSlayerPreferences
 * @include: codeslayer/codeslayer-preferences.h
 *
 * Saves the profile preferences using the GKeyFile. The file is saved in the 
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
  GtkWidget          *window;
  CodeSlayerProfiles *profiles;
};

G_DEFINE_TYPE (CodeSlayerPreferences, codeslayer_preferences, G_TYPE_OBJECT)

static void 
codeslayer_preferences_class_init (CodeSlayerPreferencesClass *klass)
{
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
codeslayer_preferences_new (GtkWidget          *window, 
                            CodeSlayerProfiles *profiles)
{
  CodeSlayerPreferencesPrivate *priv;
  CodeSlayerPreferences *preferences;

  preferences = g_object_new (codeslayer_preferences_get_type (), NULL);
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);
  priv->window = window;
  priv->profiles = profiles;
  
  return preferences;
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
  CodeSlayerProfile *profile; 
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *preferences_editor;
  GtkWidget *preferences_theme;
  GtkWidget *preferences_projects;
  GtkWidget *preferences_misc;
  GtkWidget *notebook;
  
  priv = CODESLAYER_PREFERENCES_GET_PRIVATE (preferences);

  profile = codeslayer_profiles_get_profile (priv->profiles);

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
  
  preferences_editor = codeslayer_preferences_editor_new (preferences, priv->profiles);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_editor, 
                            gtk_label_new (_("Editor")));

  preferences_theme = codeslayer_preferences_theme_new (preferences, priv->profiles);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_theme, 
                            gtk_label_new (_("Theme")));

  if (codeslayer_profile_get_projects_mode (profile))
    {
      preferences_projects = codeslayer_preferences_projects_new (preferences, priv->profiles);
      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_projects, 
                                gtk_label_new (_("Projects")));    
    }

  preferences_misc = codeslayer_preferences_misc_new (preferences, priv->profiles);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), preferences_misc, 
                            gtk_label_new (_("Misc")));

  gtk_widget_show_all (notebook);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}
