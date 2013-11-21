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

#include <codeslayer/codeslayer-window.h>
#include <codeslayer/codeslayer-engine.h>
#include <codeslayer/codeslayer-processes.h>
#include <codeslayer/codeslayer-abstract-pane.h>
#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-project-properties.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-sourceview.h>
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
 * SECTION:codeslayer-window
 * @short_description: Contains the pages.
 * @title: CodeSlayerWindow
 * @include: codeslayer/codeslayer-window.h
 *
 * A window which contains pages of documents to edit.
 */

static void codeslayer_window_class_init  (CodeSlayerWindowClass *klass);
static void codeslayer_window_init        (CodeSlayerWindow      *window);
static void codeslayer_window_finalize    (CodeSlayerWindow      *window);

static void create_profiles               (CodeSlayerWindow      *window);
static void create_plugins                (CodeSlayerWindow      *window);
static void create_menu                   (CodeSlayerWindow      *window);
static void create_projects               (CodeSlayerWindow      *window);
static void create_project_properties     (CodeSlayerWindow      *window);
static void create_side_and_bottom_pane   (CodeSlayerWindow      *window);
static void create_notebook               (CodeSlayerWindow      *window);
static void create_processes              (CodeSlayerWindow      *window);
static void create_process_bar            (CodeSlayerWindow      *window);
static void create_engines                (CodeSlayerWindow      *window);
static void create_profiles_manager       (CodeSlayerWindow      *window);
static void load_plugins                  (CodeSlayerWindow      *window);
static void create_paned_containers       (CodeSlayerWindow      *window);
static void pack_window                   (CodeSlayerWindow      *window);
static void destroy                       (CodeSlayerWindow      *window);
static gboolean delete_event              (CodeSlayerWindow      *window);
static void quit_application_action       (CodeSlayerWindow      *window);


#define CODESLAYER_WINDOW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_WINDOW_TYPE, CodeSlayerWindowPrivate))

typedef struct _CodeSlayerWindowPrivate CodeSlayerWindowPrivate;

struct _CodeSlayerWindowPrivate
{
  GtkApplication        *application;
  gchar                 *profile_name;
  CodeSlayerPreferences *preferences;
  CodeSlayerProfiles    *profiles;
  CodeSlayerProfile     *profile;
  GtkWidget             *profiles_manager;
  GtkWidget             *projects;
  GtkWidget             *project_properties;
  GtkWidget             *menubar;
  CodeSlayerEngine      *engine;
  CodeSlayerProcesses   *processes;
  CodeSlayer            *codeslayer;
  GtkWidget             *process_bar;
  GtkWidget             *notebook;
  CodeSlayerPlugins     *plugins;
  GtkWidget             *notebook_pane;
  GtkWidget             *side_pane;
  GtkWidget             *bottom_pane;
  GtkWidget             *hpaned;
  GtkWidget             *vpaned;
};

G_DEFINE_TYPE (CodeSlayerWindow, codeslayer_window, GTK_TYPE_WINDOW)

static void
codeslayer_window_class_init (CodeSlayerWindowClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_window_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerWindowPrivate));
}

static void
codeslayer_window_init (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  priv->profiles = NULL;
  priv->profile = NULL;
  priv->profiles_manager = NULL;
  priv->profile_name = NULL;
  priv->engine = NULL;
  priv->plugins = NULL;
  priv->codeslayer = NULL;
  
  gtk_window_set_default_icon_name ("codeslayer");
  
  gtk_window_set_title (GTK_WINDOW (window),
                        CODESLAYER_REGISTRY_MAIN_TITLE);
                        
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);
}

static void
codeslayer_window_finalize (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  if (priv->profile_name)
    g_free (priv->profile_name);

  if (priv->profiles)
    g_object_unref (priv->profiles);
  
  if (priv->profiles_manager)
    g_object_unref (priv->profiles_manager);
  
  if (priv->engine)
    g_object_unref (priv->engine);
  
  if (priv->plugins)
    g_object_unref (priv->plugins);
  
  if (priv->codeslayer)
    g_object_unref (priv->codeslayer);

  if (priv->profile)
    g_object_unref (priv->profile);

  G_OBJECT_CLASS (codeslayer_window_parent_class)->finalize (G_OBJECT (window));
}

/**
 * codeslayer_window_new:
 * @window: a #GtkApplication.
 *
 * Creates a new #CodeSlayerWindow.
 *
 * Returns: a new #CodeSlayerWindow. 
 */
GtkWidget*
codeslayer_window_new (GtkApplication *application, 
                       gchar          *profile_name)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *window;
  
  window = g_object_new (codeslayer_window_get_type (), "type", GTK_WINDOW_TOPLEVEL, NULL);
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  priv->application = application;
  
  if (profile_name != NULL)
    priv->profile_name = g_strdup (profile_name);

  g_signal_connect_swapped (G_OBJECT (window), "destroy", 
                            G_CALLBACK (destroy), window);
                    
  g_signal_connect_swapped (G_OBJECT (window), "delete_event",
                            G_CALLBACK (delete_event), window);
                            
  create_profiles (CODESLAYER_WINDOW (window));

  create_profiles_manager (CODESLAYER_WINDOW (window));

  create_plugins (CODESLAYER_WINDOW (window));

  create_menu (CODESLAYER_WINDOW (window));

  create_notebook (CODESLAYER_WINDOW (window));

  create_project_properties (CODESLAYER_WINDOW (window));

  create_projects (CODESLAYER_WINDOW (window));

  create_processes (CODESLAYER_WINDOW (window));
  
  create_process_bar (CODESLAYER_WINDOW (window));

  create_side_and_bottom_pane (CODESLAYER_WINDOW (window));  
  
  create_paned_containers (CODESLAYER_WINDOW (window));

  create_engines (CODESLAYER_WINDOW (window));

  pack_window (CODESLAYER_WINDOW (window));

  load_plugins (CODESLAYER_WINDOW (window));
            
  codeslayer_engine_load_profile (priv->engine);
  
  return window;
}

