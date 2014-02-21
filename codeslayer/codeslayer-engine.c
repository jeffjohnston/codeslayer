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
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-projects-search.h>
#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-profiles-manager.h>
#include <codeslayer/codeslayer-profile.h>
#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-notebook-tab.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-notebook-pane.h>
#include <codeslayer/codeslayer-regexview.h>
#include <codeslayer/codeslayer-sourceview.h>

/**
 * SECTION:codeslayer-engine
 * @short_description: Central delegation class.
 * @title: CodeSlayerEngine
 * @include: codeslayer/codeslayer-engine.h
 *
 * The engine sits in the middle of the framework and delegates calls out to 
 * the rest of the API. By having such a central engine we can keep various 
 * parts of the application very decoupled.
 */

static void codeslayer_engine_class_init    (CodeSlayerEngineClass *klass);
static void codeslayer_engine_init          (CodeSlayerEngine      *engine);
static void codeslayer_engine_finalize      (CodeSlayerEngine      *engine);

static void close_default_document            (CodeSlayerEngine      *engine);
static void new_document_action             (CodeSlayerEngine      *engine);
static void open_document_action            (CodeSlayerEngine      *engine);
static void save_document_action            (CodeSlayerEngine      *engine);
static void save_all_documents_action       (CodeSlayerEngine      *engine);
static void close_document_action           (CodeSlayerEngine      *engine);
static void regular_expression_action       (CodeSlayerEngine      *engine);
static void go_to_line_action               (CodeSlayerEngine      *engine);
static gboolean go_to_line_keypress_action  (GtkWidget             *entry,
                                             GdkEventKey           *event, 
                                             CodeSlayerEngine      *engine);
static void fullscreen_window_action        (CodeSlayerEngine      *engine);
static void toggle_side_pane_action         (CodeSlayerEngine      *engine);
static void open_side_pane_action           (CodeSlayerEngine      *engine);
static void close_side_pane_action          (CodeSlayerEngine      *engine);
static void toggle_bottom_pane_action       (CodeSlayerEngine      *engine);
static void open_bottom_pane_action         (CodeSlayerEngine      *engine);
static void close_bottom_pane_action        (CodeSlayerEngine      *engine);
static void draw_spaces_action              (CodeSlayerEngine      *engine);
static void word_wrap_action                (CodeSlayerEngine      *engine);
static void page_removed_action             (CodeSlayerEngine      *engine,
                                             GtkWidget             *page, 
                                             guint                  page_num);
static void show_preferences_action         (CodeSlayerEngine      *engine);
static void registry_changed_action         (CodeSlayerEngine      *engine);
static void show_plugins_action             (CodeSlayerEngine      *engine);
static void search_find_action              (CodeSlayerEngine      *engine);
static void search_find_next_action         (CodeSlayerEngine      *engine);
static void search_find_previous_action     (CodeSlayerEngine      *engine);
static void search_replace_action           (CodeSlayerEngine      *engine);
static void cut_action                      (CodeSlayerEngine      *engine);
static void copy_action                     (CodeSlayerEngine      *engine);
static void paste_action                    (CodeSlayerEngine      *engine);
static void copy_lines_action               (CodeSlayerEngine      *engine);
static void uppercase_action                (CodeSlayerEngine      *engine);
static void lowercase_action                (CodeSlayerEngine      *engine);

/* projects specific code */

static void search_find_projects_action     (CodeSlayerEngine      *engine,
                                             gchar                 *search_paths);
static void add_projects_action             (CodeSlayerEngine      *engine,
                                             GSList                *files);
static void remove_project_action           (CodeSlayerEngine      *engine,
                                             CodeSlayerProject     *project);
static void select_projects_document_action (CodeSlayerEngine      *engine,
                                             CodeSlayerDocument    *document);
static void select_document_action          (CodeSlayerEngine      *engine, 
                                             guint                  page_num);
static void scan_external_changes_action    (CodeSlayerEngine      *engine);
static gboolean close_search_action         (CodeSlayerEngine      *engine,
                                             GdkEvent              *event);
static void select_search_document_action   (CodeSlayerEngine      *engine,
                                             CodeSlayerDocument    *document);
static void rename_file_path_action         (CodeSlayerEngine      *engine,
                                             gchar                 *file_path,
                                             gchar                 *renamed_file_path);
                                             
static void load_regular_expression         (CodeSlayerEngine      *engine);
static void load_window_settings            (CodeSlayerEngine      *engine);
static void sync_menu_and_notebook          (CodeSlayerEngine      *engine);
static void save_document_settings          (CodeSlayerEngine      *engine);
static void save_window_settings            (CodeSlayerEngine      *engine);

                                                   
#define CODESLAYER_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_ENGINE_TYPE, CodeSlayerEnginePrivate))

typedef struct _CodeSlayerEnginePrivate CodeSlayerEnginePrivate;

