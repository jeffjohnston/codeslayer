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
#include <codeslayer/codeslayer-abstract-engine.h>
#include <codeslayer/codeslayer-engine.h>
#include <codeslayer/codeslayer-processes.h>
#include <codeslayer/codeslayer-abstract-pane.h>
#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-projects-engine.h>
#include <codeslayer/codeslayer-project-properties.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-notebook-pane.h>
#include <codeslayer/codeslayer-notebook-search.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-processbar.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-profiles.h>
#include <codeslayer/codeslayer-profiles-manager.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer.h>
#include <codeslayer/codeslayer-utils.h>

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
                                                gint                       n_files,
                                                const gchar                *hint);
static void create_profiles                    (CodeSlayerApplication      *application);
static void create_profiles                    (CodeSlayerApplication      *application);
static void create_plugins                     (CodeSlayerApplication      *application);
static void create_window                      (CodeSlayerApplication      *application);
static void create_menu                        (CodeSlayerApplication      *application);
static void create_projects                    (CodeSlayerApplication      *application);
static void create_project_properties          (CodeSlayerApplication      *application);
static void create_side_and_bottom_pane        (CodeSlayerApplication      *application);
static void create_notebook                    (CodeSlayerApplication      *application);
static void create_processes                   (CodeSlayerApplication      *application);
static void create_process_bar                 (CodeSlayerApplication      *application);
static void create_engine                      (CodeSlayerApplication      *application);
static void create_profiles_manager            (CodeSlayerApplication      *application);
static void load_plugins                       (CodeSlayerApplication      *application);
static void create_paned_containers            (CodeSlayerApplication      *application);
static void pack_window                        (CodeSlayerApplication      *application);
static void destroy                            (GtkWidget                  *window, 
                                                CodeSlayerApplication      *application);
static gboolean delete_event                   (GtkWidget                  *window, 
                                                GdkEvent                   *event,
                                                CodeSlayerApplication      *application);
static void quit_application_action            (CodeSlayerApplication      *application);
static void verify_home_dir_exists             (void);
static void verify_plugins_dir_exists          (void);
static void verify_plugins_config_dir_exists   (void);
static void verify_profiles_dir_exists         (void);

#define CODESLAYER_APPLICATION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_APPLICATION_TYPE, CodeSlayerApplicationPrivate))

typedef struct _CodeSlayerApplicationPrivate CodeSlayerApplicationPrivate;

struct _CodeSlayerApplicationPrivate
{
  GtkWidget                 *window;
  gchar                     *profile_name;
  CodeSlayerPreferences     *preferences;
  CodeSlayerProfiles        *profiles;
  CodeSlayerProfilesManager *profiles_manager;
  GtkWidget                 *projects;
  GtkWidget                 *project_properties;
  GtkWidget                 *menubar;
  CodeSlayerEngine          *engine;
  CodeSlayerProjectsEngine  *projects_engine;
  CodeSlayerProcesses       *processes;
  CodeSlayer                *codeslayer;
  GtkWidget                 *process_bar;
  GtkWidget                 *notebook;
  CodeSlayerPlugins         *plugins;
  GtkWidget                 *notebook_pane;
  GtkWidget                 *side_pane;
  GtkWidget                 *bottom_pane;
  GtkWidget                 *hpaned;
  GtkWidget                 *vpaned;
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
  priv->profiles = NULL;
  priv->profiles_manager = NULL;
  priv->engine = NULL;
  priv->projects_engine = NULL;
  priv->plugins = NULL;
  priv->codeslayer = NULL;
  priv->profile_name = NULL;
}

static void
codeslayer_application_finalize (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);
  
  if (priv->profile_name)
    g_free (priv->profile_name);

  if (priv->profiles)
    g_object_unref (priv->profiles);
  
  if (priv->profiles_manager)
    g_object_unref (priv->profiles_manager);
  
  if (priv->engine)
    g_object_unref (priv->engine);
  
  if (priv->projects_engine)
    g_object_unref (priv->projects_engine);
  
  if (priv->plugins)
    g_object_unref (priv->plugins);
  
  if (priv->codeslayer)
    g_object_unref (priv->codeslayer);

  G_OBJECT_CLASS (codeslayer_application_parent_class)->finalize (G_OBJECT (application));
}

