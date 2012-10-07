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

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-engine.h>
#include <codeslayer/codeslayer-processes.h>
#include <codeslayer/codeslayer-abstract-pane.h>
#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-project-properties.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-notebook-pane.h>
#include <codeslayer/codeslayer-notebook-search.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-statusbar.h>
#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-repository.h>
#include <codeslayer/codeslayer-settings.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer.h>
#include <codeslayer/codeslayer-utils.h>

typedef struct
{
  GtkWidget *window;
  CodeSlayerSettings *settings;
  CodeSlayerPreferences *preferences;
  GtkWidget *projects;
  GtkWidget *project_properties;
  GtkWidget *menubar;
  CodeSlayerEngine *engine;
  CodeSlayerProcesses *processes;
  CodeSlayer *codeslayer;
  GtkWidget *statusbar;
  GtkWidget *notebook;
  CodeSlayerGroups *groups;
  CodeSlayerPlugins *plugins;
  GtkWidget *notebook_pane;
  GtkWidget *side_pane;
  GtkWidget *bottom_pane;
  GtkWidget *hpaned;
  GtkWidget *vpaned;
} Context;

Context context;

static void create_settings                  (Context     *context);
static void create_preferences               (Context     *context);
static void create_plugins                   (Context     *context);
static void create_window                    (Context     *context);
static void create_groups                    (Context     *context);
static void create_menu                      (Context     *context);
static void create_projects                  (Context     *context);
static void create_project_properties        (Context     *context);
static void create_side_and_bottom_pane      (Context     *context);
static void create_notebook                  (Context     *context);
static void create_processes                 (Context     *context);
static void create_statusbar                 (Context     *context);
static void create_engine                    (Context     *context);
static void load_plugins                     (Context     *context);
static void create_paned_containers          (Context     *context);
static void pack_window                      (Context     *context);
static void set_visbility_of_panes           (Context     *context);
static void destroy                          (GtkWidget   *window, 
                                              Context     *context);
static gboolean delete_event                 (GtkWidget   *window, 
                                              GdkEvent    *event,
                                              Context     *context);
static void load_settings                    (Context     *context);
static void save_settings                    (Context     *context);
static void quit_application_action          (Context     *context);
static void verify_home_dir_exists           (void);
static void verify_groups_dir_exists         (void);
static void verify_groups_conf_exists        (void);
static void verify_plugins_dir_exists        (void);
static void verify_configuration_dir_exists  (void);

int
main (int   argc, 
      char *argv[])
{
  g_type_init ();
  gdk_threads_init ();
  gdk_threads_enter ();

  gtk_init (&argc, &argv);
  
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
  
  verify_home_dir_exists ();
  verify_groups_dir_exists ();
  verify_groups_conf_exists ();
  verify_plugins_dir_exists ();
  verify_configuration_dir_exists ();
  
  create_window (&context);

  create_settings (&context);

  create_preferences (&context);
  
  create_plugins (&context);

  load_settings (&context);

  create_groups (&context);

  create_menu (&context);

  create_notebook (&context);

  create_project_properties (&context);

  create_projects (&context);

  create_processes (&context);
  
  create_statusbar (&context);

  create_side_and_bottom_pane (&context);  
  
  create_engine (&context);

  load_plugins (&context);
  
  codeslayer_engine_open_active_group (context.engine);

  create_paned_containers (&context);

  pack_window (&context);

  gtk_widget_show_all (context.window);
  
  set_visbility_of_panes (&context);
  
  gtk_main ();
  gdk_threads_leave ();

  return 0;
}

static void
create_settings (Context *context)
{
  CodeSlayerSettings *settings;
  settings = codeslayer_settings_new ();
  context->settings = settings;
}

static void
create_preferences (Context *context)
{
  CodeSlayerPreferences *preferences;
  preferences = codeslayer_preferences_new (context->window);
  context->preferences = preferences;
}

static void
create_plugins (Context *context)
{
  CodeSlayerPlugins *plugins;
  plugins = codeslayer_plugins_new (context->window);
  context->plugins = plugins;
}