struct _CodeSlayerEnginePrivate
{
  GtkWindow             *window;
  CodeSlayerProfile     *profile;
  CodeSlayerProfiles    *profiles;
  CodeSlayerRegistry    *registry;
  CodeSlayerPreferences *preferences;
  CodeSlayerPlugins     *plugins;
  GtkWidget             *projects;
  GtkWidget             *search;
  GtkWidget             *menu_bar;
  GtkWidget             *notebook;
  GtkWidget             *notebook_search;
  GtkWidget             *notebook_pane;
  GtkWidget             *regex_view;
  GtkWidget             *side_pane;
  GtkWidget             *bottom_pane;
  GtkWidget             *hpaned;
  GtkWidget             *vpaned;
  GdkWindowState         window_state;

  GtkWidget             *go_to_line_dialog;
  GdkRGBA                go_to_line_error_color;
  GdkRGBA                go_to_line_default_color;  
};

G_DEFINE_TYPE (CodeSlayerEngine, codeslayer_engine, G_TYPE_OBJECT)

static void
codeslayer_engine_class_init (CodeSlayerEngineClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_engine_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerEnginePrivate));
}

static void
codeslayer_engine_init (CodeSlayerEngine *engine) 
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  priv->preferences = NULL;
  priv->search = NULL;
  priv->regex_view = NULL;
}

static void
codeslayer_engine_finalize (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  g_object_unref (priv->preferences);  
  G_OBJECT_CLASS (codeslayer_engine_parent_class)->finalize (G_OBJECT (engine));
}

/**
 * codeslayer_engine_new:
 * @window: a #GtkWindow.
 * @profile: a #CodeSlayerProfile.
 * @profiles: a #CodeSlayerProfiles.
 * @profiles_manager: a #CodeSlayerProfilesManager.
 * @plugins: a #CodeSlayerPlugins.
 * @projects: a #CodeSlayerProjects.
 * @menu_bar: a #CodeSlayerMenuBar.
 * @notebook: a #CodeSlayerNotebook.
 * @notebook_search: a #CodeSlayerNotebookSearch.
 * @notebook_pane: a #CodeSlayerNotebookPane.
 * @side_pane: a #CodeSlayerSidePane.
 * @bottom_pane: a #CodeSlayerBottomPane.
 * @hpaned: the main horizontal pane.
 * @vpaned: the main vertical pane.
 *
 * Returns: a new #CodeSlayerEngine. 
 */