static void
codeslayer_application_startup (GApplication *application)
{
  CodeSlayerApplicationPrivate *priv;

  G_APPLICATION_CLASS (codeslayer_application_parent_class)->startup (application);
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
  
  verify_home_dir_exists ();
  verify_plugins_dir_exists ();
  verify_plugins_config_dir_exists ();
  verify_profiles_dir_exists ();
  
  create_profiles (CODESLAYER_APPLICATION (application));

  create_window (CODESLAYER_APPLICATION (application));

  create_plugins (CODESLAYER_APPLICATION (application));

  create_menu (CODESLAYER_APPLICATION (application));

  create_notebook (CODESLAYER_APPLICATION (application));

  create_project_properties (CODESLAYER_APPLICATION (application));

  create_projects (CODESLAYER_APPLICATION (application));

  create_processes (CODESLAYER_APPLICATION (application));
  
  create_process_bar (CODESLAYER_APPLICATION (application));

  create_side_and_bottom_pane (CODESLAYER_APPLICATION (application));  
  
  create_paned_containers (CODESLAYER_APPLICATION (application));

  create_engine (CODESLAYER_APPLICATION (application));

  create_profiles_manager (CODESLAYER_APPLICATION (application));

  pack_window (CODESLAYER_APPLICATION (application));

  load_plugins (CODESLAYER_APPLICATION (application));
  
  codeslayer_engine_load_profile (priv->engine);
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
  CodeSlayerApplicationPrivate *priv;
  gint i;

  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  for (i = 0; i < n_files; i++)
    {
      GFile *file = files[i];
      if (g_file_query_exists (file, NULL))
        {
          gchar *file_path = g_file_get_path (file);
          codeslayer_engine_open_editor (priv->engine, file_path);
          g_free (file_path);
        }
    }
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
  priv->profile_name = profile_name;

  g_application_set_application_id (G_APPLICATION (application), NULL);
  g_application_set_flags (G_APPLICATION (application), G_APPLICATION_HANDLES_OPEN);

  return application;
}

static void
create_profiles (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  CodeSlayerProfiles *profiles;
  CodeSlayerProfile *profile = NULL;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  profiles = codeslayer_profiles_new ();
  priv->profiles = profiles;
  
  if (priv->profile_name != NULL)
    {
      profile = codeslayer_profiles_retrieve_profile (priv->profiles, priv->profile_name);
      if (profile == NULL)
        g_warning ("The profile name '%s' is invalid", priv->profile_name);
    }

  if (profile == NULL)
    profile = codeslayer_profiles_retrieve_profile (priv->profiles, CODESLAYER_PROFILES_DEFAULT);
  
  if (profile == NULL)
    profile = codeslayer_profiles_create_profile (priv->profiles, CODESLAYER_PROFILES_DEFAULT);
  
  codeslayer_profiles_load_profile (profiles, profile);
}

static void
create_plugins (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  CodeSlayerPlugins *plugins;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  plugins = codeslayer_plugins_new (priv->window);
  priv->plugins = plugins;
}

static void
create_window (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *window;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  priv->window = window;
  
  gtk_window_set_default_icon_name ("codeslayer");

  gtk_window_set_title (GTK_WINDOW (window),
                        CODESLAYER_REGISTRY_MAIN_TITLE);
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);

  g_signal_connect (G_OBJECT (window), "destroy", 
                    G_CALLBACK (destroy), application);
                    
  g_signal_connect (G_OBJECT (window), "delete_event",
                    G_CALLBACK (delete_event), application);

  gtk_application_add_window (GTK_APPLICATION (application), 
                              GTK_WINDOW (priv->window));
}

static void 
create_project_properties (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *project_properties;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  project_properties = codeslayer_project_properties_new ();
  priv->project_properties = project_properties;
  
  /* take control of the reference otherwise the dialog will */
  g_object_ref_sink (priv->project_properties);
}

static void 
create_projects (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *projects;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  projects = codeslayer_projects_new (priv->window,
                                      priv->profiles, 
                                      priv->project_properties);
  priv->projects = projects;
}

static void 
create_notebook (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *notebook;
  GtkWidget *notebook_search;
  GtkWidget *notebook_pane;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  notebook = codeslayer_notebook_new (GTK_WINDOW (priv->window), priv->profiles);
  priv->notebook = notebook;
  
  notebook_search = codeslayer_notebook_search_new (notebook, priv->profiles);
  notebook_pane = codeslayer_notebook_pane_new ();
  priv->notebook_pane = notebook_pane;
  
  codeslayer_notebook_pane_set_notebook (CODESLAYER_NOTEBOOK_PANE (notebook_pane), notebook);
  codeslayer_notebook_pane_set_notebook_search (CODESLAYER_NOTEBOOK_PANE (notebook_pane), notebook_search);
}

static void 
create_side_and_bottom_pane (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *side_pane;
  GtkWidget *bottom_pane;

  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  side_pane = codeslayer_side_pane_new (priv->profiles, priv->process_bar);
  priv->side_pane = side_pane;
  
  bottom_pane = codeslayer_bottom_pane_new (priv->profiles);
  priv->bottom_pane = bottom_pane;
}

static void 
create_processes (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  CodeSlayerProcesses *processes;  
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  processes = codeslayer_processes_new ();
  priv->processes = processes;
}  