static void
create_window (Context *context)
{
  GtkWidget *window;
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  context->window = window;
  
  gtk_window_set_default_icon_name ("codeslayer");

  gtk_window_set_title (GTK_WINDOW (window),
                        CODESLAYER_SETTINGS_MAIN_TITLE);
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);

  g_signal_connect (G_OBJECT (window), "destroy", 
                    G_CALLBACK (destroy), context);
                    
  g_signal_connect (G_OBJECT (window), "delete_event",
                    G_CALLBACK (delete_event), context);
}

static void 
create_project_properties (Context *context)
{
  GtkWidget *project_properties;

  project_properties = codeslayer_project_properties_new ();
  context->project_properties = project_properties;
  
  /* take control of the reference otherwise the dialog will */
  g_object_ref_sink (context->project_properties);
}

static void 
create_projects (Context *context)
{
  GtkWidget *projects;
  projects = codeslayer_projects_new (context->window,
                                      context->preferences, 
                                      context->groups, 
                                      context->project_properties);
  context->projects = projects;
}

static void 
create_notebook (Context *context)
{
  GtkWidget *notebook;
  GtkWidget *notebook_search;
  GtkWidget *notebook_pane;

  notebook = codeslayer_notebook_new (GTK_WINDOW (context->window), context->preferences, context->settings);
  context->notebook = notebook;
  
  notebook_search = codeslayer_notebook_search_new (notebook, context->settings);
  notebook_pane = codeslayer_notebook_pane_new ();
  context->notebook_pane = notebook_pane;
  
  codeslayer_notebook_pane_set_notebook (CODESLAYER_NOTEBOOK_PANE (notebook_pane), notebook);
  codeslayer_notebook_pane_set_notebook_search (CODESLAYER_NOTEBOOK_PANE (notebook_pane), notebook_search);
}

static void 
create_side_and_bottom_pane (Context *context)
{
  GtkWidget *side_pane;
  GtkWidget *bottom_pane;

  side_pane = codeslayer_side_pane_new (context->preferences, context->statusbar);
  context->side_pane = side_pane;
  
  codeslayer_abstract_pane_add (CODESLAYER_ABSTRACT_PANE (side_pane), 
                                context->projects, "Projects");

  bottom_pane = codeslayer_bottom_pane_new (context->preferences);
  context->bottom_pane = bottom_pane;
}

static void 
create_processes (Context *context)
{
  CodeSlayerProcesses *processes;  
  processes = codeslayer_processes_new ();
  context->processes = processes;
}  

static void 
create_statusbar (Context *context)
{
  GtkWidget *statusbar;
  statusbar = codeslayer_statusbar_new (context->processes);
  context->statusbar = statusbar;
}

static void 
create_engine (Context *context)
{
  CodeSlayerEngine *engine;
  
  engine = codeslayer_engine_new (GTK_WINDOW (context->window), 
                                  context->settings, 
                                  context->preferences, 
                                  context->plugins, 
                                  context->groups, 
                                  context->projects, 
                                  context->menubar, 
                                  context->notebook_pane, 
                                  context->side_pane, 
                                  context->bottom_pane);
  context->engine = engine;
}

static void 
create_menu (Context *context)
{
  GtkWidget *menubar;
  menubar = codeslayer_menubar_new (context->window, context->groups, context->preferences, context->settings);
  context->menubar = menubar;
  
  g_signal_connect_swapped (G_OBJECT (menubar), "quit-application",
                            G_CALLBACK (quit_application_action), context);
}

static void 
create_groups (Context *context)
{
  CodeSlayerGroups *groups;
  groups = codeslayer_repository_get_groups ();
  context->groups = groups;
}

static void
load_plugins (Context *context)
{
  CodeSlayer *codeslayer;
  
  codeslayer = codeslayer_new (GTK_WINDOW (context->window), 
                               CODESLAYER_PREFERENCES (context->preferences), 
                               context->processes,
                               CODESLAYER_MENUBAR (context->menubar), 
                               CODESLAYER_NOTEBOOK (context->notebook), 
                               CODESLAYER_PROJECTS (context->projects), 
                               CODESLAYER_PROJECT_PROPERTIES (context->project_properties), 
                               CODESLAYER_SIDE_PANE (context->side_pane), 
                               CODESLAYER_BOTTOM_PANE (context->bottom_pane),
                               context->groups);
                               
  context->codeslayer = codeslayer;
  codeslayer_plugins_load (context->plugins, G_OBJECT(codeslayer));
}