CodeSlayerEngine*
codeslayer_engine_new (GtkWindow          *window,
                       CodeSlayerProfile  *profile,
                       CodeSlayerProfiles *profiles,
                       GtkWidget          *profiles_manager,
                       CodeSlayerPlugins  *plugins,
                       GtkWidget          *projects,
                       GtkWidget          *menu_bar,
                       GtkWidget          *notebook,
                       GtkWidget          *notebook_search,
                       GtkWidget          *notebook_pane, 
                       GtkWidget          *side_pane,
                       GtkWidget          *bottom_pane, 
                       GtkWidget          *hpaned,
                       GtkWidget          *vpaned)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 
  CodeSlayerEngine *engine;

  engine = CODESLAYER_ENGINE (g_object_new (codeslayer_engine_get_type (), NULL));
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  priv->window = window;
  priv->projects = projects;
  priv->profiles = profiles;
  priv->profile = profile;
  priv->plugins = plugins;
  priv->menu_bar = menu_bar;
  priv->notebook = notebook;
  priv->notebook_search = notebook_search;
  priv->notebook_pane = notebook_pane;
  priv->side_pane = side_pane;
  priv->bottom_pane = bottom_pane;
  priv->hpaned = hpaned;
  priv->vpaned = vpaned;
  
  priv->preferences = codeslayer_preferences_new (GTK_WIDGET (window), profile);

  registry = codeslayer_profile_get_registry (profile);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "new-document",
                            G_CALLBACK (new_document_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "open-document",
                            G_CALLBACK (open_document_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "save-document",
                            G_CALLBACK (save_document_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "save-all-documents",
                            G_CALLBACK (save_all_documents_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "close-document",
                            G_CALLBACK (close_document_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "find",
                            G_CALLBACK (search_find_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "replace",
                            G_CALLBACK (search_replace_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "find-next",
                            G_CALLBACK (search_find_next_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "find-previous",
                            G_CALLBACK (search_find_previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "regular-expression",
                            G_CALLBACK (regular_expression_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "go-to-line",
                            G_CALLBACK (go_to_line_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "fullscreen-window",
                            G_CALLBACK (fullscreen_window_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "show-side-pane",
                            G_CALLBACK (toggle_side_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "show-bottom-pane",
                            G_CALLBACK (toggle_bottom_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (side_pane), "open-pane",
                            G_CALLBACK (open_side_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (side_pane), "close-pane",
                            G_CALLBACK (close_side_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (bottom_pane), "open-pane",
                            G_CALLBACK (open_bottom_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (bottom_pane), "close-pane",
                            G_CALLBACK (close_bottom_pane_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "draw-spaces",
                            G_CALLBACK (draw_spaces_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "word-wrap",
                            G_CALLBACK (word_wrap_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "show-preferences",
                            G_CALLBACK (show_preferences_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (priv->notebook), "page-removed",
                            G_CALLBACK (page_removed_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (registry), "registry-changed",
                            G_CALLBACK (registry_changed_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu_bar), "show-plugins",
                            G_CALLBACK (show_plugins_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu_bar), "cut",
                            G_CALLBACK (cut_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu_bar), "copy",
                            G_CALLBACK (copy_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu_bar), "paste",
                            G_CALLBACK (paste_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu_bar), "copy-lines",
                            G_CALLBACK (copy_lines_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu_bar), "to-uppercase",
                            G_CALLBACK (uppercase_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu_bar), "to-lowercase",
                            G_CALLBACK (lowercase_action), engine);
                            
  g_signal_connect_swapped (G_OBJECT (priv->menu_bar), "show-profiles",
                            G_CALLBACK (codeslayer_profiles_manager_run_dialog), 
                            profiles_manager);                            
                            
  /* projects specific code */     
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "find-projects",
                            G_CALLBACK (search_find_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "find-projects",
                            G_CALLBACK (search_find_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "add-projects",
                            G_CALLBACK (add_projects_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu_bar), "scan-external-changes",
                            G_CALLBACK (scan_external_changes_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "remove-project",
                            G_CALLBACK (remove_project_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "select-document",
                            G_CALLBACK (select_projects_document_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "select-document",
                            G_CALLBACK (select_document_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (projects), "file-path-renamed",
                            G_CALLBACK (rename_file_path_action), engine);

  return engine;
}

/**
 * codeslayer_engine_load_profile:
 * @engine: a #CodeSlayerEngine.
 */
void
codeslayer_engine_load_profile (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  load_window_settings (engine);
  load_regular_expression (engine);

  if (!codeslayer_profile_get_enable_projects (priv->profile))
    new_document_action (engine);
      
  if (!codeslayer_abstract_pane_exists (CODESLAYER_ABSTRACT_PANE (priv->side_pane), 
                                        priv->projects))
    {
      codeslayer_abstract_pane_insert (CODESLAYER_ABSTRACT_PANE (priv->side_pane), 
                                       priv->projects, "Projects", 0);
    }

  if (codeslayer_profile_get_enable_projects (priv->profile))
    {
      GList *projects;
      GList *documents;
      
      projects = codeslayer_profile_get_projects (priv->profile);
      while (projects != NULL)
        {
          CodeSlayerProject *project = projects->data;
          codeslayer_projects_add_project (CODESLAYER_PROJECTS (priv->projects), 
                                           project);
          projects = g_list_next (projects);
        }

      documents = codeslayer_profile_get_documents (priv->profile);
      while (documents != NULL)
        {
          CodeSlayerDocument *document = documents->data;
          codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), 
                                               document);
          documents = g_list_next (documents);
        }
        
      gtk_widget_show (priv->projects);
    }
  else
    {
      gtk_widget_hide (priv->projects);
    }
  
  codeslayer_profile_remove_all_documents (priv->profile);
  
  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
  codeslayer_plugins_activate (priv->plugins, priv->profile);
  
  g_signal_emit_by_name ((gpointer) registry, "registry-initialized");
}

/**
 * codeslayer_engine_open_document:
 * @engine: a #CodeSlayerEngine.
 * @file_path: the document to open by file path.
 *
 * Will first check to make sure that the document doesn't already exist.
 */
void
codeslayer_engine_open_document (CodeSlayerEngine *engine, 
                                 gchar            *file_path)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerDocument *document;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  close_default_document (engine);
  
  document = codeslayer_document_new ();
  codeslayer_document_set_file_path (document, file_path);
  
  if (!codeslayer_notebook_select_document (CODESLAYER_NOTEBOOK (priv->notebook), document))
    codeslayer_notebook_add_document (CODESLAYER_NOTEBOOK (priv->notebook), document);
  
  g_object_unref (document);

  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
}

/**
 * codeslayer_engine_save_profile:
 * @engine: a #CodeSlayerEngine.
 */
gboolean
codeslayer_engine_save_profile (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  if (priv->profile == NULL)
    return TRUE;
  
  if (codeslayer_notebook_has_unsaved_documents (CODESLAYER_NOTEBOOK (priv->notebook)))
    return FALSE;

  save_window_settings (engine);
  save_document_settings (engine);
  
  if (!codeslayer_profile_get_enable_projects (priv->profile))
    codeslayer_profile_remove_all_projects (priv->profile);
  
  codeslayer_profiles_save_profile (priv->profiles, priv->profile);
  
  return TRUE;
}

static void
close_default_document (CodeSlayerEngine *engine)

{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 1)
    {
      GtkWidget *source_view;
      GtkTextBuffer *buffer;
      source_view = codeslayer_notebook_get_active_source_view (CODESLAYER_NOTEBOOK (priv->notebook));
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));

      if (gtk_text_buffer_get_char_count (buffer) <= 0)
        codeslayer_notebook_close_document (CODESLAYER_NOTEBOOK (priv->notebook), 0);
    }
}

static gchar* 
get_document_name (CodeSlayerEngine *engine)
{
  gchar *result = NULL;
  gchar *num = NULL;
  static gint count = 0;

  count++;

  num = g_strdup_printf ("%d", count);

  result = g_strconcat (CODESLAYER_SOURCE_VIEW_UNTITLED, " ", num, NULL);

  g_free (num);

  return result;
}

static void
new_document_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerDocument *document;
  gchar *name;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  name = get_document_name (engine);
  
  document = codeslayer_document_new ();
  g_object_set (document, "name", name, NULL);
  
  codeslayer_notebook_add_document (CODESLAYER_NOTEBOOK (priv->notebook), document);
  
  g_object_unref (document);
  g_free (name);
  
  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
}

static void
open_document_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *dialog;
  gint response;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  dialog = gtk_file_chooser_dialog_new (_("Select File"), 
                                        GTK_WINDOW (priv->window),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_OK, 
                                        NULL);
                                        
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);;
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GSList *files = gtk_file_chooser_get_files (GTK_FILE_CHOOSER (dialog));
      GSList *list;
      list = files;
      
      while (list != NULL)
        {
          GFile *file = list->data;
          gchar *file_path;
          CodeSlayerDocument *document;

          file_path = g_file_get_path (file);
          
          document = codeslayer_document_new ();
          codeslayer_document_set_file_path (document, file_path);
          
          codeslayer_notebook_add_document (CODESLAYER_NOTEBOOK (priv->notebook), document);
          
          g_object_unref (document);

          g_free (file_path);
          list = g_slist_next (list);
        }

      g_slist_foreach (files, (GFunc) g_object_unref, NULL);
      g_slist_free (files);
    }
  gtk_widget_destroy (GTK_WIDGET (dialog));

  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
}

static void
save_document_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  notebook = priv->notebook;
  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  codeslayer_notebook_save_document (CODESLAYER_NOTEBOOK (notebook), page);
}

static void
save_all_documents_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  notebook = priv->notebook;
  codeslayer_notebook_save_all_documents (CODESLAYER_NOTEBOOK (notebook));
}

static void
close_document_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  notebook = priv->notebook;
  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  codeslayer_notebook_close_document (CODESLAYER_NOTEBOOK (notebook), page);
}

static void
page_removed_action (CodeSlayerEngine *engine, 
                     GtkWidget        *page,
                     guint             removed_page_num)
{
  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
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
  CodeSlayerRegistry *registry; 
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  if (gtk_widget_get_visible (GTK_WIDGET(priv->side_pane)))
    {
      gtk_widget_hide (GTK_WIDGET(priv->side_pane));
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE,
                                       FALSE);
    }
  else
    {
      gtk_widget_show (GTK_WIDGET(priv->side_pane));
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE,
                                       TRUE);
    }
}

static void
open_side_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);  

  registry = codeslayer_profile_get_registry (priv->profile);

  gtk_widget_show (GTK_WIDGET(priv->side_pane));
  codeslayer_registry_set_boolean (registry, 
                                   CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE,
                                   TRUE);
}

static void
close_side_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  gtk_widget_hide (GTK_WIDGET(priv->side_pane));
  codeslayer_registry_set_boolean (registry, 
                                   CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE,
                                   FALSE);
}

static void
toggle_bottom_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  if (gtk_widget_get_visible (GTK_WIDGET(priv->bottom_pane)))
    {
      gtk_widget_hide (GTK_WIDGET(priv->bottom_pane));
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE,
                                       FALSE);
    }
  else
    {
      gtk_widget_show (GTK_WIDGET(priv->bottom_pane));
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE,
                                       TRUE);
    }
    
  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
}

static void
open_bottom_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  registry = codeslayer_profile_get_registry (priv->profile);

  gtk_widget_show (GTK_WIDGET(priv->bottom_pane));
  codeslayer_registry_set_boolean (registry, 
                                   CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE,
                                   TRUE);
  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
}

static void
close_bottom_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  registry = codeslayer_profile_get_registry (priv->profile);

  gtk_widget_hide (GTK_WIDGET(priv->bottom_pane));
  codeslayer_registry_set_boolean (registry, 
                                   CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE,
                                   FALSE);
  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
}

static void
draw_spaces_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 
  gboolean draw_spaces;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  registry = codeslayer_profile_get_registry (priv->profile);

  draw_spaces = codeslayer_registry_get_boolean (registry, 
                                                 CODESLAYER_REGISTRY_DRAW_SPACES);
  if (draw_spaces)
    {
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_DRAW_SPACES,
                                       FALSE);
      registry_changed_action (engine);
    }
  else
    {
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_DRAW_SPACES,
                                       TRUE);
      registry_changed_action (engine);
    }
}

