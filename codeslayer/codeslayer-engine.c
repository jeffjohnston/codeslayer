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

#include <codeslayer/codeslayer-engine.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-search.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-repository.h>
#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-notebook-tab.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-notebook-pane.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-plugins.h>

/**
 * SECTION:codeslayer-engine
 * @short_description: Central delegation class.
 * @title: CodeSlayerEngine
 * @include: codeslayer/codeslayer-engine.h
 *
 * The engine sits in the middle of the framework and delegates calls out to 
 * the rest of the API. For instance the "group-changed" menu command will 
 * send a signal to the engine, which in turn invokes the correct functions on 
 * the projects tree and notebook to load up the requested group. By having  
 * such a central engine we can keep various parts of the application very 
 * decoupled.
 */

static void codeslayer_engine_class_init       (CodeSlayerEngineClass  *klass);
static void codeslayer_engine_init             (CodeSlayerEngine       *engine);
static void codeslayer_engine_finalize         (CodeSlayerEngine       *engine);

static void apply_preferences                  (CodeSlayerEngine       *engine, 
                                                CodeSlayerPreferences  *preferences);
static void group_changed_action               (CodeSlayerEngine       *engine, 
                                                gchar                  *group_name);
static void new_group_action                   (CodeSlayerEngine       *engine,
                                                gchar                  *group_name);
static void rename_group_action                (CodeSlayerEngine       *engine,  
                                                gchar                  *group_name);
static void remove_group_action                (CodeSlayerEngine       *engine);
static void save_editor_action                 (CodeSlayerEngine       *engine);
static void save_all_editors_action            (CodeSlayerEngine       *engine);
static void close_editor_action                (CodeSlayerEngine       *engine);
static void search_find_action                 (CodeSlayerEngine       *engine);
static void search_find_next_action            (CodeSlayerEngine       *engine);
static void search_find_previous_action        (CodeSlayerEngine       *engine);
static void search_find_incremental_action     (CodeSlayerEngine       *engine);
static void search_replace_action              (CodeSlayerEngine       *engine);
static void search_find_projects_action        (CodeSlayerEngine       *engine,
                                                gchar                  *search_paths);
static void fullscreen_window_action           (CodeSlayerEngine       *engine);
static void toggle_side_pane_action            (CodeSlayerEngine       *engine);
static void open_side_pane_action              (CodeSlayerEngine       *engine);
static void close_side_pane_action             (CodeSlayerEngine       *engine);
static void toggle_bottom_pane_action          (CodeSlayerEngine       *engine);
static void open_bottom_pane_action            (CodeSlayerEngine       *engine);
static void close_bottom_pane_action           (CodeSlayerEngine       *engine);
static void draw_spaces_action                 (CodeSlayerEngine       *engine);
static void sync_projects_with_editor_action   (CodeSlayerEngine       *engine, 
                                                gboolean                sync_projects_with_editor);
static void add_project_action                 (CodeSlayerEngine       *engine,
                                                GFile                  *file);
static void remove_project_action              (CodeSlayerEngine       *engine,
                                                CodeSlayerProject      *project);
static void project_modified_action            (CodeSlayerEngine       *engine,
                                                CodeSlayerProject      *project);
static void add_page_action                    (CodeSlayerEngine       *engine,
                                                CodeSlayerDocument     *document);
static void rename_file_path_action            (CodeSlayerEngine       *engine,
                                                gchar                  *file_path,
                                                gchar                  *renamed_file_path);
static void switch_page_action                 (CodeSlayerEngine       *engine,
                                                GtkNotebookPage        *page, 
                                                guint                   page_num);
static void page_removed_action                (CodeSlayerEngine       *engine,
                                                GtkNotebookPage        *page, 
                                                guint                   page_num);
static void show_preferences_action            (CodeSlayerEngine       *engine);
static void show_plugins_action                (CodeSlayerEngine       *engine);
static gboolean close_search_action            (CodeSlayerEngine       *engine,
                                                GdkEvent               *event);
