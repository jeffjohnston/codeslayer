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
#include "encoding.h"
#include <gtksourceview/gtksource.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-notebook-tab.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-sourceview.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-registry.h>

/**
 * SECTION:codeslayer-notebook
 * @short_description: Contains the pages.
 * @title: CodeSlayerNotebook
 * @include: codeslayer/codeslayer-notebook.h
 *
 * A notebook which contains pages of documents to edit.
 */

static void codeslayer_notebook_class_init  (CodeSlayerNotebookClass *klass);
static void codeslayer_notebook_init        (CodeSlayerNotebook      *notebook);
static void codeslayer_notebook_finalize    (CodeSlayerNotebook      *notebook);
static void select_editor_action            (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_editor_action             (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_all_editors_action        (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_other_editors_action      (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_right_editors_action      (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_left_editors_action       (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void buffer_modified_action          (GtkTextBuffer           *buffer, 
                                             CodeSlayerNotebook      *notebook);
static void save_as_dialog                  (CodeSlayerNotebook      *notebook, 
                                             GtkWidget               *notebook_page, 
                                             CodeSlayerDocument      *document);
static void registry_changed_action         (CodeSlayerNotebook      *notebook);
static GtkWidget* save_editor               (CodeSlayerNotebook      *notebook, 
                                             gint                     page_num);
static void get_dirty_buffer_pages          (CodeSlayerNotebook      *notebook, 
                                             gint                     page,
                                             GList                   **dirty_pages);
static gboolean resolve_dirty_buffers       (CodeSlayerNotebook      *notebook, 
                                             GList                   *dirty_pages);

#define CODESLAYER_NOTEBOOK_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_TYPE, CodeSlayerNotebookPrivate))

typedef struct _CodeSlayerNotebookPrivate CodeSlayerNotebookPrivate;

struct _CodeSlayerNotebookPrivate
{
  GtkWindow         *window;
  CodeSlayerProfile *profile;
};

enum
{
  SELECT_EDITOR,
  EDITOR_SAVED,
  EDITORS_ALL_SAVED,
  LAST_SIGNAL
};

static guint codeslayer_notebook_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerNotebook, codeslayer_notebook, GTK_TYPE_NOTEBOOK)

static void
codeslayer_notebook_class_init (CodeSlayerNotebookClass *klass)
{
  /**
   * CodeSlayerNotebook::select-editor
   * @codeslayernotebook: the notebook that received the signal
   *
   * The ::editor-saved signal is emitted when an editor is saved successfully
   */
  codeslayer_notebook_signals[SELECT_EDITOR] =
    g_signal_new ("select-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookClass, select_editor), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
                  
  /**
   * CodeSlayerNotebook::editor-saved
   * @codeslayernotebook: the notebook that received the signal
   *
   * The ::editor-saved signal is emitted when an editor is saved successfully
   */
  codeslayer_notebook_signals[EDITOR_SAVED] =
    g_signal_new ("editor-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookClass, editor_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayerNotebook::editors-all-saved
   * @codeslayernotebook: the notebook that received the signal
   * @editors: a #GList of #CodeSlayerSourceView objects that were saved
   *
   * The ::editors-all-saved signal is emitted when all the editors have been saved successfully
   */
  codeslayer_notebook_signals[EDITORS_ALL_SAVED] =
    g_signal_new ("editors-all-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookClass, editors_all_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_POINTER);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_notebook_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerNotebookPrivate));
}

static void
codeslayer_notebook_init (CodeSlayerNotebook *notebook)
{
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
}

static void
codeslayer_notebook_finalize (CodeSlayerNotebook *notebook)
{
  G_OBJECT_CLASS (codeslayer_notebook_parent_class)->finalize (G_OBJECT (notebook));
}

/**
 * codeslayer_notebook_new:
 * @window: a #GtkWindow.
 * @registry: a #CodeSlayerRegistry.
 *
 * Creates a new #CodeSlayerNotebook.
 *
 * Returns: a new #CodeSlayerNotebook. 
 */
GtkWidget*
codeslayer_notebook_new (GtkWindow         *window, 
                         CodeSlayerProfile *profile)
{
  CodeSlayerNotebookPrivate *priv;
  GtkWidget *notebook;
  CodeSlayerRegistry *registry;
  
  notebook = g_object_new (codeslayer_notebook_get_type (), NULL);
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);
  priv->profile = profile;
  priv->window = window;
  
  registry = codeslayer_profile_get_registry (profile);
  
  g_signal_connect_swapped (G_OBJECT (registry), "registry-initialized",
                            G_CALLBACK (registry_changed_action), CODESLAYER_NOTEBOOK (notebook));
  
  g_signal_connect_swapped (G_OBJECT (registry), "registry-changed",
                            G_CALLBACK (registry_changed_action), CODESLAYER_NOTEBOOK (notebook));
  
  return notebook;
}

/**
 * codeslayer_notebook_add_document:
 * @notebook: a #CodeSlayerNotebook.
 * @document: a #CodeSlayerDocument.
 * 
 * Add a new page to the notebook based on the document. This method is 
 * for internal use only.
 */
void
codeslayer_notebook_add_document (CodeSlayerNotebook *notebook,
                                  CodeSlayerDocument *document)
{
  CodeSlayerNotebookPrivate *priv;
  const gchar *file_path;
  const gchar *name = NULL;
  GtkTextBuffer *buffer;
  GtkWidget *source_view;
  GtkWidget *notebook_page;
  GtkWidget *notebook_tab;
  gint page_num;
  gchar *contents;
  gint line_number;
  GTimeVal *modification_time;
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);
  
  /* create page, source view and buffer */

  source_view = codeslayer_source_view_new (priv->window, document, priv->profile);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(source_view));

  file_path = codeslayer_document_get_file_path (document);
  name = codeslayer_document_get_name (document);
  
  if (file_path != NULL)
    {
      contents = codeslayer_utils_get_utf8_text (file_path);
      if (contents != NULL)
        {
          gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (buffer));
          codeslayer_source_view_set_text (CODESLAYER_SOURCE_VIEW (source_view), contents);
          gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (buffer));
          gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (buffer), FALSE);
          g_free (contents);
        }
      
      modification_time = codeslayer_utils_get_modification_time (file_path);
      codeslayer_source_view_set_modification_time (CODESLAYER_SOURCE_VIEW (source_view), modification_time);
    }

  notebook_page = codeslayer_notebook_page_new (source_view);

  /* create tab */
  
  notebook_tab = codeslayer_notebook_tab_new (GTK_WIDGET (notebook), name);
  codeslayer_notebook_tab_set_notebook_page (CODESLAYER_NOTEBOOK_TAB (notebook_tab), 
                                             notebook_page);

  g_signal_connect (G_OBJECT (notebook_tab), "select-editor",
                    G_CALLBACK (select_editor_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-editor",
                    G_CALLBACK (close_editor_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-all-editors",
                    G_CALLBACK (close_all_editors_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-other-editors",
                    G_CALLBACK (close_other_editors_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-right-editors",
                    G_CALLBACK (close_right_editors_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-left-editors",
                    G_CALLBACK (close_left_editors_action), notebook);

  page_num = gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                                       GTK_WIDGET (notebook_page),
                                       GTK_WIDGET (notebook_tab));
  gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (notebook),
                                    GTK_WIDGET (notebook_page), TRUE);

  g_signal_connect (G_OBJECT (buffer), "modified-changed",
                    G_CALLBACK (buffer_modified_action), notebook);

  gtk_widget_show_all (GTK_WIDGET (notebook_tab));
  gtk_widget_show_all (GTK_WIDGET (notebook_page));

  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page_num);
  gtk_widget_grab_focus (source_view);

  line_number = codeslayer_document_get_line_number (document);
  if (line_number > 0)
    codeslayer_source_view_scroll_to_line (CODESLAYER_SOURCE_VIEW (source_view), line_number);
}

/**
 * codeslayer_notebook_select_document:
 * @notebook: a #CodeSlayerNotebook.
 * @document: a #CodeSlayerDocument.
 *
 * Returns: is TRUE if the document is able to be found. 
 */
gboolean
codeslayer_notebook_select_document (CodeSlayerNotebook *notebook, 
                                     CodeSlayerDocument *document)
{
  gint pages;
  gint page;
  const gchar *file_path;
  
  file_path = codeslayer_document_get_file_path (document);

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
              GtkWidget *source_view;
              source_view = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
              codeslayer_source_view_scroll_to_line (CODESLAYER_SOURCE_VIEW (source_view), line_number);
            }

          return TRUE;
        }
    }
    
  return FALSE;    
}

/**
 * codeslayer_notebook_save_document:
 * @notebook: a #CodeSlayerNotebook.
 * @page_num: the notebook page to save.
 */
void
codeslayer_notebook_save_document (CodeSlayerNotebook *notebook, 
                                   gint                page_num)
{
  GList *editors = NULL;
  GtkWidget *editor;
  
  editor = save_editor (notebook, page_num);
  
  if (editor != NULL)
    editors = g_list_prepend (editors, editor);
  
  if (editors != NULL)
    {
      g_signal_emit_by_name((gpointer)notebook, "editors-all-saved", editors);
      g_list_free (editors);
    }
}

/**
 * codeslayer_notebook_save_all_documents:
 * @notebook: a #CodeSlayerNotebook.
 */
void
codeslayer_notebook_save_all_documents (CodeSlayerNotebook *notebook)
{
  GList *editors = NULL;
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (i = 0; i < pages; i++)
    {
      GtkWidget *editor;
      editor = save_editor (notebook, i);
      if (editor != NULL)
        editors = g_list_prepend (editors, editor);
    }
  
  if (editors != NULL)
    {
      editors = g_list_reverse (editors);
      g_signal_emit_by_name((gpointer)notebook, "editors-all-saved", editors);
      g_list_free (editors);
    }
}

static GtkWidget*
save_editor (CodeSlayerNotebook *notebook, 
             gint                page_num)
{
  GtkWidget *notebook_page;
  GtkWidget *editor = NULL;
  GtkTextBuffer *buffer;
  GtkWidget *notebook_tab;
  
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);

  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  if (gtk_text_buffer_get_modified (buffer))
    {
      CodeSlayerDocument *document;
      const gchar *file_path;
      GTimeVal *modification_time;
      gchar *contents;
      GtkTextIter start;
      GtkTextIter end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);

      contents = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

      document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      file_path = codeslayer_document_get_file_path (document);
      
      if (file_path == NULL)
        {
          save_as_dialog (notebook, notebook_page, document);
          file_path = codeslayer_document_get_file_path (document);
        }

      if (file_path != NULL)
        {
          if (!g_file_set_contents (file_path, contents, -1, NULL))
            {
              g_free (contents);
              return NULL;
            }
          g_free (contents);
          
          modification_time = codeslayer_utils_get_modification_time (file_path);
          codeslayer_source_view_set_modification_time (CODESLAYER_SOURCE_VIEW (editor), modification_time);
              
          g_signal_emit_by_name((gpointer)notebook, "editor-saved", editor);          
          gtk_text_buffer_set_modified (buffer, FALSE);
        }      
    }

  notebook_tab = gtk_notebook_get_tab_label (GTK_NOTEBOOK (notebook),
                                             GTK_WIDGET (notebook_page));
  codeslayer_notebook_tab_show_buffer_clean (CODESLAYER_NOTEBOOK_TAB (notebook_tab));
  
  return editor;
}