static void
word_wrap_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry;
  gboolean word_wrap;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  word_wrap = codeslayer_registry_get_boolean (registry, 
                                               CODESLAYER_REGISTRY_WORD_WRAP);
                                                   
  if (word_wrap)
    {
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_WORD_WRAP,
                                       FALSE);
      registry_changed_action (engine);
    }
  else
    {
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_WORD_WRAP,
                                       TRUE);
      registry_changed_action (engine);
    }
}

static void
show_preferences_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_preferences_run_dialog (priv->preferences);
}

static void
registry_changed_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv; 
  gint pages;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));

  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page; 
      GtkWidget *source_view;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      codeslayer_source_view_sync_registry (CODESLAYER_SOURCE_VIEW (source_view));
    }
    
  if (codeslayer_profile_get_enable_projects (priv->profile))
    gtk_widget_show (priv->projects);
  else
    gtk_widget_hide (priv->projects);

  sync_menu_and_notebook (engine);
}

static void
regular_expression_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv; 
  CodeSlayerRegistry *registry;
  gboolean regular_expression_view;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  regular_expression_view = codeslayer_registry_get_boolean (registry, 
                                                        CODESLAYER_REGISTRY_ENABLE_REGULAR_EXPRESSION);

  if (regular_expression_view)
    {
      codeslayer_abstract_pane_remove (CODESLAYER_ABSTRACT_PANE (priv->bottom_pane), 
                                       priv->regex_view);
      priv->regex_view = NULL;

      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_ENABLE_REGULAR_EXPRESSION,
                                       FALSE);
    }
  else
    {
      priv->regex_view = codeslayer_regex_view_new (priv->notebook_search, 
                                                    priv->notebook, 
                                                    priv->profile);
                                                    
      codeslayer_abstract_pane_add (CODESLAYER_ABSTRACT_PANE (priv->bottom_pane), 
                                    priv->regex_view, _("Regular Expression"));
      
      gtk_widget_set_visible (gtk_paned_get_child2 (GTK_PANED(priv->vpaned)), TRUE);
      
      codeslayer_registry_set_boolean (registry, 
                                       CODESLAYER_REGISTRY_ENABLE_REGULAR_EXPRESSION,
                                       TRUE);
    }
}