static void open_document_action               (CodeSlayerEngine       *engine,
                                                CodeSlayerDocument     *document);
static void editor_preferences_changed_action  (CodeSlayerEngine       *engine);

static void notify_visible_pane_action         (CodeSlayerEngine       *engine,
                                                GParamSpec             *spec);
                                                   
#define CODESLAYER_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_ENGINE_TYPE, CodeSlayerEnginePrivate))

typedef struct _CodeSlayerEnginePrivate CodeSlayerEnginePrivate;

struct _CodeSlayerEnginePrivate
{
  GtkWindow             *window;
  CodeSlayerPreferences *preferences;
  CodeSlayerPlugins     *plugins;
  GtkWidget             *search;
  GtkWidget             *projects;
  GtkWidget             *menubar;
  GtkWidget             *notebook;
  GtkWidget             *notebook_pane;
  GtkWidget             *side_pane;
  GtkWidget             *bottom_pane;
  CodeSlayerGroups      *groups;
  GdkWindowState         window_state;
  gboolean               sync_projects_with_editor;
};

G_DEFINE_TYPE (CodeSlayerEngine, codeslayer_engine, G_TYPE_OBJECT)

static void
codeslayer_engine_class_init (CodeSlayerEngineClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_engine_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerEnginePrivate));
}

static void
codeslayer_engine_init (CodeSlayerEngine *engine) {}

static void
codeslayer_engine_finalize (CodeSlayerEngine *engine)
{
  G_OBJECT_CLASS (codeslayer_engine_parent_class)->finalize (G_OBJECT (engine));
}


/**
 * codeslayer_engine_new:
 * @window: a #GtkWindow.
 * @preferences: a #CodeSlayerPreferences.
 * @plugins: a #CodeSlayerPlugins.
 * @groups: a #CodeSlayerGroups.
 * @projects: a #CodeSlayerProjects.
 * @menubar: a #CodeSlayerMenuBar.
 * @notebook_pane: a #CodeSlayerNotebookPane.
 * @side_pane: a #CodeSlayerSidePane.
 * @bottom_pane: a #CodeSlayerBottomPane.
 *
 * Creates a new #CodeSlayerEngine. There should be only one engine 
 * per application.
 *
 * Returns: a new #CodeSlayerEngine. 
 */
