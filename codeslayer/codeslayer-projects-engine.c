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

#include <stdlib.h>
#include <codeslayer/codeslayer-engine.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-search.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-projects-search.h>
#include <codeslayer/codeslayer-projects-engine.h>
#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-config.h>
#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-notebook-tab.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-notebook-pane.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-plugins.h>

/**
 * SECTION:codeslayer-projects-engine
 * @short_description: Central delegation class.
 * @title: CodeSlayerProjectsEngine
 * @include: codeslayer/codeslayer-projects-engine.h
 *
 * The engine sits in the middle of the framework and delegates calls out to 
 * the rest of the API. By having such a central engine we can keep various 
 * parts of the application very decoupled.
 */

static void codeslayer_projects_engine_class_init  (CodeSlayerProjectsEngineClass *klass);
static void codeslayer_projects_engine_init        (CodeSlayerProjectsEngine      *engine);
static void codeslayer_projects_engine_finalize    (CodeSlayerProjectsEngine      *engine);

static void search_find_projects_action            (CodeSlayerProjectsEngine      *engine,
                                                    gchar                         *search_paths);
static void new_projects_action                    (CodeSlayerProjectsEngine      *engine,
                                                    gchar                         *file_name);
static void open_projects_action                   (CodeSlayerProjectsEngine      *engine,
                                                    GFile                         *file);
static void add_projects_action                    (CodeSlayerProjectsEngine      *engine,
                                                    GSList                        *files);
static void remove_project_action                  (CodeSlayerProjectsEngine      *engine,
                                                    CodeSlayerProject             *project);
static void select_projects_document_action        (CodeSlayerProjectsEngine      *engine,
                                                    CodeSlayerDocument            *document);
static void select_editor_action                   (CodeSlayerProjectsEngine      *engine, 
                                                    guint                          page_num);
static void scan_external_changes_action           (CodeSlayerProjectsEngine      *engine);
static gboolean close_search_action                (CodeSlayerProjectsEngine      *engine,
                                                    GdkEvent                      *event);
static void select_search_document_action          (CodeSlayerProjectsEngine      *engine,
                                                    CodeSlayerDocument            *document);
static void rename_file_path_action                (CodeSlayerProjectsEngine      *engine,
                                                    gchar                         *file_path,
                                                    gchar                         *renamed_file_path);
                                                   
#define CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROJECTS_ENGINE_TYPE, CodeSlayerProjectsEnginePrivate))

typedef struct _CodeSlayerProjectsEnginePrivate CodeSlayerProjectsEnginePrivate;

struct _CodeSlayerProjectsEnginePrivate
{
  GtkWindow               *window;
  CodeSlayerSettings      *settings;
  CodeSlayerPreferences   *preferences;
  CodeSlayerConfigHandler *config_handler;
  CodeSlayerPlugins       *plugins;
  GtkWidget               *search;
  GtkWidget               *menubar;
  GtkWidget               *notebook;
  GtkWidget               *notebook_pane;
  GtkWidget               *side_pane;
  GtkWidget               *bottom_pane;
  GtkWidget               *hpaned;
  GtkWidget               *vpaned;
  GdkWindowState           window_state;
  GtkWidget               *projects;

  GtkWidget               *go_to_line_dialog;
  GdkRGBA                  go_to_line_error_color;
  GdkRGBA                  go_to_line_default_color;  
};

G_DEFINE_TYPE (CodeSlayerProjectsEngine, codeslayer_projects_engine, CODESLAYER_ABSTRACT_ENGINE_TYPE)

static void
codeslayer_projects_engine_class_init (CodeSlayerProjectsEngineClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_projects_engine_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerProjectsEnginePrivate));
}

static void
codeslayer_projects_engine_init (CodeSlayerProjectsEngine *engine) 
{
  CodeSlayerProjectsEnginePrivate *priv;
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  priv->search = NULL;
}

static void
codeslayer_projects_engine_finalize (CodeSlayerProjectsEngine *engine)
{
  G_OBJECT_CLASS (codeslayer_projects_engine_parent_class)->finalize (G_OBJECT (engine));
}