static void
create_paned_containers (Context *context)
{
  GtkWidget *hpaned;
  GtkWidget *vpaned;
  gint hpaned_position;
  gint vpaned_position;

  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  context->hpaned = hpaned;

  vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  context->vpaned = vpaned;
  
  gtk_paned_pack1 (GTK_PANED (hpaned), GTK_WIDGET (context->side_pane), FALSE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), GTK_WIDGET (context->notebook_pane), TRUE, FALSE);

  hpaned_position = codeslayer_settings_get_integer (context->settings,
                                                     CODESLAYER_SETTINGS_HPANED_POSITION);
  if (hpaned_position == -1)
    hpaned_position = 250;

  gtk_paned_set_position (GTK_PANED (hpaned), hpaned_position);

  gtk_paned_pack1 (GTK_PANED (vpaned), GTK_WIDGET (hpaned), TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (vpaned), GTK_WIDGET (context->bottom_pane), FALSE, FALSE);

  vpaned_position = codeslayer_settings_get_integer (context->settings,
                                                     CODESLAYER_SETTINGS_VPANED_POSITION);
  if (vpaned_position == -1)
    vpaned_position = 250;

  gtk_paned_set_position (GTK_PANED (vpaned), vpaned_position);
}

static void
pack_window (Context *context)
{
  GtkWidget *vbox;
  
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (context->menubar), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), context->vpaned, TRUE, TRUE, 0);
  
  gtk_container_add (GTK_CONTAINER (context->window), vbox);
}

static void
set_visbility_of_panes (Context *context)
{
  gboolean show_side_pane;
  gboolean show_bottom_pane;

  show_side_pane = codeslayer_settings_get_boolean (context->settings,
                                                    CODESLAYER_SETTINGS_SIDE_PANE_VISIBLE);
  gtk_widget_set_visible (gtk_paned_get_child1 (GTK_PANED(context->hpaned)), 
                                                show_side_pane);

  show_bottom_pane = codeslayer_settings_get_boolean (context->settings,
                                                      CODESLAYER_SETTINGS_BOTTOM_PANE_VISIBLE);
  gtk_widget_set_visible (gtk_paned_get_child2 (GTK_PANED(context->vpaned)), 
                                                show_bottom_pane);
}

static void
destroy (GtkWidget *window, 
         Context   *context)
{
  g_object_unref (context->preferences);
  g_object_unref (context->settings);
  g_object_unref (context->engine);
  g_object_unref (context->groups);
  g_object_unref (context->plugins);
  g_object_unref (context->codeslayer);
  
  gtk_widget_destroy (GTK_WIDGET (context->project_properties));
  g_object_unref (context->project_properties);     

  gtk_main_quit ();
}

static gboolean
delete_event (GtkWidget *window, 
              GdkEvent  *event, 
              Context   *context)
{
  if (!codeslayer_engine_close_active_group (context->engine))
    return TRUE;

  save_settings (context);
  codeslayer_repository_save_groups (context->groups);
  codeslayer_plugins_deactivate (context->plugins);
  return FALSE;
}

static void
quit_application_action (Context *context)
{
  if (!codeslayer_engine_close_active_group (context->engine))
    return;
  
  save_settings (context);
  codeslayer_plugins_deactivate (context->plugins);
  gtk_widget_destroy (context->window);
}

static void
load_settings (Context *context)
{
  gint window_width;
  gint window_height;
  gint window_x;
  gint window_y;

  window_width = codeslayer_settings_get_integer (context->settings,
                                                  CODESLAYER_SETTINGS_WINDOW_WIDTH);
  if (window_width < 0)
    {
      window_width = 800;
    }
  
  window_height = codeslayer_settings_get_integer (context->settings,
                                                   CODESLAYER_SETTINGS_WINDOW_HEIGHT);
  if (window_height < 0)
    {
      window_height = 600;
    }
    
  gtk_window_set_default_size (GTK_WINDOW (context->window), window_width, window_height);

  window_x = codeslayer_settings_get_integer (context->settings,
                                              CODESLAYER_SETTINGS_WINDOW_X);
  if (window_x < 0)
    {
      window_x = 10;
    }
    
  window_y = codeslayer_settings_get_integer (context->settings,
                                              CODESLAYER_SETTINGS_WINDOW_Y);
  if (window_y < 0)
    {
      window_y = 10;
    }
    
  gtk_window_move (GTK_WINDOW (context->window), window_x, window_y);
}