CodeSlayerEngine*
codeslayer_engine_new (GtkWindow             *window,
                       CodeSlayerPreferences *preferences,
                       CodeSlayerPlugins     *plugins,
                       CodeSlayerGroups      *groups,
                       GtkWidget             *projects, 
                       GtkWidget             *menubar,
                       GtkWidget             *notebook_pane, 
                       GtkWidget             *side_pane,
                       GtkWidget             *bottom_pane)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerEngine *engine;

  engine = CODESLAYER_ENGINE (g_object_new (codeslayer_engine_get_type (), NULL));
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  priv->window = window;
  priv->preferences = preferences;
  priv->plugins = plugins;
  priv->groups = groups;
  priv->projects = projects;
  priv->menubar = menubar;
  priv->notebook = codeslayer_notebook_pane_get_notebook (CODESLAYER_NOTEBOOK_PANE (notebook_pane));
  priv->notebook_pane = notebook_pane;
  priv->side_pane = side_pane;
  priv->bottom_pane = bottom_pane;
  priv->search = NULL;
  
  apply_preferences (CODESLAYER_ENGINE (engine), preferences);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "group-changed",
                            G_CALLBACK (group_changed_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "new-group",
                            G_CALLBACK (new_group_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "rename-group",
                            G_CALLBACK (rename_group_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "remove-group",
                            G_CALLBACK (remove_group_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "save-editor",
                            G_CALLBACK (save_editor_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "save-all-editors",
                            G_CALLBACK (save_all_editors_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "close-editor",
                            G_CALLBACK (close_editor_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "find",
                            G_CALLBACK (search_find_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "replace",
                            G_CALLBACK (search_replace_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "find-next",
                            G_CALLBACK (search_find_next_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "find-previous",
                            G_CALLBACK (search_find_previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "find-incremental",
                            G_CALLBACK (search_find_incremental_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "find-projects",
                            G_CALLBACK (search_find_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "find-projects",
                            G_CALLBACK (search_find_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "fullscreen-window",
                            G_CALLBACK (fullscreen_window_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "show-side-pane",
                            G_CALLBACK (toggle_side_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "show-bottom-pane",
                            G_CALLBACK (toggle_bottom_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (side_pane), "open-pane",
                            G_CALLBACK (open_side_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (side_pane), "close-pane",
                            G_CALLBACK (close_side_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (bottom_pane), "open-pane",
                            G_CALLBACK (open_bottom_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (bottom_pane), "close-pane",
                            G_CALLBACK (close_bottom_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "draw-spaces",
                            G_CALLBACK (draw_spaces_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "add-project",
                            G_CALLBACK (add_project_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "show-preferences",
                            G_CALLBACK (show_preferences_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "show-plugins",
                            G_CALLBACK (show_plugins_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "sync-projects-with-editor",
                            G_CALLBACK (sync_projects_with_editor_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "remove-project",
                            G_CALLBACK (remove_project_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "project-modified",
                            G_CALLBACK (project_modified_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "open-document",
                            G_CALLBACK (add_page_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "file-path-renamed",
                            G_CALLBACK (rename_file_path_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (priv->notebook), "switch-page",
                            G_CALLBACK (switch_page_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (priv->notebook), "page-removed",
                            G_CALLBACK (page_removed_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (preferences), "editor-settings-changed",
                            G_CALLBACK (editor_preferences_changed_action), engine);
                    
  g_signal_connect_swapped (G_OBJECT (priv->side_pane), "notify::visible",
                            G_CALLBACK (notify_visible_pane_action), engine);

  g_signal_connect_swapped (G_OBJECT (priv->bottom_pane), "notify::visible",
                            G_CALLBACK (notify_visible_pane_action), engine);

  return engine;
}

static void
apply_preferences (CodeSlayerEngine      *engine, 
                   CodeSlayerPreferences *preferences)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  priv->sync_projects_with_editor = codeslayer_preferences_get_boolean (preferences, 
                                                                        CODESLAYER_PREFERENCES_PROJECTS_SYNC_WITH_EDITOR);
}

/**
 * codeslayer_engine_close_active_group:
 * @engine: a #CodeSlayerEngine.
 *
 * Close the current active #CodeSlayerGroup.
 *
 * Returns: TRUE if the active group closed successfully.
 */
gboolean
codeslayer_engine_close_active_group (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  gint pages;
  CodeSlayerGroup *active_group;
  GList *documents = NULL;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  notebook = priv->notebook;
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page;
      CodeSlayerDocument *document;
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
      document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      documents = g_list_append (documents, document);
    }
  
  active_group = codeslayer_groups_get_active_group (priv->groups);
  codeslayer_repository_save_documents (active_group, documents);
  g_list_free (documents);

  return codeslayer_notebook_close_all_editors (CODESLAYER_NOTEBOOK (notebook));
}

/**
 * codeslayer_engine_open_active_group:
 * @engine: a #CodeSlayerEngine.
 *
 * Open the #CodeSlayerGroup marked as active.
 */
void
codeslayer_engine_open_active_group (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerGroup *active_group;
  GList *documents;
  GList *tmp;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  active_group = codeslayer_groups_get_active_group (priv->groups);
  
  if (codeslayer_group_get_libs (active_group) == NULL)
    {
      GList *libs;
      libs = codeslayer_repository_get_libs (active_group);
      codeslayer_group_set_libs (active_group, libs);
    }
  codeslayer_plugins_activate (priv->plugins, active_group);
  
  if (codeslayer_group_get_projects (active_group) == NULL)
    {
      GList* projects;
      projects = codeslayer_repository_get_projects (active_group);
      codeslayer_group_set_projects (active_group, projects);
    }
  
  documents = codeslayer_repository_get_documents (active_group);
  documents = g_list_reverse (documents);

  codeslayer_projects_load_group (CODESLAYER_PROJECTS (priv->projects), active_group);
  
  tmp = documents;

  while (tmp != NULL)
    {
      CodeSlayerDocument *document = tmp->data;
      codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), 
                                           document);
      tmp = g_list_next (tmp);
    }

  g_list_free (documents);

  codeslayer_menubar_sync_with_notebook (CODESLAYER_MENUBAR (priv->menubar), priv->notebook);
  codeslayer_notebook_pane_sync_with_notebook (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));
}

static void
group_changed_action (CodeSlayerEngine *engine,
                      gchar            *group_name)
{
  CodeSlayerEnginePrivate *priv;
  GList *list;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  if (!codeslayer_engine_close_active_group (engine))
    return;

  list = codeslayer_groups_get_list (priv->groups);
  while (list != NULL)
    {
      CodeSlayerGroup *group = list->data;
      if (g_strcmp0 (group_name, codeslayer_group_get_name (group)) == 0)
        {
          codeslayer_groups_set_active_group (priv->groups, group);
          codeslayer_repository_save_groups (priv->groups);
          codeslayer_menubar_refresh_groups (CODESLAYER_MENUBAR (priv->menubar), priv->groups);
          if (priv->search != NULL)
            codeslayer_search_clear (CODESLAYER_SEARCH (priv->search));
          codeslayer_engine_open_active_group (engine);
          return;
        }
      list = g_list_next (list);
    }
}

static void
new_group_action (CodeSlayerEngine *engine, 
                  gchar            *group_name)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerGroup *group;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  group = codeslayer_group_new ();

  codeslayer_group_set_name (group, group_name);
  
  g_object_force_floating (G_OBJECT (group));

  codeslayer_repository_create_group (group);

  if (!codeslayer_engine_close_active_group (engine))
    return;

  codeslayer_groups_set_active_group (priv->groups, group);
  codeslayer_groups_add_group (priv->groups, group);

  codeslayer_menubar_refresh_groups (CODESLAYER_MENUBAR (priv->menubar), priv->groups);
  codeslayer_projects_load_group (CODESLAYER_PROJECTS (priv->projects), group);
}

static void
rename_group_action (CodeSlayerEngine *engine,
                     gchar            *group_name)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerGroup *active_group;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  active_group = codeslayer_groups_get_active_group (priv->groups);

  codeslayer_repository_rename_group (active_group, group_name);
  codeslayer_group_set_name (active_group, group_name);

  codeslayer_menubar_refresh_groups (CODESLAYER_MENUBAR (priv->menubar), priv->groups);
}

static void
remove_group_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerGroup *active_group;
  const gchar *group_name;
  CodeSlayerGroup *next_group;
    
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  active_group = codeslayer_groups_get_active_group (priv->groups);
  group_name = codeslayer_group_get_name (active_group);
  next_group = codeslayer_groups_find_next_group (priv->groups, group_name);

  codeslayer_repository_delete_group (active_group);

  codeslayer_notebook_close_all_editors (CODESLAYER_NOTEBOOK (priv->notebook));

  codeslayer_groups_remove_group (priv->groups, active_group);
  codeslayer_groups_set_active_group (priv->groups, next_group);

  codeslayer_menubar_refresh_groups (CODESLAYER_MENUBAR (priv->menubar), priv->groups);

  codeslayer_engine_open_active_group (engine);
}

static void
add_project_action (CodeSlayerEngine *engine,
                    GFile            *file)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerProject *project;
  gchar *project_name;
  gchar *project_key;
  gchar *folder_path;
  CodeSlayerGroup *active_group;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  project = codeslayer_project_new ();
  
  project_name = g_file_get_basename (file);
  codeslayer_project_set_name (project, project_name);
  g_free (project_name);

  project_key = codeslayer_utils_create_key();
  codeslayer_project_set_key (project, project_key);
  g_free (project_key);

  folder_path = g_file_get_path (file);
  codeslayer_project_set_folder_path (project, folder_path);
  g_free (folder_path);
  
  g_object_force_floating (G_OBJECT (project));

  active_group = codeslayer_groups_get_active_group (priv->groups);

  codeslayer_group_add_project (active_group, project);

  codeslayer_repository_save_projects (active_group);

  codeslayer_projects_add_project (CODESLAYER_PROJECTS (priv->projects), project);
}

static void
remove_project_action (CodeSlayerEngine  *engine,
                       CodeSlayerProject *project)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerGroup *active_group;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  active_group = codeslayer_groups_get_active_group (priv->groups);

  codeslayer_group_remove_project (active_group, project);
  codeslayer_repository_save_projects (active_group);
}

static void
project_modified_action (CodeSlayerEngine  *engine,
                         CodeSlayerProject *project)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerGroup *active_group;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  active_group = codeslayer_groups_get_active_group (priv->groups);
  
  codeslayer_repository_save_projects (active_group);
}

static void
add_page_action (CodeSlayerEngine   *engine,
                 CodeSlayerDocument *document)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  const gchar *file_path;
  GFile *file;
  gint pages;
  gint page;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

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
  codeslayer_menubar_sync_with_notebook (CODESLAYER_MENUBAR (priv->menubar), priv->notebook);
  codeslayer_notebook_pane_sync_with_notebook (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));  
}

static void
save_editor_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  notebook = priv->notebook;
  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  codeslayer_notebook_save_editor (CODESLAYER_NOTEBOOK (notebook), page);
}

static void
save_all_editors_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  notebook = priv->notebook;
  codeslayer_notebook_save_all_editors (CODESLAYER_NOTEBOOK (notebook));
}

static void
close_editor_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  notebook = priv->notebook;
  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  codeslayer_notebook_close_editor (CODESLAYER_NOTEBOOK (notebook), page);
}

static void
switch_page_action (CodeSlayerEngine *engine, 
                    GtkNotebookPage  *page,
                    guint             page_num)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook_page;
  CodeSlayerDocument *document;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page_num);
  document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
  
  if (priv->sync_projects_with_editor)
    {
      if (!codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), document))
        codeslayer_notebook_page_show_document_not_found_info_bar (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
    }
}

static void
page_removed_action (CodeSlayerEngine *engine, 
                     GtkNotebookPage  *page,
                     guint             removed_page_num)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_menubar_sync_with_notebook (CODESLAYER_MENUBAR (priv->menubar), priv->notebook);
  codeslayer_notebook_pane_sync_with_notebook (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));  
}

static void
search_find_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  if (gtk_widget_get_ancestor (gtk_window_get_focus (priv->window), CODESLAYER_PROJECTS_TYPE))
    codeslayer_projects_search_find (CODESLAYER_PROJECTS (priv->projects));
  else 
    codeslayer_notebook_pane_search_find (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));
}

static void
search_replace_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_notebook_pane_search_replace (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));
}

static void
search_find_next_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_notebook_pane_search_find_next (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));
}

static void
search_find_previous_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_notebook_pane_search_find_previous (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));
}

static void
search_find_incremental_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_notebook_pane_search_find_incremental (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));
}