static void

go_to_line_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *content_area;
  GtkWidget *hbox;
  GtkWidget *entry;
  GtkStyleContext *style_context;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  priv->go_to_line_dialog = gtk_dialog_new ();  
  
  gtk_window_set_title (GTK_WINDOW (priv->go_to_line_dialog), _("Go To Line"));
  gtk_window_set_transient_for (GTK_WINDOW (priv->go_to_line_dialog), 
                                GTK_WINDOW (priv->window));
  gtk_window_set_resizable (GTK_WINDOW (priv->go_to_line_dialog), FALSE);
  
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->go_to_line_dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->go_to_line_dialog), TRUE);
  gtk_dialog_set_default_response (GTK_DIALOG (priv->go_to_line_dialog), 
                                   GTK_RESPONSE_OK);
  
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (priv->go_to_line_dialog));
  
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  gtk_entry_set_width_chars (GTK_ENTRY (entry), 15);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 3);

  g_signal_connect (G_OBJECT (entry), "key-press-event",
                    G_CALLBACK (go_to_line_keypress_action), engine);

  gtk_widget_show_all (hbox);
  gtk_container_add (GTK_CONTAINER (content_area), hbox);
  
  style_context = gtk_widget_get_style_context (entry);
  
  gdk_rgba_parse (&(priv->go_to_line_error_color), "#ed3636");
  gtk_style_context_get_color (style_context, GTK_STATE_FLAG_NORMAL, 
                               &(priv->go_to_line_default_color));    

  gtk_dialog_run (GTK_DIALOG (priv->go_to_line_dialog));
  gtk_widget_destroy (priv->go_to_line_dialog);
}

static gboolean            
go_to_line_keypress_action (GtkWidget        *entry,
                            GdkEventKey      *event, 
                            CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  const gchar *text;
  gchar *string;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  gtk_widget_override_color (entry, GTK_STATE_FLAG_NORMAL, 
                             &(priv->go_to_line_default_color));

  if (event->keyval != GDK_KEY_Return)
    return FALSE;

  text = gtk_entry_get_text (GTK_ENTRY (entry));
    
  string = g_strdup (text);
  
  if (codeslayer_utils_isdigit (string))
    {
      GtkWidget *source_view;
      source_view = codeslayer_notebook_get_active_source_view (CODESLAYER_NOTEBOOK (priv->notebook));
      if (!codeslayer_source_view_scroll_to_line (CODESLAYER_SOURCE_VIEW (source_view), atoi(string)))
        {
          gtk_widget_override_color (entry, GTK_STATE_FLAG_NORMAL, 
                                     &(priv->go_to_line_error_color));
          return FALSE;
        }
    }
  
  g_free (string);

  g_signal_emit_by_name ((gpointer) priv->go_to_line_dialog, "close");
  return FALSE;
}