static void
save_as_dialog (CodeSlayerNotebook *notebook, 
                GtkWidget          *notebook_page, 
                CodeSlayerDocument *document)
{
  CodeSlayerNotebookPrivate *priv;
  GtkWidget *dialog;
  gint response;
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);
  
  dialog = gtk_file_chooser_dialog_new (_("Save As"), 
                                        GTK_WINDOW (priv->window),
                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_SAVE, GTK_RESPONSE_OK, 
                                        NULL);
                                        
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      gchar *file_path;
      GtkWidget *notebook_label;
      gchar *basename;

      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      basename = g_path_get_basename (file_path);

      codeslayer_document_set_file_path (document, file_path);
      
      notebook_label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (notebook), 
                                                   notebook_page);
      codeslayer_notebook_tab_set_label_name (CODESLAYER_NOTEBOOK_TAB (notebook_label),
                                              basename);

      g_object_unref (file);
      g_free (file_path);
      g_free (basename);
    }
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

/**
 * codeslayer_notebook_close_document:
 * @notebook: a #CodeSlayerNotebook.
 * @page_num: the notebook page to close. Pages begin with 0 starting 
 *            from the left.
 *
 * Returns: is TRUE unless the editor needs to be saved. If the editor needs 
 *          to be saved then the page will not be closed and this method 
 *          will return FALSE.
 */