static void
create_profiles (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  CodeSlayerProfiles *profiles;
  CodeSlayerProfile *profile = NULL;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

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
    
  priv->profile = profile;
}

static void 
create_profiles_manager (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *profiles_manager;
    
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);
  
  profiles_manager = codeslayer_profiles_manager_new (GTK_WIDGET (window),
                                                      priv->application, 
                                                      priv->profiles, 
                                                      priv->profile);

  priv->profiles_manager = profiles_manager;
}

static void
create_plugins (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  CodeSlayerPlugins *plugins;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  plugins = codeslayer_plugins_new (GTK_WIDGET (window));
  priv->plugins = plugins;
}

static void 
create_project_properties (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *project_properties;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  project_properties = codeslayer_project_properties_new ();
  priv->project_properties = project_properties;
  
  /* take control of the reference otherwise the dialog will */
  g_object_ref_sink (priv->project_properties);
}

static void 
create_projects (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *projects;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  projects = codeslayer_projects_new (GTK_WIDGET (window),
                                      priv->profile, 
                                      priv->project_properties);
  priv->projects = projects;
}

static void 
create_notebook (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *notebook;
  GtkWidget *notebook_search;
  GtkWidget *notebook_pane;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  notebook = codeslayer_notebook_new (GTK_WINDOW (window), priv->profile);
  priv->notebook = notebook;
  
  notebook_search = codeslayer_notebook_search_new (notebook, priv->profile);
  notebook_pane = codeslayer_notebook_pane_new ();
  priv->notebook_pane = notebook_pane;
  
  codeslayer_notebook_pane_set_notebook (CODESLAYER_NOTEBOOK_PANE (notebook_pane), notebook);
  codeslayer_notebook_pane_set_notebook_search (CODESLAYER_NOTEBOOK_PANE (notebook_pane), notebook_search);
}

static void 
create_side_and_bottom_pane (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *side_pane;
  GtkWidget *bottom_pane;

  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  side_pane = codeslayer_side_pane_new (priv->profile, priv->process_bar);
  priv->side_pane = side_pane;
  
  bottom_pane = codeslayer_bottom_pane_new (priv->profile);
  priv->bottom_pane = bottom_pane;
}

static void 
create_processes (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  CodeSlayerProcesses *processes;  
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  processes = codeslayer_processes_new ();
  priv->processes = processes;
}  

static void 
create_process_bar (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *process_bar;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  process_bar = codeslayer_process_bar_new (priv->processes);
  priv->process_bar = process_bar;
}

static void 
create_engines (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  CodeSlayerEngine *engine;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  engine = codeslayer_engine_new (GTK_WINDOW (window), 
                                  priv->profile,
                                  priv->profiles,
                                  priv->profiles_manager,
                                  priv->plugins,
                                  priv->projects,
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
create_menu (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *menubar;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  menubar = codeslayer_menu_bar_new (GTK_WIDGET (window), priv->profile);
  priv->menubar = menubar;
  
  g_signal_connect_swapped (G_OBJECT (menubar), "quit-application",
                            G_CALLBACK (quit_application_action), window);
}

static void
load_plugins (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  CodeSlayer *codeslayer;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  codeslayer = codeslayer_new (GTK_WINDOW (window), 
                               priv->profile,
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
create_paned_containers (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *hpaned;
  GtkWidget *vpaned;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

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
pack_window (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  GtkWidget *vbox;
  
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);
  
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->menubar), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), priv->vpaned, TRUE, TRUE, 0);
  
  gtk_container_add (GTK_CONTAINER (window), vbox);
}

static void
destroy (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);
  gtk_widget_destroy (GTK_WIDGET (priv->project_properties));
  g_object_unref (priv->project_properties);  
}

static gboolean
delete_event (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  if (!codeslayer_engine_save_profile (priv->engine))
    return TRUE;

  codeslayer_notebook_close_all_documents (CODESLAYER_NOTEBOOK (priv->notebook));
  codeslayer_plugins_deactivate (priv->plugins);
  return FALSE;
}

static void
quit_application_action (CodeSlayerWindow *window)
{
  CodeSlayerWindowPrivate *priv;
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);

  if (!codeslayer_engine_save_profile (priv->engine))
    return;
  
  codeslayer_notebook_close_all_documents (CODESLAYER_NOTEBOOK (priv->notebook));
  codeslayer_plugins_deactivate (priv->plugins);
  gtk_widget_destroy (GTK_WIDGET (window));
}

void
codeslayer_window_open_editor (CodeSlayerWindow *window, 
                               gchar            *file_path)
{
  CodeSlayerWindowPrivate *priv;
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);
  codeslayer_engine_open_editor (priv->engine, file_path);
}

CodeSlayerProfile*  
codeslayer_window_get_profile (CodeSlayerWindow  *window)
{
  CodeSlayerWindowPrivate *priv;
  priv = CODESLAYER_WINDOW_GET_PRIVATE (window);
  return priv->profile;
}