static void
show_plugins_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_plugins_run_dialog (priv->plugins, priv->profile);
}

static void
search_find_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *focused_window;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  focused_window = gtk_window_get_focus (priv->window);
  if (focused_window != NULL && gtk_widget_get_ancestor (focused_window, CODESLAYER_PROJECTS_TYPE))
      g_signal_emit_by_name ((gpointer) priv->projects, "search-find");
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
cut_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;  
  GtkWidget *focused_window;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  focused_window = gtk_window_get_focus (priv->window);
  if (focused_window != NULL && gtk_widget_get_ancestor (focused_window, CODESLAYER_PROJECTS_TYPE))
    {
      g_signal_emit_by_name ((gpointer) priv->projects, "cut-file-folder");
    }
  else if (focused_window != NULL && 
           (gtk_widget_get_ancestor (focused_window, GTK_TYPE_ENTRY) || 
            gtk_widget_get_ancestor (focused_window, GTK_TYPE_TEXT_VIEW)))
    {
      g_signal_emit_by_name ((gpointer) focused_window, "cut-clipboard");
    }
}

static void
copy_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;  
  GtkWidget *focused_window;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  focused_window = gtk_window_get_focus (priv->window);
  if (focused_window != NULL && gtk_widget_get_ancestor (focused_window, CODESLAYER_PROJECTS_TYPE))
    {
      g_signal_emit_by_name ((gpointer) priv->projects, "copy-file-folder");
    }
  else if (focused_window != NULL && 
           (gtk_widget_get_ancestor (focused_window, GTK_TYPE_ENTRY) || 
            gtk_widget_get_ancestor (focused_window, GTK_TYPE_TEXT_VIEW)))
    {
      g_signal_emit_by_name ((gpointer) focused_window, "copy-clipboard");
    }
}

static void
paste_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;  
  GtkWidget *focused_window;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  focused_window = gtk_window_get_focus (priv->window);
  if (focused_window != NULL && gtk_widget_get_ancestor (focused_window, CODESLAYER_PROJECTS_TYPE))
    {
      g_signal_emit_by_name ((gpointer) priv->projects, "paste-file-folder");
    }
  else if (focused_window != NULL && 
           (gtk_widget_get_ancestor (focused_window, GTK_TYPE_ENTRY) || 
            gtk_widget_get_ancestor (focused_window, GTK_TYPE_TEXT_VIEW)))
    {
      g_signal_emit_by_name ((gpointer) focused_window, "paste-clipboard");
    }
}

static void
copy_lines_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *source_view;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  source_view = codeslayer_notebook_get_active_source_view (CODESLAYER_NOTEBOOK (priv->notebook));
  g_signal_emit_by_name ((gpointer) source_view, "copy-lines");
}

static void
lowercase_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *source_view;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  source_view = codeslayer_notebook_get_active_source_view (CODESLAYER_NOTEBOOK (priv->notebook));
  g_signal_emit_by_name ((gpointer) source_view, "to-lowercase");
}

static void
uppercase_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *source_view;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  source_view = codeslayer_notebook_get_active_source_view (CODESLAYER_NOTEBOOK (priv->notebook));
  g_signal_emit_by_name ((gpointer) source_view, "to-uppercase");
}

/* projects specific code */

static void
add_projects_action (CodeSlayerEngine *engine,
                     GSList           *files)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  gtk_widget_show (priv->projects);
  open_side_pane_action (engine);

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

      codeslayer_profile_add_project (priv->profile, project);

      codeslayer_profiles_save_profile (priv->profiles, priv->profile);

      codeslayer_projects_add_project (CODESLAYER_PROJECTS (priv->projects), project);
      
      files = g_slist_next (files);
    }
    
  g_signal_emit_by_name ((gpointer) priv->projects, "projects-changed");
}

static void
remove_project_action (CodeSlayerEngine  *engine,
                       CodeSlayerProject *project)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  codeslayer_profile_remove_project (priv->profile, project);
  
  codeslayer_profiles_save_profile (priv->profiles, priv->profile);
  
  g_signal_emit_by_name ((gpointer) priv->projects, "projects-changed");
}

static void
select_projects_document_action (CodeSlayerEngine   *engine,
                                 CodeSlayerDocument *document)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *notebook;
  const gchar *file_path;
  GFile *file;

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
  
  if (!codeslayer_notebook_select_document (CODESLAYER_NOTEBOOK (notebook), document))
    codeslayer_notebook_add_document (CODESLAYER_NOTEBOOK (notebook), document);

  sync_menu_and_notebook (CODESLAYER_ENGINE (engine));
}