gboolean
codeslayer_notebook_close_document (CodeSlayerNotebook *notebook, 
                                    gint                page_num)
{
  gboolean result = FALSE;
  GList *dirty_pages = NULL;
  
  get_dirty_buffer_pages (notebook, page_num, &dirty_pages);

  if (resolve_dirty_buffers (notebook, dirty_pages))
    {
      gint current_page_num;
      current_page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));

      gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);
      
      if (current_page_num == page_num)
        g_signal_emit_by_name((gpointer)notebook, "select-editor", --page_num);

      result = TRUE;
    }

  g_list_foreach (dirty_pages, (GFunc)g_free, NULL);
  g_list_free (dirty_pages);

  return result;
}

/**
 * codeslayer_notebook_has_unsaved_documents:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Returns: is FALSE unless there are editors that need to saved.
 */
gboolean
codeslayer_notebook_has_unsaved_documents (CodeSlayerNotebook *notebook)
{
  gboolean result = TRUE;
  GList *dirty_pages = NULL;
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (i = 0; i < pages; i++)
    get_dirty_buffer_pages (notebook, i, &dirty_pages);
  
  if (resolve_dirty_buffers (notebook, dirty_pages))
    result = FALSE;

  g_list_foreach (dirty_pages, (GFunc)g_free, NULL);
  g_list_free (dirty_pages);

  return result;
}

