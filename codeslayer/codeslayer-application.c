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
static gint codeslayer_application_options     (GApplication               *application,
                                                GVariantDict               *options);

static void show_profiles                      (void);
static void codeslayer_application_open        (GApplication               *application,
                                                GFile                      **files,
                                                gint                        n_files,
                                                const gchar                *hint);
static GtkWindow* get_window                   (GApplication               *application);
static void verify_home_dir_exists             (void);
static void verify_plugins_dir_exists          (void);
static void verify_plugins_config_dir_exists   (void);
static void verify_profiles_dir_exists         (void);

#define CODESLAYER_APPLICATION_VERSION "version"
#define CODESLAYER_APPLICATION_SHOW_PROFILES "show-profiles"
#define CODESLAYER_APPLICATION_OPEN_PROFILE "open-profile"

G_DEFINE_TYPE (CodeSlayerApplication, codeslayer_application, GTK_TYPE_APPLICATION)

static gboolean version_arg = FALSE;
static gboolean show_profiles_arg = FALSE;
static gchar *open_profile_arg = NULL;

static GOptionEntry entries[] =
{
{ CODESLAYER_APPLICATION_VERSION, 'v', 0, G_OPTION_ARG_NONE, &version_arg, "The current version", NULL },
{ CODESLAYER_APPLICATION_SHOW_PROFILES, 's', 0, G_OPTION_ARG_NONE, &show_profiles_arg, "Show all the profiles", NULL },
{ CODESLAYER_APPLICATION_OPEN_PROFILE, 'p', 0, G_OPTION_ARG_STRING, &open_profile_arg, "Open the named profile (ex: -p test).", NULL },
{ NULL }
};

static void 
codeslayer_application_class_init (CodeSlayerApplicationClass *klass)
{
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);
  application_class->startup = codeslayer_application_startup;
  application_class->shutdown = codeslayer_application_shutdown;
  application_class->handle_local_options = codeslayer_application_options;
  application_class->activate = codeslayer_application_activate;
  application_class->open = codeslayer_application_open;
  
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_application_finalize;
}

static void
codeslayer_application_init (CodeSlayerApplication *application)
{
  g_application_add_main_option_entries (G_APPLICATION (application), entries);
}

static void
codeslayer_application_finalize (CodeSlayerApplication *application)
{
  if (open_profile_arg != NULL)
    g_free (open_profile_arg);

  G_OBJECT_CLASS (codeslayer_application_parent_class)->finalize (G_OBJECT (application));
}

/**
 * codeslayer_application_new:
 *
 * Creates a new #CodeSlayerApplication.
 *
 * Returns: a new #CodeSlayerApplication. 
 */
CodeSlayerApplication*
codeslayer_application_new (void)
{
  CodeSlayerApplication *application;
  application = CODESLAYER_APPLICATION (g_object_new (codeslayer_application_get_type (), NULL));
  
  if (g_strcmp0 (CODESLAYER_HOME, ".codeslayer-dev") == 0)
    g_application_set_application_id (G_APPLICATION (application), "org.codeslayer.dev");
  else
    g_application_set_application_id (G_APPLICATION (application), "org.codeslayer");
  
  g_application_set_flags (G_APPLICATION (application), G_APPLICATION_HANDLES_OPEN);

  return application;
}

static gint
codeslayer_application_options (GApplication *application,
                                GVariantDict *dict)
{
  if (version_arg)
    {
      g_print ("%s\n", PACKAGE_STRING);
      return 0;
    }

  if (show_profiles_arg)
    {
      show_profiles();
      return 0;      
    }

  if (open_profile_arg != NULL)
    {
      if (!codeslayer_utils_profile_exists (open_profile_arg))
        {
          g_print ("The profile name '%s' is invalid\n", open_profile_arg);
          return 0;
        }
      return -1;      
    }

  return -1;
}

static gint
compare_profiles (gchar *a, 
                  gchar *b)
{
  return g_strcmp0 (a, b);
}

static void 
show_profiles ()
{
  GList *profile_names;
  GList *list;

  profile_names = codeslayer_utils_get_profile_names ();
  list = profile_names;
  
  list = g_list_sort (list, (GCompareFunc) compare_profiles);

  while (list != NULL)
    {
      gchar *profile_name = list->data;
      g_print ("%s\n", profile_name);
      list = g_list_next (list);
    }

  g_list_free_full (profile_names, g_free);
}

static void
codeslayer_application_startup (GApplication *application)
{
  GtkWidget *window;

  G_APPLICATION_CLASS (codeslayer_application_parent_class)->startup (application);
  
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
  
  verify_home_dir_exists ();
  verify_plugins_dir_exists ();
  verify_plugins_config_dir_exists ();
  verify_profiles_dir_exists ();

  if (open_profile_arg != NULL)
    window = codeslayer_window_new (GTK_APPLICATION (application), open_profile_arg);
  else     
    window = codeslayer_window_new (GTK_APPLICATION (application), CODESLAYER_PROFILES_DEFAULT);
  
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
                             gint          n_files,
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
        
          codeslayer_window_open_document (CODESLAYER_WINDOW (window), file_path);
          g_free (file_path);
        }
    }
  
  if (window != NULL)  
    gtk_window_present (window);
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

  /* do not have window without projects enabled so create default window */
  
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
