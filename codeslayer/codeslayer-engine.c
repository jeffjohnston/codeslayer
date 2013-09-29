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
#include <codeslayer/codeslayer-profile.h>
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
 * the rest of the API. By having such a central engine we can keep various 
 * parts of the application very decoupled.
 */

static void codeslayer_engine_class_init                (CodeSlayerEngineClass  *klass);
static void codeslayer_engine_init                      (CodeSlayerEngine       *engine);
static void codeslayer_engine_finalize                  (CodeSlayerEngine       *engine);

static void new_editor_action                           (CodeSlayerEngine       *engine);
static void open_editor_action                          (CodeSlayerEngine       *engine);
static void save_editor_action                          (CodeSlayerEngine       *engine);
static void save_all_editors_action                     (CodeSlayerEngine       *engine);
static void close_editor_action                         (CodeSlayerEngine       *engine);
static void search_find_action                          (CodeSlayerEngine       *engine);
static void search_find_next_action                     (CodeSlayerEngine       *engine);
static void search_find_previous_action                 (CodeSlayerEngine       *engine);
static void search_replace_action                       (CodeSlayerEngine       *engine);
static void go_to_line_action                           (CodeSlayerEngine       *engine);
static gboolean go_to_line_keypress_action              (GtkWidget              *entry,
                                                         GdkEventKey            *event, 
                                                         CodeSlayerEngine       *engine);
static void fullscreen_window_action                    (CodeSlayerEngine       *engine);
static void toggle_side_pane_action                     (CodeSlayerEngine       *engine);
static void open_side_pane_action                       (CodeSlayerEngine       *engine);
static void close_side_pane_action                      (CodeSlayerEngine       *engine);
static void toggle_bottom_pane_action                   (CodeSlayerEngine       *engine);
static void open_bottom_pane_action                     (CodeSlayerEngine       *engine);
static void close_bottom_pane_action                    (CodeSlayerEngine       *engine);
static void draw_spaces_action                          (CodeSlayerEngine       *engine);
static void word_wrap_action                            (CodeSlayerEngine       *engine);
static void page_removed_action                         (CodeSlayerEngine       *engine,
                                                         GtkWidget              *page, 
                                                         guint                   page_num);
static void show_preferences_action                     (CodeSlayerEngine       *engine);
static void registry_changed_action                     (CodeSlayerEngine       *engine);

static void show_plugins_action                         (CodeSlayerEngine      *engine);
                                                   
#define CODESLAYER_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_ENGINE_TYPE, CodeSlayerEnginePrivate))

typedef struct _CodeSlayerEnginePrivate CodeSlayerEnginePrivate;

struct _CodeSlayerEnginePrivate
{
  GtkWindow               *window;
  CodeSlayerRegistry      *registry;
  CodeSlayerPreferences   *preferences;
  CodeSlayerProfileHandler *profile_handler;
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

  GtkWidget               *go_to_line_dialog;
  GdkRGBA                  go_to_line_error_color;
  GdkRGBA                  go_to_line_default_color;  
};

G_DEFINE_TYPE (CodeSlayerEngine, codeslayer_engine, CODESLAYER_ABSTRACT_ENGINE_TYPE)

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
  priv->search = NULL;
}

static void
codeslayer_engine_finalize (CodeSlayerEngine *engine)
{
  G_OBJECT_CLASS (codeslayer_engine_parent_class)->finalize (G_OBJECT (engine));
}