/**
 * codeslayer_notebook_close_all_documents:
 * @notebook: a #CodeSlayerNotebook.
 */
void
codeslayer_notebook_close_all_documents (CodeSlayerNotebook *notebook)
{
  gint pages;
  gint page_num;
  GtkWidget *notebook_page;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);

  while (pages > 1)
    {
      gint page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), GTK_WIDGET (notebook_page));
      if (page != 0)
          gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), 0);
      else
          gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), -1);
      pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    }

  gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), 0);
}

/**
 * codeslayer_notebook_get_active_document:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Returns: the active editor.
 */
GtkWidget*
codeslayer_notebook_get_active_document (CodeSlayerNotebook *notebook)
{
  gint page_num;
  GtkWidget *notebook_page;

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 
                                             page_num);
  
  if (notebook_page == NULL)
    return NULL;

  return codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
}

/**
 * codeslayer_notebook_get_all_documents:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Returns: a #GList of #CodeSlayerSourceView. Note: you need to call g_list_free
 * when you are done with the list.
 */
GList*
codeslayer_notebook_get_all_documents (CodeSlayerNotebook *notebook)
{
  GList *results = NULL;
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
  
  for (i = 0; i < pages; i++)
    {
      GtkWidget *notebook_page;
      GtkWidget *editor; 
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i);
      editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      results = g_list_prepend (results, editor);
    }
    
  return g_list_reverse (results);
}

static void
select_editor_action (CodeSlayerNotebookTab *notebook_tab,
                      CodeSlayerNotebook    *notebook)
{
  GtkWidget *notebook_page;
  gint page;
  
  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                GTK_WIDGET (notebook_page));
  g_signal_emit_by_name((gpointer)notebook, "select-editor", page);
}

static void
close_editor_action (CodeSlayerNotebookTab *notebook_tab,
                     CodeSlayerNotebook    *notebook)
{
  GtkWidget *notebook_page;
  gint page;
  
  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                GTK_WIDGET (notebook_page));
  codeslayer_notebook_close_document (notebook, page);
}

static void
close_all_editors_action (CodeSlayerNotebookTab *notebook_tab,
                          CodeSlayerNotebook    *notebook)
{
  if (!codeslayer_notebook_has_unsaved_documents (notebook))
    codeslayer_notebook_close_all_documents (notebook);
}

static void
close_other_editors_action (CodeSlayerNotebookTab *notebook_tab,
                            CodeSlayerNotebook    *notebook)
{
  GList *dirty_pages = NULL;
  gint pages;
  gint page;
  gint i;
  GtkWidget *notebook_page;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), GTK_WIDGET (notebook_page));

  for (i = 0; i < pages; i++)
    {
      if (i != page)
        get_dirty_buffer_pages (notebook, i, &dirty_pages);
    }
    
  if (resolve_dirty_buffers (notebook, dirty_pages))
    {
      while (pages > 1)
        {
          page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                        GTK_WIDGET (notebook_page));
          if (page != 0) 
            gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), 0);
          else 
            gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), -1);
          
          pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
        }
    }
    
  g_list_foreach (dirty_pages, (GFunc)g_free, NULL);
  g_list_free (dirty_pages);
}

static void
close_right_editors_action (CodeSlayerNotebookTab *notebook_tab,
                            CodeSlayerNotebook    *notebook)
{
  GList *dirty_pages = NULL;
  gint pages;
  gint page;
  gint i;
  GtkWidget *notebook_page;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), GTK_WIDGET (notebook_page));

  for (i = page; i < pages; i++)
    get_dirty_buffer_pages (notebook, i, &dirty_pages);
  
  if (resolve_dirty_buffers (notebook, dirty_pages))
    {
      while (pages > page + 1)
        {
          gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), -1);
          pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
        }
    }

  g_list_foreach (dirty_pages, (GFunc)g_free, NULL);
  g_list_free (dirty_pages);
}