static void
select_document_action (CodeSlayerEngine *engine, 
                        guint             page_num)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 
  GtkWidget *notebook_page;
  gboolean sync_with_document;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  if (codeslayer_profile_get_projects (priv->profile) == NULL)
    return;

  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page_num);
  if (notebook_page == NULL)
    return;
  
  sync_with_document = codeslayer_registry_get_boolean (registry, 
                                                        CODESLAYER_REGISTRY_SYNC_WITH_DOCUMENT);
  
  if (sync_with_document)
    {
      GtkWidget *source_view;
      CodeSlayerDocument *document;

      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view));

      if (!codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), document))
        codeslayer_notebook_page_show_document_not_found_info_bar (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
    }
}

static void
search_find_projects_action (CodeSlayerEngine *engine,
                             gchar            *file_paths)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 
  gint search_width;
  gint search_height;
  gint search_x;
  gint search_y;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  if (priv->search == NULL)
    {
      priv->search = codeslayer_projects_search_new (priv->window, 
                                                     priv->profile);

      g_signal_connect_swapped (G_OBJECT (priv->search), "close",
                                G_CALLBACK (close_search_action), engine);
                        
      g_signal_connect_swapped (G_OBJECT (priv->search), "select-document",
                                G_CALLBACK (select_search_document_action), engine);
    }
    
  if (!gtk_widget_get_visible (priv->search))
    {
      search_width = codeslayer_registry_get_integer (registry,
                                                      CODESLAYER_REGISTRY_SEARCH_WIDTH);
      if (search_width < 0)
        search_width = 600;
        
      search_height = codeslayer_registry_get_integer (registry,
                                                       CODESLAYER_REGISTRY_SEARCH_HEIGHT);
      if (search_height < 0)
        search_height = 350;
        
      gtk_window_set_default_size (GTK_WINDOW (priv->search), search_width, search_height);

      search_x = codeslayer_registry_get_integer (registry,
                                                  CODESLAYER_REGISTRY_SEARCH_X);
      if (search_x < 0)
        search_x = 10;
        
      search_y = codeslayer_registry_get_integer (registry,
                                                  CODESLAYER_REGISTRY_SEARCH_Y);
      if (search_y < 0)
        search_y = 10;

      gtk_window_move (GTK_WINDOW (priv->search), search_x, search_y);
    }
    
  gtk_window_present (GTK_WINDOW (priv->search));
  gtk_widget_show_all (GTK_WIDGET (priv->search));
  
  if (codeslayer_utils_has_text (file_paths))
    codeslayer_projects_search_find_selection (CODESLAYER_PROJECTS_SEARCH (priv->search), file_paths);
  else
    codeslayer_projects_search_find_projects (CODESLAYER_PROJECTS_SEARCH (priv->search));
}

static gboolean
close_search_action (CodeSlayerEngine *engine, 
                     GdkEvent         *event)
{
  CodeSlayerEnginePrivate *priv; 
  CodeSlayerRegistry *registry; 
  gint width;
  gint height;
  gint x;
  gint y;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  gtk_window_get_size (GTK_WINDOW (priv->search), &width, &height);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_SEARCH_WIDTH,
                                   width);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_SEARCH_HEIGHT,
                                   height);

  gtk_window_get_position (GTK_WINDOW (priv->search), &x, &y);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_SEARCH_X, x);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_SEARCH_Y, y);

  gtk_widget_hide (priv->search);

  return TRUE;
}

static void
select_search_document_action (CodeSlayerEngine   *engine,
                               CodeSlayerDocument *document)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  codeslayer_projects_select_document (CODESLAYER_PROJECTS (priv->projects), 
                                       document);
}

static void
scan_external_changes_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  gint pages;
  gint page;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  codeslayer_projects_refresh (CODESLAYER_PROJECTS (priv->projects));
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));

  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page; 
      GtkWidget *source_view;
      CodeSlayerDocument *document;
      const gchar *file_path;
      GTimeVal *original_modification_time;
      GTimeVal *latest_modification_time;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view));
      file_path = codeslayer_document_get_file_path (document);
      
      if (file_path == NULL)
        continue;
        
      original_modification_time = codeslayer_source_view_get_modification_time (CODESLAYER_SOURCE_VIEW (source_view));

      latest_modification_time = codeslayer_utils_get_modification_time (file_path);
      
      if (latest_modification_time->tv_sec > original_modification_time->tv_sec)
        codeslayer_notebook_page_show_external_changes_info_bar (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
        
      g_free (latest_modification_time);
    }

  g_signal_emit_by_name ((gpointer) priv->projects, "projects-changed");
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
      GtkWidget *source_view;
      CodeSlayerDocument *document;
      const gchar *current_file_path;
      guint length;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view));
      current_file_path = codeslayer_document_get_file_path (document);

      length = g_strv_length (&file_path);

      if (g_ascii_strncasecmp (current_file_path, file_path, length) == 0)
        {
          gchar *replacement_file_path;
          gchar *replacement_basename;
          GtkWidget *notebook_label;
          
          replacement_file_path = codeslayer_utils_strreplace (current_file_path, 
                                                               file_path,
                                                               renamed_file_path);
          codeslayer_document_set_file_path (document,
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

/* engine common code */

static void
save_document_settings (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GList *documents = NULL;
  gint pages;
  gint page;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  if (codeslayer_profile_get_enable_projects (priv->profile) == FALSE)
    return;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));
  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page;
      GtkWidget *source_view;
      CodeSlayerDocument *document;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view));

      documents = g_list_append (documents, document);
    }
    
  codeslayer_profile_set_documents (priv->profile, documents);
}