/**
 * codeslayer_engine_new:
 * @window: a #GtkWindow.
 * @registry: a #CodeSlayerRegistry.
 * @preferences: a #CodeSlayerPreferences.
 * @plugins: a #CodeSlayerPlugins.
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
codeslayer_engine_new (GtkWindow               *window,
                       CodeSlayerRegistry      *registry,
                       CodeSlayerPreferences   *preferences,
                       CodeSlayerProfileHandler *profile_handler,
                       CodeSlayerPlugins       *plugins,
                       GtkWidget               *menubar,
                       GtkWidget               *notebook,
                       GtkWidget               *notebook_pane, 
                       GtkWidget               *side_pane,
                       GtkWidget               *bottom_pane, 
                       GtkWidget               *hpaned,
                       GtkWidget               *vpaned)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerEngine *engine;

  engine = CODESLAYER_ENGINE (g_object_new (codeslayer_engine_get_type (), NULL));
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  priv->window = window;
  priv->registry = registry;
  priv->preferences = preferences;
  priv->profile_handler = profile_handler;
  priv->plugins = plugins;
  priv->menubar = menubar;
  priv->notebook = notebook;
  priv->notebook_pane = notebook_pane;
  priv->side_pane = side_pane;
  priv->bottom_pane = bottom_pane;
  priv->hpaned = hpaned;
  priv->vpaned = vpaned;
  
  g_object_set (CODESLAYER_ABSTRACT_ENGINE (engine), 
                "window", window, 
                "registry", registry, 
                "profile_handler", profile_handler, 
                "menubar", menubar, 
                "notebook", notebook, 
                "notebook_pane", notebook_pane, 
                "side_pane", side_pane, 
                "bottom_pane", bottom_pane, 
                "hpaned", hpaned, 
                "vpaned", vpaned, 
                NULL);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "new-editor",
                            G_CALLBACK (new_editor_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "open-editor",
                            G_CALLBACK (open_editor_action), engine);
  
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
  
  g_signal_connect_swapped (G_OBJECT (menubar), "go-to-line",
                            G_CALLBACK (go_to_line_action), engine);
  
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
  
  g_signal_connect_swapped (G_OBJECT (menubar), "word-wrap",
                            G_CALLBACK (word_wrap_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menubar), "show-preferences",
                            G_CALLBACK (show_preferences_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (priv->notebook), "page-removed",
                            G_CALLBACK (page_removed_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (registry), "registry-changed",
                            G_CALLBACK (registry_changed_action), engine);

  g_signal_connect_swapped (G_OBJECT (menubar), "show-plugins",
                            G_CALLBACK (show_plugins_action), engine);

  return engine;
}

/**
 * codeslayer_engine_load_default_profile:
 * @engine: a #CodeSlayerEngine.
 */
void
codeslayer_engine_load_default_profile (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerProfile *profile;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  profile = codeslayer_profile_handler_load_default_profile (priv->profile_handler);
  
  g_signal_emit_by_name ((gpointer) priv->registry, "registry-initialized");

  codeslayer_abstract_engine_load_window_settings (CODESLAYER_ABSTRACT_ENGINE (engine));
  
  new_editor_action (engine);

  codeslayer_plugins_activate (priv->plugins, profile);  
}

void
codeslayer_engine_open_editor (CodeSlayerEngine *engine, 
                               gchar            *file_path)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  CodeSlayerDocument *document;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  editor = codeslayer_notebook_get_active_editor (CODESLAYER_NOTEBOOK (priv->notebook));
  if (editor == NULL)
    return;
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  if (gtk_text_buffer_get_char_count (buffer) <= 0 && 
      gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 1)
    codeslayer_notebook_close_editor (CODESLAYER_NOTEBOOK (priv->notebook), 0);
  
  document = codeslayer_document_new ();
  codeslayer_document_set_file_path (document, file_path);
  codeslayer_notebook_add_editor (CODESLAYER_NOTEBOOK (priv->notebook), document);
  
  g_object_unref (document);

  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));
}

static void
new_editor_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  CodeSlayerDocument *document;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);

  document = codeslayer_document_new ();
  codeslayer_notebook_add_editor (CODESLAYER_NOTEBOOK (priv->notebook), document);
  
  g_object_unref (document);
  
  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));
}

static void
open_editor_action (CodeSlayerEngine *engine)
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
          
          codeslayer_notebook_add_editor (CODESLAYER_NOTEBOOK (priv->notebook), document);
          
          g_object_unref (document);

          g_free (file_path);
          list = g_slist_next (list);
        }

      g_slist_foreach (files, (GFunc) g_object_unref, NULL);
      g_slist_free (files);
    }
  gtk_widget_destroy (GTK_WIDGET (dialog));

  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));
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
page_removed_action (CodeSlayerEngine *engine, 
                     GtkWidget        *page,
                     guint             removed_page_num)
{
  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));
}