/**
 * codeslayer_projects_engine_new:
 * @window: a #GtkWindow.
 * @settings: a #CodeSlayerSettings.
 * @preferences: a #CodeSlayerPreferences.
 * @plugins: a #CodeSlayerPlugins.
 * @projects: a #CodeSlayerProjects.
 * @menubar: a #CodeSlayerMenuBar.
 * @notebook_pane: a #CodeSlayerNotebookPane.
 * @side_pane: a #CodeSlayerSidePane.
 * @bottom_pane: a #CodeSlayerBottomPane.
 *
 * Creates a new #CodeSlayerProjectsEngine. There should be only one engine 
 * per application.
 *
 * Returns: a new #CodeSlayerProjectsEngine. 
 */
CodeSlayerProjectsEngine*
codeslayer_projects_engine_new (GtkWindow               *window,
                                CodeSlayerSettings      *settings,
                                CodeSlayerPreferences   *preferences,
                                CodeSlayerConfigHandler *config_handler,
                                CodeSlayerPlugins       *plugins,
                                GtkWidget               *projects, 
                                GtkWidget               *menubar,
                                GtkWidget               *notebook,
                                GtkWidget               *notebook_pane, 
                                GtkWidget               *side_pane,
                                GtkWidget               *bottom_pane, 
                                GtkWidget               *hpaned,
                                GtkWidget               *vpaned)
{
  CodeSlayerProjectsEnginePrivate *priv;
  CodeSlayerProjectsEngine *engine;

  engine = CODESLAYER_PROJECTS_ENGINE (g_object_new (codeslayer_projects_engine_get_type (), NULL));
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);

  priv->window = window;
  priv->settings = settings;
  priv->preferences = preferences;
  priv->config_handler = config_handler;
  priv->plugins = plugins;
  priv->projects = projects;
  priv->menubar = menubar;
  priv->notebook = notebook;
  priv->notebook_pane = notebook_pane;
  priv->side_pane = side_pane;
  priv->bottom_pane = bottom_pane;
  priv->hpaned = hpaned;
  priv->vpaned = vpaned;
    
  g_object_set (CODESLAYER_ABSTRACT_ENGINE (engine), 
                "window", window, 
                "settings", settings, 
                "preferences", preferences, 
                "config_handler", config_handler, 
                "menubar", menubar, 
                "notebook", notebook, 
                "notebook_pane", notebook_pane, 
                "side_pane", side_pane, 
                "bottom_pane", bottom_pane, 
                "hpaned", hpaned, 
                "vpaned", vpaned, 
                NULL);
    
  g_signal_connect_swapped (G_OBJECT (menubar), "find-projects",
                            G_CALLBACK (search_find_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "find-projects",
                            G_CALLBACK (search_find_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "new-projects",
                            G_CALLBACK (new_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "open-projects",
                            G_CALLBACK (open_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "add-projects",
                            G_CALLBACK (add_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "scan-external-changes",
                            G_CALLBACK (scan_external_changes_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "remove-project",
                            G_CALLBACK (remove_project_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "select-document",
                            G_CALLBACK (select_projects_document_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (priv->notebook), "select-editor",
                            G_CALLBACK (select_editor_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "file-path-renamed",
                            G_CALLBACK (rename_file_path_action), engine);
  
  return engine;
}

static void
new_projects_action (CodeSlayerProjectsEngine *engine,
                     gchar                    *file_name)
{
  CodeSlayerProjectsEnginePrivate *priv;
  CodeSlayerConfig *config;
  GFile *file;

  file = g_file_new_for_path (file_name);

  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  
  if (!codeslayer_abstract_engine_save_config (CODESLAYER_ABSTRACT_ENGINE (engine)))
    return;
  
  codeslayer_notebook_close_all_editors (CODESLAYER_NOTEBOOK (priv->notebook));
  codeslayer_projects_clear (CODESLAYER_PROJECTS (priv->projects));

  config = codeslayer_config_handler_load_new_config (priv->config_handler, file);
  if (config == NULL)
    return;
    
  codeslayer_config_set_projects_mode (config, TRUE);
    
  codeslayer_abstract_pane_insert (CODESLAYER_ABSTRACT_PANE (priv->side_pane), 
                                   priv->projects, "Projects", 0);
  
  g_signal_emit_by_name ((gpointer) priv->preferences, "initialize-preferences");
  
  codeslayer_abstract_engine_load_window_settings (CODESLAYER_ABSTRACT_ENGINE (engine));

  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));

  codeslayer_plugins_activate (priv->plugins, config);

  g_object_unref (file);
}

static void
open_projects_action (CodeSlayerProjectsEngine *engine,
                      GFile            *file)
{
  CodeSlayerProjectsEnginePrivate *priv;
  CodeSlayerConfig *config;
  GList *projects;
  GList *documents;
  
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  
  if (!codeslayer_abstract_engine_save_config (CODESLAYER_ABSTRACT_ENGINE (engine)))
    return;
  
  codeslayer_notebook_close_all_editors (CODESLAYER_NOTEBOOK (priv->notebook));
  codeslayer_projects_clear (CODESLAYER_PROJECTS (priv->projects));

  config = codeslayer_config_handler_load_file_config (priv->config_handler, file);
  if (config == NULL)
    return;
    
  codeslayer_config_set_projects_mode (config, TRUE);
    
  codeslayer_abstract_pane_insert (CODESLAYER_ABSTRACT_PANE (priv->side_pane), 
                                   priv->projects, "Projects", 0);
  
  g_signal_emit_by_name ((gpointer) priv->preferences, "initialize-preferences");
  
  codeslayer_abstract_engine_load_window_settings (CODESLAYER_ABSTRACT_ENGINE (engine));
 
  projects = codeslayer_config_get_projects (config);
  while (projects != NULL)
    {
      CodeSlayerProject *project = projects->data;
      codeslayer_projects_add_project (CODESLAYER_PROJECTS (priv->projects), project);
      projects = g_list_next (projects);
    }

  documents = codeslayer_config_get_documents (config);
  while (documents != NULL)
    {
      CodeSlayerDocument *document = documents->data;
      codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), 
                                           document);
      g_object_unref (document);
      /*codeslayer_notebook_add_editor (CODESLAYER_NOTEBOOK (priv->notebook), document);*/
      documents = g_list_next (documents);
    }

  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));

  codeslayer_plugins_activate (priv->plugins, config);
}

static void
add_projects_action (CodeSlayerProjectsEngine *engine,
                     GSList                   *files)
{
  CodeSlayerProjectsEnginePrivate *priv;
  CodeSlayerConfig *config;
  
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  config = codeslayer_config_handler_get_config (priv->config_handler);

  while (files != NULL)
    {
      GFile *file = files->data;

      CodeSlayerProject *project;
      gchar *project_name;
      gchar *folder_path;

      project = codeslayer_project_new ();
      
      project_name = g_file_get_basename (file);
      codeslayer_project_set_name (project, project_name);
      g_free (project_name);

      folder_path = g_file_get_path (file);
      codeslayer_project_set_folder_path (project, folder_path);
      g_free (folder_path);
      
      g_object_force_floating (G_OBJECT (project));

      codeslayer_config_add_project (config, project);

      codeslayer_config_handler_save_config (priv->config_handler);

      codeslayer_projects_add_project (CODESLAYER_PROJECTS (priv->projects), project);
      
      files = g_slist_next (files);
    }
    
  g_signal_emit_by_name ((gpointer) priv->projects, "projects-changed");
}

static void
remove_project_action (CodeSlayerProjectsEngine *engine,
                       CodeSlayerProject        *project)
{
  CodeSlayerProjectsEnginePrivate *priv;
  CodeSlayerConfig *config;
  
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  config = codeslayer_config_handler_get_config (priv->config_handler);

  codeslayer_config_remove_project (config, project);
  
  codeslayer_config_handler_save_config (priv->config_handler);
  
  g_signal_emit_by_name ((gpointer) priv->projects, "projects-changed");
}

static void
select_projects_document_action (CodeSlayerProjectsEngine *engine,
                                 CodeSlayerDocument       *document)
{
  CodeSlayerProjectsEnginePrivate *priv;
  GtkWidget *notebook;
  const gchar *file_path;
  GFile *file;
  gint pages;
  gint page;

  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  
  notebook = priv->notebook;
  file_path = codeslayer_document_get_file_path (document);

  /* make sure the file still exists */
  
  file = g_file_new_for_path (file_path);
  if (!g_file_query_exists (file, NULL))
    {
      g_object_unref (file);
      return;
    }
  g_object_unref (file);

  /* make sure the page is not already part of the notebook */
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page;
      CodeSlayerDocument *current_document;
      const gchar *current_file_path;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
      current_document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      current_file_path = codeslayer_document_get_file_path (current_document);

      if (g_strcmp0 (current_file_path, file_path) == 0)
        {
          gint line_number;
        
          gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);

          line_number = codeslayer_document_get_line_number (document);
          if (line_number > 0)
            {
              GtkWidget *editor;
              editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
              codeslayer_editor_scroll_to_line (CODESLAYER_EDITOR (editor), line_number);
            }

          return;
        }
    }

  /* all passed so add to the notebook */

  codeslayer_notebook_add_editor (CODESLAYER_NOTEBOOK (notebook), document);

  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));
}