static void
save_window_settings (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 
  gint width;
  gint height;
  gint x;
  gint y;
  gint position;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  gtk_window_get_size (GTK_WINDOW (priv->window), &width, &height);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_WINDOW_WIDTH,
                                   width);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_WINDOW_HEIGHT,
                                   height);

  gtk_window_get_position (GTK_WINDOW (priv->window), &x, &y);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_WINDOW_X, x);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_WINDOW_Y, y);

  position = gtk_paned_get_position (GTK_PANED (priv->hpaned));
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_HPANED_POSITION,
                                   position);

  position = gtk_paned_get_position (GTK_PANED (priv->vpaned));
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_VPANED_POSITION,
                                   position);
}

static void
load_regular_expression (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry;
  gboolean regular_expression;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  regular_expression = codeslayer_registry_get_boolean (registry, 
                                                        CODESLAYER_REGISTRY_ENABLE_REGULAR_EXPRESSION);
  if (regular_expression)
    {
      priv->regex_view = codeslayer_regex_view_new (priv->notebook_search, 
                                                    priv->notebook, 
                                                    priv->profile);
                                                    
      codeslayer_abstract_pane_add (CODESLAYER_ABSTRACT_PANE (priv->bottom_pane), 
                                    priv->regex_view, _("Regular Expression"));
    }
}

static void
load_window_settings (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerRegistry *registry; 
  gint window_width;
  gint window_height;
  gint window_x;
  gint window_y;
  gint hpaned_position;
  gint vpaned_position;
  gboolean show_side_pane;
  gboolean show_bottom_pane;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  registry = codeslayer_profile_get_registry (priv->profile);
    
  /* window specific settings */                                              

  window_width = codeslayer_registry_get_integer (registry,
                                                  CODESLAYER_REGISTRY_WINDOW_WIDTH);
  if (window_width < 0)
    {
      window_width = 800;
    }
  
  window_height = codeslayer_registry_get_integer (registry,
                                                   CODESLAYER_REGISTRY_WINDOW_HEIGHT);
  if (window_height < 0)
    {
      window_height = 600;
    }
    
  /*gtk_window_set_default_size (GTK_WINDOW (priv->window), window_width, window_height);*/
  gtk_window_resize (GTK_WINDOW (priv->window), window_width, window_height);

  window_x = codeslayer_registry_get_integer (registry,
                                              CODESLAYER_REGISTRY_WINDOW_X);
  if (window_x < 0)
    {
      window_x = 10;
    }
    
  window_y = codeslayer_registry_get_integer (registry,
                                              CODESLAYER_REGISTRY_WINDOW_Y);
  if (window_y < 0)
    {
      window_y = 10;
    }
    
  gtk_window_move (GTK_WINDOW (priv->window), window_x, window_y);
    
  /* side and bottom pane settings */
  
  hpaned_position = codeslayer_registry_get_integer (registry,
                                                     CODESLAYER_REGISTRY_HPANED_POSITION);
  if (hpaned_position == -1)
    hpaned_position = 250;

  gtk_paned_set_position (GTK_PANED (priv->hpaned), hpaned_position);
                                                
  vpaned_position = codeslayer_registry_get_integer (registry,
                                                     CODESLAYER_REGISTRY_VPANED_POSITION);
  if (vpaned_position == -1)
    vpaned_position = 250;

  gtk_paned_set_position (GTK_PANED (priv->vpaned), vpaned_position);
    
  /* we have to this before we show or hide the panes */
  gtk_widget_show_all (GTK_WIDGET (priv->window));
    
  /* show or hide panes */
  
  show_side_pane = codeslayer_registry_get_boolean (registry,
                                                    CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE);
  gtk_widget_set_visible (gtk_paned_get_child1 (GTK_PANED(priv->hpaned)), 
                                                show_side_pane);
                                                
  show_bottom_pane = codeslayer_registry_get_boolean (registry,
                                                      CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE);
  gtk_widget_set_visible (gtk_paned_get_child2 (GTK_PANED(priv->vpaned)), 
                                                show_bottom_pane);
}

static void 
sync_menu_and_notebook (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  gboolean enable_projects;
  gboolean has_open_documents;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  enable_projects = codeslayer_profile_get_enable_projects (priv->profile);
  has_open_documents = codeslayer_notebook_has_open_documents (CODESLAYER_NOTEBOOK (priv->notebook));
  
  g_signal_emit_by_name ((gpointer) priv->notebook, "sync-notebook");
  g_signal_emit_by_name ((gpointer) priv->menu_bar, "sync-menu", enable_projects, has_open_documents);
}