static void
search_find_projects_action (CodeSlayerEngine *engine,
                             gchar            *file_paths)
{
  CodeSlayerEnginePrivate *priv;
  gint search_width;
  gint search_height;
  gint search_x;
  gint search_y;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  if (!priv->search)
    {
      priv->search = codeslayer_search_new (priv->window, 
                                            priv->preferences, 
                                            priv->groups);

      gtk_window_set_type_hint (GTK_WINDOW (priv->search), GDK_WINDOW_TYPE_HINT_DIALOG);

      g_signal_connect_swapped (G_OBJECT (priv->search), "delete_event",
                                G_CALLBACK (close_search_action), engine);
                        
      g_signal_connect_swapped (G_OBJECT (priv->search), "select-document",
                                G_CALLBACK (open_document_action), engine);
    }

  if (!gtk_widget_get_visible (priv->search))
    {
      search_width = codeslayer_preferences_get_integer (priv->preferences,
                                                         CODESLAYER_PREFERENCES_SEARCH_WIDTH);
      if (search_width < 0)
        search_width = 600;
        
      search_height = codeslayer_preferences_get_integer (priv->preferences,
                                                          CODESLAYER_PREFERENCES_SEARCH_HEIGHT);
      if (search_height < 0)
        search_height = 350;
        
      gtk_window_set_default_size (GTK_WINDOW (priv->search), search_width, search_height);

      search_x = codeslayer_preferences_get_integer (priv->preferences,
                                                     CODESLAYER_PREFERENCES_SEARCH_X);
      if (search_x < 0)
        search_x = 10;
        
      search_y = codeslayer_preferences_get_integer (priv->preferences,
                                                     CODESLAYER_PREFERENCES_SEARCH_Y);
      if (search_y < 0)
        search_y = 10;        

      gtk_window_move (GTK_WINDOW (priv->search), search_x, search_y);
    }
    
  gtk_window_present (GTK_WINDOW (priv->search));
  gtk_widget_show_all (GTK_WIDGET (priv->search));

  if (file_paths)
    codeslayer_search_add_page (CODESLAYER_SEARCH (priv->search), file_paths);
  else
    codeslayer_search_default_page (CODESLAYER_SEARCH (priv->search));    
}