static void
close_left_editors_action (CodeSlayerNotebookTab *notebook_tab,
                           CodeSlayerNotebook    *notebook)
{
  GList *dirty_pages = NULL;
  GtkWidget *notebook_page;
  gint page;
  gint i;
  
  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), GTK_WIDGET (notebook_page));

  for (i = 0; i < page; i++)
    get_dirty_buffer_pages (notebook, i, &dirty_pages);

  if (resolve_dirty_buffers (notebook, dirty_pages)) 
    {
      while (page != 0)
        {
          gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), 0);
          page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                        GTK_WIDGET (notebook_page));
        }
    }
    
  g_list_foreach (dirty_pages, (GFunc)g_free, NULL);
  g_list_free (dirty_pages);
}

static void
get_dirty_buffer_pages (CodeSlayerNotebook *notebook, 
                        gint                page,
                        GList              **dirty_pages)
{
  GtkWidget *notebook_page;
  GtkWidget *editor;
  GtkTextBuffer *buffer;

  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  if (gtk_text_buffer_get_modified (buffer))
    {
      gint *dirty_page = malloc (sizeof (gint));
      *dirty_page = page;
      *dirty_pages = g_list_append (*dirty_pages, dirty_page);      
    }
}

static gboolean
resolve_dirty_buffers (CodeSlayerNotebook *notebook, 
                       GList              *dirty_pages)
{
  CodeSlayerNotebookPrivate *priv;
  guint dirty_pages_length;
  
  dirty_pages_length = g_list_length (dirty_pages);
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);

  while (dirty_pages != NULL)
    {
      gint *page = dirty_pages->data;

      GtkWidget *notebook_page;
      CodeSlayerDocument *document;
      const gchar *name;
      gchar *text;
      GtkWidget *dialog;
      gint response;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), *page);
      document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      name = codeslayer_document_get_name (document);
      
      text = g_strdup_printf (_("Save changes to %s?"), name);

      dialog = gtk_message_dialog_new_with_markup (priv->window, GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_WARNING,
                                                   GTK_BUTTONS_NONE, text, NULL);
      gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_NO, GTK_RESPONSE_NO);

      if (dirty_pages_length > 1)
        gtk_dialog_add_button (GTK_DIALOG (dialog), _("No To All"), 100);

      gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
      gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_YES, GTK_RESPONSE_YES);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

      g_free (text);

      response = gtk_dialog_run (GTK_DIALOG (dialog));
      if (response == GTK_RESPONSE_YES)
        {
          codeslayer_notebook_save_document (notebook, *page);
        }
      else if (response == GTK_RESPONSE_CANCEL)
        {
          gtk_widget_destroy (dialog);
          return FALSE;
        }
      else if (response == 100)
        {
          gtk_widget_destroy (dialog);
          return TRUE;
        }
      gtk_widget_destroy (dialog);

      dirty_pages = g_list_next (dirty_pages);
    }

  return TRUE;
}

static void
buffer_modified_action (GtkTextBuffer      *buffer, 
                        CodeSlayerNotebook *notebook)
{
  gint page_num;
  GtkWidget *notebook_page;
  GtkWidget *notebook_tab;

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));

  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);

  notebook_tab = gtk_notebook_get_tab_label (GTK_NOTEBOOK (notebook),
                                             GTK_WIDGET (notebook_page));
                                             
  if (gtk_text_buffer_get_modified (buffer))
    codeslayer_notebook_tab_show_buffer_dirty (CODESLAYER_NOTEBOOK_TAB (notebook_tab));
  else
    codeslayer_notebook_tab_show_buffer_clean (CODESLAYER_NOTEBOOK_TAB (notebook_tab));
}

static void
registry_changed_action (CodeSlayerNotebook *notebook)
{
  CodeSlayerNotebookPrivate *priv;
  CodeSlayerRegistry *registry;
  gchar *editor_value;
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  editor_value = codeslayer_registry_get_string (registry, CODESLAYER_REGISTRY_EDITOR_TAB_POSITION);

  if (g_strcmp0 (editor_value, "left") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
  else if (g_strcmp0 (editor_value, "right") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_RIGHT);
  else if (g_strcmp0 (editor_value, "top") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  else if (g_strcmp0 (editor_value, "bottom") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);
    
  if (editor_value)                                             
    g_free (editor_value);
}                                   