static void
search_find_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  GtkWidget *focused_window;

  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  focused_window = gtk_window_get_focus (priv->window);
  
  if (focused_window == NULL || !gtk_widget_get_ancestor (focused_window, CODESLAYER_PROJECTS_TYPE))
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
      codeslayer_registry_set_boolean (priv->registry, 
                                       CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE,
                                       FALSE);
    }
  else
    {
      gtk_widget_show (GTK_WIDGET(priv->side_pane));
      codeslayer_registry_set_boolean (priv->registry, 
                                       CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE,
                                       TRUE);
    }
}

static void
open_side_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  gtk_widget_show (GTK_WIDGET(priv->side_pane));
  codeslayer_registry_set_boolean (priv->registry, 
                                   CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE,
                                   TRUE);
}

static void
close_side_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  gtk_widget_hide (GTK_WIDGET(priv->side_pane));
  codeslayer_registry_set_boolean (priv->registry, 
                                   CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE,
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
      codeslayer_registry_set_boolean (priv->registry, 
                                       CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE,
                                       FALSE);
    }
  else
    {
      gtk_widget_show (GTK_WIDGET(priv->bottom_pane));
      codeslayer_registry_set_boolean (priv->registry, 
                                       CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE,
                                       TRUE);
    }
    
  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));
}

static void
open_bottom_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  gtk_widget_show (GTK_WIDGET(priv->bottom_pane));
  codeslayer_registry_set_boolean (priv->registry, 
                                   CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE,
                                   TRUE);
  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));
}

static void
close_bottom_pane_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  gtk_widget_hide (GTK_WIDGET(priv->bottom_pane));
  codeslayer_registry_set_boolean (priv->registry, 
                                   CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE,
                                   FALSE);
  codeslayer_abstract_engine_sync_menu_bar (CODESLAYER_ABSTRACT_ENGINE (engine));
}

static void
draw_spaces_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  gboolean draw_spaces;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  draw_spaces = codeslayer_registry_get_boolean (priv->registry, 
                                                 CODESLAYER_REGISTRY_DRAW_SPACES);
  if (draw_spaces)
    {
      codeslayer_registry_set_boolean (priv->registry, 
                                       CODESLAYER_REGISTRY_DRAW_SPACES,
                                       FALSE);
      registry_changed_action (engine);
    }
  else
    {
      codeslayer_registry_set_boolean (priv->registry, 
                                       CODESLAYER_REGISTRY_DRAW_SPACES,
                                       TRUE);
      registry_changed_action (engine);
    }
}

static void
word_wrap_action (CodeSlayerEngine *engine)
{
  CodeSlayerEnginePrivate *priv;
  gboolean word_wrap;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  
  word_wrap = codeslayer_registry_get_boolean (priv->registry, 
                                               CODESLAYER_REGISTRY_WORD_WRAP);
                                                   
  if (word_wrap)
    {
      codeslayer_registry_set_boolean (priv->registry, 
                                       CODESLAYER_REGISTRY_WORD_WRAP,
                                       FALSE);
      registry_changed_action (engine);
    }
  else
    {
      codeslayer_registry_set_boolean (priv->registry, 
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
      GtkWidget *editor;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      codeslayer_editor_sync_registry (CODESLAYER_EDITOR (editor));
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
      GtkWidget *editor;
      editor = codeslayer_notebook_get_active_editor (CODESLAYER_NOTEBOOK (priv->notebook));
      if (!codeslayer_editor_scroll_to_line (CODESLAYER_EDITOR (editor), atoi(string)))
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
  CodeSlayerProfile *profile;
  
  priv = CODESLAYER_ENGINE_GET_PRIVATE (engine);
  profile = codeslayer_profile_handler_get_profile (priv->profile_handler);
  
  codeslayer_plugins_run_dialog (priv->plugins, profile);
}