static gboolean
close_search_action (CodeSlayerEngine *engine, 
                     GdkEvent         *event)
{
  CodeSlayerEnginePrivate *priv; 
  gint width;
  gint height;
  gint x;
  gint y;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  gtk_window_get_size (GTK_WINDOW (priv->search), &width, &height);
  codeslayer_preferences_set_integer (priv->preferences,
                                      CODESLAYER_PREFERENCES_SEARCH_WIDTH,
                                      width);
  codeslayer_preferences_set_integer (priv->preferences,
                                      CODESLAYER_PREFERENCES_SEARCH_HEIGHT,
                                      height);

  gtk_window_get_position (GTK_WINDOW (priv->search), &x, &y);
  codeslayer_preferences_set_integer (priv->preferences,
                                      CODESLAYER_PREFERENCES_SEARCH_X, x);
  codeslayer_preferences_set_integer (priv->preferences,
                                      CODESLAYER_PREFERENCES_SEARCH_Y, y);

  codeslayer_preferences_save (priv->preferences);
  
  gtk_widget_hide (priv->search);

  return TRUE;
}

static void
open_document_action (CodeSlayerEngine   *engine,
                      CodeSlayerDocument *document)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), 
                                            document);
}

static void
fullscreen_window_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  if (priv->window_state != GDK_WINDOW_STATE_FULLSCREEN)
    {
      gtk_window_fullscreen (priv->window);
      priv->window_state = GDK_WINDOW_STATE_FULLSCREEN;
    }
  else
    {
      gtk_window_unfullscreen (priv->window);
      priv->window_state = 0;
    }
}