static void
save_settings (Context *context)
{
  gint width;
  gint height;
  gint x;
  gint y;
  gint position;

  gtk_window_get_size (GTK_WINDOW (context->window), &width, &height);
  codeslayer_settings_set_integer (context->settings,
                                   CODESLAYER_SETTINGS_WINDOW_WIDTH,
                                   width);
  codeslayer_settings_set_integer (context->settings,
                                   CODESLAYER_SETTINGS_WINDOW_HEIGHT,
                                   height);

  gtk_window_get_position (GTK_WINDOW (context->window), &x, &y);
  codeslayer_settings_set_integer (context->settings,
                                   CODESLAYER_SETTINGS_WINDOW_X, x);
  codeslayer_settings_set_integer (context->settings,
                                   CODESLAYER_SETTINGS_WINDOW_Y, y);

  position = gtk_paned_get_position (GTK_PANED (context->hpaned));
  codeslayer_settings_set_integer (context->settings,
                                   CODESLAYER_SETTINGS_HPANED_POSITION,
                                   position);

  position = gtk_paned_get_position (GTK_PANED (context->vpaned));
  codeslayer_settings_set_integer (context->settings,
                                   CODESLAYER_SETTINGS_VPANED_POSITION,
                                   position);

  codeslayer_settings_save (context->settings);
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
verify_groups_dir_exists (void)
{
  gchar *groups_path;
  GFile *groups_file;

  groups_path = g_build_filename (g_get_home_dir (), 
                                  CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, NULL);
  groups_file = g_file_new_for_path (groups_path);
  if (!g_file_query_exists (groups_file, NULL))
    {
      gchar *default_group_path;
      GFile *default_group_file;

      g_file_make_directory (groups_file, NULL, NULL);

      default_group_path = g_build_filename (g_get_home_dir (), 
                                             CODESLAYER_HOME, 
                                             CODESLAYER_GROUPS_DIR, 
                                             CODESLAYER_DEFAULT_GROUP_DIR, NULL);
      default_group_file = g_file_new_for_path (default_group_path);
      if (!g_file_query_exists (default_group_file, NULL))
        g_file_make_directory (default_group_file, NULL, NULL);

      g_free (default_group_path);
      g_object_unref (default_group_file);
    }

  g_free (groups_path);
  g_object_unref (groups_file);
}

static void
verify_groups_conf_exists ()
{
  gchar *conf_path;
  GFile *conf_file;

  conf_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, 
                                CODESLAYER_GROUPS_DIR, 
                                CODESLAYER_GROUPS_CONF, 
                                NULL);
  
  conf_file = g_file_new_for_path (conf_path);
  if (!g_file_query_exists (conf_file, NULL))
    {
      GFileIOStream *stream;
      stream = g_file_create_readwrite (conf_file, G_FILE_CREATE_NONE, NULL, NULL);
      g_io_stream_close (G_IO_STREAM (stream), NULL, NULL);
      g_object_unref (stream);
    }

  g_free (conf_path);
  g_object_unref (conf_file);
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
verify_configuration_dir_exists (void)
{
  gchar *configuration_dir;
  GFile *file;
  
  configuration_dir = g_build_filename (g_get_home_dir (),
                                        CODESLAYER_HOME,
                                        CODESLAYER_PLUGINS_DIR,
                                        CODESLAYER_CONFIGURATION_DIR,
                                        NULL);
  file = g_file_new_for_path (configuration_dir);

  if (!g_file_query_exists (file, NULL)) 
    g_file_make_directory (file, NULL, NULL);

  g_free (configuration_dir);
  g_object_unref (file);
}