static void
select_editor_action (CodeSlayerProjectsEngine *engine, 
                      guint                     page_num)
{
  CodeSlayerProjectsEnginePrivate *priv;
  CodeSlayerConfig *config;
  GtkWidget *notebook_page;
  CodeSlayerDocument *document;
  gboolean sync_with_editor;
  
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  config = codeslayer_config_handler_get_config (priv->config_handler);
  
  if (codeslayer_config_get_projects (config) == NULL)
    return;

  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page_num);
  if (notebook_page == NULL)
    return;
  
  document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (notebook_page));

  sync_with_editor = codeslayer_settings_get_boolean (priv->settings, 
                                                      CODESLAYER_SETTINGS_SYNC_WITH_EDITOR);
  
  if (sync_with_editor)
    {
      if (!codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), document))
        codeslayer_notebook_page_show_document_not_found_info_bar (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
    }
}

static void
search_find_projects_action (CodeSlayerProjectsEngine *engine,
                             gchar                    *file_paths)
{
  CodeSlayerProjectsEnginePrivate *priv;
  CodeSlayerConfig *config;
  gint search_width;
  gint search_height;
  gint search_x;
  gint search_y;
  
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  config = codeslayer_config_handler_get_config (priv->config_handler);
  
  if (priv->search == NULL)
    {
      priv->search = codeslayer_projects_search_new (priv->window, 
                                                     priv->preferences);

      g_signal_connect_swapped (G_OBJECT (priv->search), "close",
                                G_CALLBACK (close_search_action), engine);
                        
      g_signal_connect_swapped (G_OBJECT (priv->search), "select-document",
                                G_CALLBACK (select_search_document_action), engine);
    }
    
  if (!gtk_widget_get_visible (priv->search))
    {
      search_width = codeslayer_settings_get_integer (priv->settings,
                                                      CODESLAYER_SETTINGS_SEARCH_WIDTH);
      if (search_width < 0)
        search_width = 600;
        
      search_height = codeslayer_settings_get_integer (priv->settings,
                                                       CODESLAYER_SETTINGS_SEARCH_HEIGHT);
      if (search_height < 0)
        search_height = 350;
        
      gtk_window_set_default_size (GTK_WINDOW (priv->search), search_width, search_height);

      search_x = codeslayer_settings_get_integer (priv->settings,
                                                  CODESLAYER_SETTINGS_SEARCH_X);
      if (search_x < 0)
        search_x = 10;
        
      search_y = codeslayer_settings_get_integer (priv->settings,
                                                  CODESLAYER_SETTINGS_SEARCH_Y);
      if (search_y < 0)
        search_y = 10;

      gtk_window_move (GTK_WINDOW (priv->search), search_x, search_y);
    }
    
  gtk_window_present (GTK_WINDOW (priv->search));
  gtk_widget_show_all (GTK_WIDGET (priv->search));
  
  codeslayer_projects_search_set_config (CODESLAYER_PROJECTS_SEARCH (priv->search), config);
  
  if (codeslayer_utils_has_text (file_paths))
    codeslayer_projects_search_find_selection (CODESLAYER_PROJECTS_SEARCH (priv->search), file_paths);
  else
    codeslayer_projects_search_find_projects (CODESLAYER_PROJECTS_SEARCH (priv->search));
}