static void
toggle_side_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  if (gtk_widget_get_visible (GTK_WIDGET(priv->side_pane)))
    {
      gtk_widget_hide (GTK_WIDGET(priv->side_pane));
      codeslayer_preferences_set_boolean (priv->preferences, 
                                          CODESLAYER_PREFERENCES_SIDE_PANE_VISIBLE,
                                          FALSE);
    }
  else
    {
      gtk_widget_show (GTK_WIDGET(priv->side_pane));
      codeslayer_preferences_set_boolean (priv->preferences, 
                                          CODESLAYER_PREFERENCES_SIDE_PANE_VISIBLE,
                                          TRUE);
    }
}

static void
open_side_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  gtk_widget_show (GTK_WIDGET(priv->side_pane));
  codeslayer_preferences_set_boolean (priv->preferences, 
                                      CODESLAYER_PREFERENCES_SIDE_PANE_VISIBLE,
                                      TRUE);
}

static void
close_side_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  gtk_widget_hide (GTK_WIDGET(priv->side_pane));
  codeslayer_preferences_set_boolean (priv->preferences, 
                                      CODESLAYER_PREFERENCES_SIDE_PANE_VISIBLE,
                                      FALSE);
}

static void
toggle_bottom_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  if (gtk_widget_get_visible (GTK_WIDGET(priv->bottom_pane)))
    {
      gtk_widget_hide (GTK_WIDGET(priv->bottom_pane));
      codeslayer_preferences_set_boolean (priv->preferences, 
                                          CODESLAYER_PREFERENCES_BOTTOM_PANE_VISIBLE,
                                          FALSE);
    }
  else
    {
      gtk_widget_show (GTK_WIDGET(priv->bottom_pane));
      codeslayer_preferences_set_boolean (priv->preferences, 
                                          CODESLAYER_PREFERENCES_BOTTOM_PANE_VISIBLE,
                                          TRUE);
    }
}

