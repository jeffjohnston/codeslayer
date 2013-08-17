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
#include <codeslayer/codeslayer-repository.h>
#include <codeslayer/codeslayer-settings.h>
#include <codeslayer/codeslayer-preferences.h>
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
static void create_settings                    (CodeSlayerApplication      *application);
static void create_preferences                 (CodeSlayerApplication      *application);
static void create_plugins                     (CodeSlayerApplication      *application);
static void create_window                      (CodeSlayerApplication      *application);
static void create_groups                      (CodeSlayerApplication      *application);
static void create_menu                        (CodeSlayerApplication      *application);
static void create_projects                    (CodeSlayerApplication      *application);
static void create_project_properties          (CodeSlayerApplication      *application);
static void create_side_and_bottom_pane        (CodeSlayerApplication      *application);
static void create_notebook                    (CodeSlayerApplication      *application);
static void create_processes                   (CodeSlayerApplication      *application);
static void create_process_bar                 (CodeSlayerApplication      *application);
static void create_engine                      (CodeSlayerApplication      *application);
static void load_plugins                       (CodeSlayerApplication      *application);
static void create_paned_containers            (CodeSlayerApplication      *application);
static void pack_window                        (CodeSlayerApplication      *application);
static void set_visbility_of_panes             (CodeSlayerApplication      *application);
static void destroy                            (GtkWidget                  *window, 
                                                CodeSlayerApplication      *application);
static gboolean delete_event                   (GtkWidget                  *window, 
                                                GdkEvent                   *event,
                                                CodeSlayerApplication      *application);
static void load_settings                      (CodeSlayerApplication      *application);
static void save_settings                      (CodeSlayerApplication      *application);
static void quit_application_action            (CodeSlayerApplication      *application);
static void verify_home_dir_exists             (void);
static void verify_plugins_dir_exists          (void);
static void verify_plugins_config_dir_exists   (void);

#define CODESLAYER_APPLICATION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_APPLICATION_TYPE, CodeSlayerApplicationPrivate))

typedef struct _CodeSlayerApplicationPrivate CodeSlayerApplicationPrivate;

struct _CodeSlayerApplicationPrivate
{
  GtkWidget                *window;
  CodeSlayerSettings       *settings;
  CodeSlayerPreferences    *preferences;
  GtkWidget                *projects;
  GtkWidget                *project_properties;
  GtkWidget                *menubar;
  CodeSlayerEngine         *engine;
  CodeSlayerProjectsEngine *projects_engine;
  CodeSlayerProcesses      *processes;
  CodeSlayer               *codeslayer;
  GtkWidget                *process_bar;
  GtkWidget                *notebook;
  CodeSlayerGroup          *group;
  CodeSlayerPlugins        *plugins;
  GtkWidget                *notebook_pane;
  GtkWidget                *side_pane;
  GtkWidget                *bottom_pane;
  GtkWidget                *hpaned;
  GtkWidget                *vpaned;
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
}

static void
codeslayer_application_finalize (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  g_object_unref (priv->preferences);
  g_object_unref (priv->settings);
  g_object_unref (priv->engine);
  /*g_object_unref (priv->group);*/
  g_object_unref (priv->plugins);
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
  
  create_window (CODESLAYER_APPLICATION (application));

  create_settings (CODESLAYER_APPLICATION (application));

  create_preferences (CODESLAYER_APPLICATION (application));
  
  create_plugins (CODESLAYER_APPLICATION (application));

  load_settings (CODESLAYER_APPLICATION (application));

  create_groups (CODESLAYER_APPLICATION (application));

  create_menu (CODESLAYER_APPLICATION (application));

  create_notebook (CODESLAYER_APPLICATION (application));

  create_project_properties (CODESLAYER_APPLICATION (application));

  create_projects (CODESLAYER_APPLICATION (application));

  create_processes (CODESLAYER_APPLICATION (application));
  
  create_process_bar (CODESLAYER_APPLICATION (application));

  create_side_and_bottom_pane (CODESLAYER_APPLICATION (application));  
  
  create_engine (CODESLAYER_APPLICATION (application));

  create_paned_containers (CODESLAYER_APPLICATION (application));

  pack_window (CODESLAYER_APPLICATION (application));

  gtk_widget_show_all (priv->window);
  
  set_visbility_of_panes (CODESLAYER_APPLICATION (application));

  load_plugins (CODESLAYER_APPLICATION (application));
  
  codeslayer_projects_engine_open_active_group (priv->projects_engine);  
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
  g_application_set_application_id (G_APPLICATION (application), NULL);
  g_application_set_flags (G_APPLICATION (application), G_APPLICATION_HANDLES_OPEN);
  return application;
}

static void
create_settings (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  CodeSlayerSettings *settings;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  settings = codeslayer_settings_new ();
  priv->settings = settings;
}

static void
create_preferences (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  CodeSlayerPreferences *preferences;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  preferences = codeslayer_preferences_new (priv->window);
  priv->preferences = preferences;
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
                        CODESLAYER_SETTINGS_MAIN_TITLE);
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
                                      priv->preferences, 
                                      priv->settings, 
                                      priv->group, 
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

  notebook = codeslayer_notebook_new (GTK_WINDOW (priv->window), priv->preferences, priv->settings);
  priv->notebook = notebook;
  
  notebook_search = codeslayer_notebook_search_new (notebook, priv->settings);
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

  side_pane = codeslayer_side_pane_new (priv->preferences, priv->process_bar);
  priv->side_pane = side_pane;
  
  codeslayer_abstract_pane_add (CODESLAYER_ABSTRACT_PANE (side_pane), 
                                priv->projects, "Projects");

  bottom_pane = codeslayer_bottom_pane_new (priv->preferences);
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

  engine = codeslayer_engine_new (GTK_WINDOW (priv->window), 
                                  priv->settings, 
                                  priv->preferences, 
                                  priv->plugins, 
                                  priv->menubar, 
                                  priv->notebook_pane, 
                                  priv->side_pane, 
                                  priv->bottom_pane);
  priv->engine = engine;

  projects_engine = codeslayer_projects_engine_new (GTK_WINDOW (priv->window), 
                                  priv->settings, 
                                  priv->preferences, 
                                  priv->plugins, 
                                  priv->group, 
                                  priv->projects, 
                                  priv->menubar, 
                                  priv->notebook_pane, 
                                  priv->side_pane, 
                                  priv->bottom_pane);
  priv->engine = engine;
  priv->projects_engine = projects_engine;
}

