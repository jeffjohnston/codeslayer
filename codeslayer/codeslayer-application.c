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

#include <codeslayer/codeslayer-application.h>
#include <codeslayer/codeslayer-window.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-profiles.h>

/**
 * SECTION:codeslayer-application
 * @short_description: Represents the main application.
 * @title: CodeSlayerApplication
 * @include: codeslayer/codeslayer-application.h
 */

static void codeslayer_application_class_init  (CodeSlayerApplicationClass *klass);
static void codeslayer_application_init        (CodeSlayerApplication      *application);
static void codeslayer_application_finalize    (CodeSlayerApplication      *application);
static void codeslayer_application_startup     (GApplication               *application);
static void codeslayer_application_shutdown    (GApplication               *application);
static void codeslayer_application_activate    (GApplication               *application);
static void codeslayer_application_open        (GApplication               *application,
                                                GFile                      **files,
                                                gint                        n_files,
                                                const gchar                *hint);
static GtkWindow* get_window                   (GApplication               *application);
static void verify_home_dir_exists             (void);
static void verify_plugins_dir_exists          (void);
static void verify_plugins_config_dir_exists   (void);
static void verify_profiles_dir_exists         (void);

#define CODESLAYER_APPLICATION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_APPLICATION_TYPE, CodeSlayerApplicationPrivate))

typedef struct _CodeSlayerApplicationPrivate CodeSlayerApplicationPrivate;

struct _CodeSlayerApplicationPrivate
{
  gchar *profile_name;
};

G_DEFINE_TYPE (CodeSlayerApplication, codeslayer_application, GTK_TYPE_APPLICATION)

static void 
codeslayer_application_class_init (CodeSlayerApplicationClass *klass)
{
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);
  application_class->startup = codeslayer_application_startup;
  application_class->shutdown = codeslayer_application_shutdown;
  application_class->activate = codeslayer_application_activate;
  application_class->open = codeslayer_application_open;

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_application_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerApplicationPrivate));
}

static void
codeslayer_application_init (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);
  priv->profile_name = NULL;
}

static void
codeslayer_application_finalize (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  if (priv->profile_name)
    g_free (priv->profile_name);

  G_OBJECT_CLASS (codeslayer_application_parent_class)->finalize (G_OBJECT (application));
}

static void
codeslayer_application_startup (GApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *window;

  G_APPLICATION_CLASS (codeslayer_application_parent_class)->startup (application);
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
  
  verify_home_dir_exists ();
  verify_plugins_dir_exists ();
  verify_plugins_config_dir_exists ();
  verify_profiles_dir_exists ();
  
  window = codeslayer_window_new (GTK_APPLICATION (application), priv->profile_name);
  gtk_application_add_window (GTK_APPLICATION (application), GTK_WINDOW (window));
}

static void
codeslayer_application_shutdown (GApplication *application)
{
  G_APPLICATION_CLASS (codeslayer_application_parent_class)->shutdown (application);
}

static void
codeslayer_application_activate (GApplication *application)
{
}

static void
codeslayer_application_open (GApplication *application,
                             GFile        **files,
                             gint         n_files,
                             const gchar  *hint)
{
  GtkWindow *window = NULL;
  gint i;
  
  for (i = 0; i < n_files; i++)
    {
      GFile *file = files[i];
      if (g_file_query_exists (file, NULL))
        {
          gchar *file_path = g_file_get_path (file);

          if (window == NULL)
            window = get_window (application);
        
          codeslayer_window_open_editor (CODESLAYER_WINDOW (window), file_path);
          g_free (file_path);
        }
    }
  
  if (window != NULL)  
    gtk_window_present (window);
}

/**
 * codeslayer_application_new:
 *
 * Creates a new #CodeSlayerApplication.
 *
 * Returns: a new #CodeSlayerApplication. 
 */
CodeSlayerApplication*
codeslayer_application_new (gchar *profile_name)
{
  CodeSlayerApplication *application;
  CodeSlayerApplicationPrivate *priv;

  application = CODESLAYER_APPLICATION (g_object_new (codeslayer_application_get_type (), NULL));
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  g_application_set_application_id (G_APPLICATION (application), "org.codeslayer");
  g_application_set_flags (G_APPLICATION (application), G_APPLICATION_HANDLES_OPEN);
  
  priv->profile_name = profile_name;

  return application;
}

/*
 * When opening files find the window that does not have projects enabled.
 * If no windows without projects exist then create a new window.
 */
static GtkWindow*
get_window (GApplication *application)
{
  GtkWidget *window;
  GList *windows;      

  windows = gtk_application_get_windows (GTK_APPLICATION (application));

  while (windows != NULL)
    {
      GtkWindow *current = windows->data;
      CodeSlayerProfile *profile = codeslayer_window_get_profile (CODESLAYER_WINDOW (current));
      if (!codeslayer_profile_get_enable_projects (profile))
        return GTK_WINDOW (current);
      windows = g_list_next (windows);
    }

  /* do not have window without projects so create one */
  
  window = codeslayer_window_new (GTK_APPLICATION (application), NULL);
  gtk_application_add_window (GTK_APPLICATION (application), GTK_WINDOW (window));  
  return GTK_WINDOW (window);
}

static void
verify_home_dir_exists (void)
{
  gchar *home_dir;
  GFile *file;
  
  home_dir = g_build_filename (g_get_home_dir (), CODESLAYER_HOME, NULL);
  file = g_file_new_for_path (home_dir);

  if (!g_file_query_exists (file, NULL)) 
    g_file_make_directory (file, NULL, NULL);

  g_free (home_dir);
  g_object_unref (file);
}

static void
verify_plugins_dir_exists (void)
{
  gchar *plugins_dir;
  GFile *file;
  
  plugins_dir = g_build_filename (g_get_home_dir (),
                                  CODESLAYER_HOME,
                                  CODESLAYER_PLUGINS_DIR,
                                  NULL);
  file = g_file_new_for_path (plugins_dir);

  if (!g_file_query_exists (file, NULL)) 
    g_file_make_directory (file, NULL, NULL);

  g_free (plugins_dir);
  g_object_unref (file);
}

static void
verify_profiles_dir_exists (void)
{
  gchar *profiles_dir;
  GFile *file;
  
  profiles_dir = g_build_filename (g_get_home_dir (),
                                  CODESLAYER_HOME,
                                  CODESLAYER_PROFILES_DIR,
                                  NULL);
  file = g_file_new_for_path (profiles_dir);

  if (!g_file_query_exists (file, NULL)) 
    g_file_make_directory (file, NULL, NULL);

  g_free (profiles_dir);
  g_object_unref (file);
}

static void
verify_plugins_config_dir_exists (void)
{
  gchar *configuration_dir;
  GFile *file;
  
  configuration_dir = g_build_filename (g_get_home_dir (),
                                        CODESLAYER_HOME,
                                        CODESLAYER_PLUGINS_DIR,
                                        CODESLAYER_PLUGINS_CONFIG_DIR,
                                        NULL);
  file = g_file_new_for_path (configuration_dir);

  if (!g_file_query_exists (file, NULL)) 
    g_file_make_directory (file, NULL, NULL);

  g_free (configuration_dir);
  g_object_unref (file);
}