static void
open_bottom_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  gtk_widget_show (GTK_WIDGET(priv->bottom_pane));
  codeslayer_preferences_set_boolean (priv->preferences, 
                                      CODESLAYER_PREFERENCES_BOTTOM_PANE_VISIBLE,
                                      TRUE);
}

static void
close_bottom_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  gtk_widget_hide (GTK_WIDGET(priv->bottom_pane));
  codeslayer_preferences_set_boolean (priv->preferences, 
                                      CODESLAYER_PREFERENCES_BOTTOM_PANE_VISIBLE,
                                      FALSE);
}

static void
draw_spaces_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  gboolean draw_spaces;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  draw_spaces = codeslayer_preferences_get_boolean (priv->preferences, 
                                                    CODESLAYER_PREFERENCES_EDITOR_DRAW_SPACES);
  if (draw_spaces)
    {
      codeslayer_preferences_set_boolean (priv->preferences, 
                                          CODESLAYER_PREFERENCES_EDITOR_DRAW_SPACES,
                                          FALSE);
      editor_preferences_changed_action (engine);
    }
  else
    {
      codeslayer_preferences_set_boolean (priv->preferences, 
                                          CODESLAYER_PREFERENCES_EDITOR_DRAW_SPACES,
                                          TRUE);
      editor_preferences_changed_action (engine);
    }
}

static void
sync_projects_with_editor_action (CodeSlayerEngine *engine, 
                                  gboolean          sync_projects_with_editor)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  priv->sync_projects_with_editor = sync_projects_with_editor;
  codeslayer_preferences_set_boolean (priv->preferences, 
                                      CODESLAYER_PREFERENCES_PROJECTS_SYNC_WITH_EDITOR,
                                      sync_projects_with_editor);
}

static void
show_preferences_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_preferences_run_dialog (priv->preferences);
}

static void
show_plugins_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerGroup *active_group;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  active_group = codeslayer_groups_get_active_group (priv->groups);
  codeslayer_plugins_run_dialog (priv->plugins, active_group);
}

static void
rename_file_path_action (CodeSlayerEngine *engine, 
                         gchar            *file_path,
                         gchar            *renamed_file_path)
{
  CodeSlayerEnginePrivate *priv;
  gint pages;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

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

static void
editor_preferences_changed_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv; 
  gint pages;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));

  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page; 
      GtkWidget *editor;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      codeslayer_editor_sync_preferences (CODESLAYER_EDITOR (editor));
    }
}

static void
notify_visible_pane_action (CodeSlayerEngine *engine,
                            GParamSpec       *spec)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_menubar_sync_with_panes (CODESLAYER_MENUBAR (priv->menubar), 
                                      gtk_widget_get_visible (priv->side_pane), 
                                      gtk_widget_get_visible (priv->bottom_pane));
}