static gboolean
close_search_action (CodeSlayerProjectsEngine *engine, 
                     GdkEvent                 *event)
{
  CodeSlayerProjectsEnginePrivate *priv; 
  gint width;
  gint height;
  gint x;
  gint y;

  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);

  gtk_window_get_size (GTK_WINDOW (priv->search), &width, &height);
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_SEARCH_WIDTH,
                                   width);
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_SEARCH_HEIGHT,
                                   height);

  gtk_window_get_position (GTK_WINDOW (priv->search), &x, &y);
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_SEARCH_X, x);
  codeslayer_settings_set_integer (priv->settings,
                                   CODESLAYER_SETTINGS_SEARCH_Y, y);

  gtk_widget_hide (priv->search);

  return TRUE;
}

static void
select_search_document_action (CodeSlayerProjectsEngine   *engine,
                               CodeSlayerDocument         *document)
{
  CodeSlayerProjectsEnginePrivate *priv;
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);
  codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), 
                                       document);
}

static void
scan_external_changes_action (CodeSlayerProjectsEngine *engine)
{
  CodeSlayerProjectsEnginePrivate *priv;
  gint pages;
  gint page;
  
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);

  codeslayer_projects_refresh (CODESLAYER_PROJECTS (priv->projects));
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));

  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page; 
      GtkWidget *editor;
      const gchar *file_path;
      GTimeVal *original_modification_time;
      GTimeVal *latest_modification_time;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      
      original_modification_time = codeslayer_editor_get_modification_time (CODESLAYER_EDITOR (editor));

      file_path = codeslayer_editor_get_file_path (CODESLAYER_EDITOR (editor));
      latest_modification_time = codeslayer_utils_get_modification_time (file_path);
      
      if (latest_modification_time->tv_sec > original_modification_time->tv_sec)
        codeslayer_notebook_page_show_external_changes_info_bar (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
        
      g_free (latest_modification_time);
    }

  g_signal_emit_by_name ((gpointer) priv->projects, "projects-changed");
}