static void 
create_menu (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  GtkWidget *menubar;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  menubar = codeslayer_menu_bar_new (priv->window, priv->preferences, priv->settings);
  priv->menubar = menubar;
  
  g_signal_connect_swapped (G_OBJECT (menubar), "quit-application",
                            G_CALLBACK (quit_application_action), application);
}

static void 
create_groups (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);
  priv->group = codeslayer_repository_get_group ();
}

static void
load_plugins (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  CodeSlayer *codeslayer;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  codeslayer = codeslayer_new (GTK_WINDOW (priv->window), 
                               CODESLAYER_PREFERENCES (priv->preferences), 
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
  gint hpaned_position;
  gint vpaned_position;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  priv->hpaned = hpaned;

  vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  priv->vpaned = vpaned;
  
  gtk_paned_pack1 (GTK_PANED (hpaned), GTK_WIDGET (priv->side_pane), FALSE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), GTK_WIDGET (priv->notebook_pane), TRUE, FALSE);

  hpaned_position = codeslayer_settings_get_integer (priv->settings,
                                                     CODESLAYER_SETTINGS_HPANED_POSITION);
  if (hpaned_position == -1)
    hpaned_position = 250;

  gtk_paned_set_position (GTK_PANED (hpaned), hpaned_position);

  gtk_paned_pack1 (GTK_PANED (vpaned), GTK_WIDGET (hpaned), TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (vpaned), GTK_WIDGET (priv->bottom_pane), FALSE, FALSE);

  vpaned_position = codeslayer_settings_get_integer (priv->settings,
                                                     CODESLAYER_SETTINGS_VPANED_POSITION);
  if (vpaned_position == -1)
    vpaned_position = 250;

  gtk_paned_set_position (GTK_PANED (vpaned), vpaned_position);
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
set_visbility_of_panes (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  gboolean show_side_pane;
  gboolean show_bottom_pane;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  show_side_pane = codeslayer_settings_get_boolean (priv->settings,
                                                    CODESLAYER_SETTINGS_SIDE_PANE_VISIBLE);
  gtk_widget_set_visible (gtk_paned_get_child1 (GTK_PANED(priv->hpaned)), 
                                                show_side_pane);

  show_bottom_pane = codeslayer_settings_get_boolean (priv->settings,
                                                      CODESLAYER_SETTINGS_BOTTOM_PANE_VISIBLE);
  gtk_widget_set_visible (gtk_paned_get_child2 (GTK_PANED(priv->vpaned)), 
                                                show_bottom_pane);
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

  if (!codeslayer_projects_engine_close_active_group (priv->projects_engine))
    return TRUE;

  save_settings (application);
  /*TODO: this is where we want to save the projects file*/
  /*codeslayer_repository_save_groups (priv->groups);*/
  return FALSE;
}

static void
quit_application_action (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  if (!codeslayer_projects_engine_close_active_group (priv->projects_engine))
    return;
  
  save_settings (application);
  gtk_widget_destroy (priv->window);
}

static void
load_settings (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  gint window_width;
  gint window_height;
  gint window_x;
  gint window_y;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  window_width = codeslayer_settings_get_integer (priv->settings,
                                                  CODESLAYER_SETTINGS_WINDOW_WIDTH);
  if (window_width < 0)
    {
      window_width = 800;
    }
  
  window_height = codeslayer_settings_get_integer (priv->settings,
                                                   CODESLAYER_SETTINGS_WINDOW_HEIGHT);
  if (window_height < 0)
    {
      window_height = 600;
    }
    
  gtk_window_set_default_size (GTK_WINDOW (priv->window), window_width, window_height);

  window_x = codeslayer_settings_get_integer (priv->settings,
                                              CODESLAYER_SETTINGS_WINDOW_X);
  if (window_x < 0)
    {
      window_x = 10;
    }
    
  window_y = codeslayer_settings_get_integer (priv->settings,
                                              CODESLAYER_SETTINGS_WINDOW_Y);
  if (window_y < 0)
    {
      window_y = 10;
    }
    
  gtk_window_move (GTK_WINDOW (priv->window), window_x, window_y);
}

static void
save_settings (CodeSlayerApplication *application)
{
  CodeSlayerApplicationPrivate *priv;
  gint width;
  gint height;
  gint x;
  gint y;
  gint position;
  
  priv = CODESLAYER_APPLICATION_GET_PRIVATE (application);

  gtk_window_get_size (GTK_WINDOW (priv->window), &width, &height);
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_WINDOW_WIDTH,
                                   width);
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_WINDOW_HEIGHT,
                                   height);

  gtk_window_get_position (GTK_WINDOW (priv->window), &x, &y);
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_WINDOW_X, x);
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_WINDOW_Y, y);

  position = gtk_paned_get_position (GTK_PANED (priv->hpaned));
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_HPANED_POSITION,
                                   position);

  position = gtk_paned_get_position (GTK_PANED (priv->vpaned));
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_VPANED_POSITION,
                                   position);

  codeslayer_settings_save (priv->settings);
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