static void 
create_process_bar (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *process_bar;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  process_bar = codeslayer_process_bar_new (priv->processes);
  priv->process_bar = process_bar;
}

static void 
create_engine (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  CodeSlayerEngine *engine;
  CodeSlayerProjectsEngine *projects_engine;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  projects_engine = codeslayer_projects_engine_new (GTK_WINDOW (priv->window), 
                                                    priv->profiles,
                                                    priv->plugins,
                                                    priv->projects,
                                                    priv->menubar, 
                                                    priv->notebook, 
                                                    priv->notebook_pane, 
                                                    priv->side_pane, 
                                                    priv->bottom_pane, 
                                                    priv->hpaned, 
                                                    priv->vpaned);
  priv->projects_engine = projects_engine;

  engine = codeslayer_engine_new (GTK_WINDOW (priv->window), 
                                  priv->projects_engine,
                                  priv->profiles,
                                  priv->plugins,
                                  priv->menubar, 
                                  priv->notebook, 
                                  priv->notebook_pane, 
                                  priv->side_pane, 
                                  priv->bottom_pane, 
                                  priv->hpaned, 
                                  priv->vpaned);
  priv->engine = engine;
}

static void 
create_profiles_manager (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  priv->profiles_manager = codeslayer_profiles_manager_new (GTK_WIDGET (priv->window), 
                                                            priv->profiles, 
                                                            priv->engine, 
                                                            priv->projects_engine);
  
  g_signal_connect_swapped (G_OBJECT (priv->menubar), "show-profiles",
                            G_CALLBACK (codeslayer_profiles_manager_run_dialog), 
                            priv->profiles_manager);
}

static void 
create_menu (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *menubar;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  menubar = codeslayer_menu_bar_new (priv->window, priv->profiles);
  priv->menubar = menubar;
  
  g_signal_connect_swapped (G_OBJECT (menubar), "quit-application",
                            G_CALLBACK (quit_application_action), application);
}

static void
load_plugins (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  CodeSlayer *codeslayer;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  codeslayer = codeslayer_new (GTK_WINDOW (priv->window), 
                               priv->profiles,
                               priv->processes,
                               CODESLAYER_MENU_BAR (priv->menubar), 
                               CODESLAYER_NOTEBOOK (priv->notebook), 
                               CODESLAYER_PROJECTS (priv->projects), 
                               CODESLAYER_PROJECT_PROPERTIES (priv->project_properties), 
                               CODESLAYER_SIDE_PANE (priv->side_pane), 
                               CODESLAYER_BOTTOM_PANE (priv->bottom_pane));
                               
  priv->codeslayer = codeslayer;
  codeslayer_plugins_load (priv->plugins, G_OBJECT(codeslayer));
}

static void
create_paned_containers (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *hpaned;
  GtkWidget *vpaned;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  priv->hpaned = hpaned;

  vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  priv->vpaned = vpaned;
  
  gtk_paned_pack1 (GTK_PANED (hpaned), GTK_WIDGET (priv->side_pane), FALSE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), GTK_WIDGET (priv->notebook_pane), TRUE, FALSE);
  gtk_paned_set_position (GTK_PANED (hpaned), 250);

  gtk_paned_pack1 (GTK_PANED (vpaned), GTK_WIDGET (hpaned), TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (vpaned), GTK_WIDGET (priv->bottom_pane), FALSE, FALSE);
  gtk_paned_set_position (GTK_PANED (vpaned), 250);
}

static void
pack_window (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *vbox;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);
  
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->menubar), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), priv->vpaned, TRUE, TRUE, 0);
  
  gtk_container_add (GTK_CONTAINER (priv->window), vbox);
}

static void
destroy (GtkWidget             *window, 
         CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);
  gtk_widget_destroy (GTK_WIDGET (priv->project_properties));
  g_object_unref (priv->project_properties);     
}

static gboolean
delete_event (GtkWidget             *window, 
              GdkEvent              *event, 
              CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  if (!codeslayer_abstract_engine_save_profile (CODESLAYER_ABSTRACT_ENGINE (priv->engine)))
    return TRUE;

  codeslayer_notebook_close_all_editors (CODESLAYER_NOTEBOOK (priv->notebook));
  codeslayer_plugins_deactivate (priv->plugins);
  return FALSE;
}

static void
quit_application_action (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  if (!codeslayer_abstract_engine_save_profile (CODESLAYER_ABSTRACT_ENGINE (priv->engine)))
    return;
  
  codeslayer_notebook_close_all_editors (CODESLAYER_NOTEBOOK (priv->notebook));
  codeslayer_plugins_deactivate (priv->plugins);
  gtk_widget_destroy (priv->window);
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