static void
rename_file_path_action (CodeSlayerProjectsEngine *engine, 
                         gchar                    *file_path,
                         gchar                    *renamed_file_path)
{
  CodeSlayerProjectsEnginePrivate *priv;
  gint pages;
  gint page;
  
  priv = CODESLAYER_PROJECTS_ENGINE_GET_PRIVATE (engine);

  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));

  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page;
      CodeSlayerDocument *current_document;
      const gchar *current_file_path;
      guint length;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      current_document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      current_file_path = codeslayer_document_get_file_path (current_document);

      length = g_strv_length (&file_path);

      if (g_ascii_strncasecmp (current_file_path, file_path, length) == 0)
        {
          gchar *replacement_file_path;
          gchar *replacement_basename;
          GtkWidget *notebook_label;
          
          replacement_file_path = codeslayer_utils_strreplace (current_file_path, 
                                                               file_path,
                                                               renamed_file_path);
          codeslayer_document_set_file_path (current_document,
                                             replacement_file_path);

          replacement_basename = g_path_get_basename (replacement_file_path);

          notebook_label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (priv->notebook),
                                                       notebook_page);
          codeslayer_notebook_tab_set_label_name (CODESLAYER_NOTEBOOK_TAB (notebook_label),
                                                  replacement_basename);

          g_free (replacement_file_path);
          g_free (replacement_basename);
        }
    }
}
