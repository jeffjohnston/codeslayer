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
static void select_document_action          (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_document_action           (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_all_documents_action      (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_other_documents_action    (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_right_documents_action    (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void close_left_documents_action     (CodeSlayerNotebookTab   *notebook_tab, 
                                             CodeSlayerNotebook      *notebook);
static void buffer_modified_action          (GtkTextBuffer           *buffer, 
                                             CodeSlayerNotebook      *notebook);
static void registry_changed_action         (CodeSlayerNotebook      *notebook);
static void save_as_dialog                  (CodeSlayerNotebook      *notebook, 
                                             GtkWidget               *notebook_page, 
                                             CodeSlayerDocument      *document);
static GtkWidget* save_document             (CodeSlayerNotebook      *notebook, 
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
  SELECT_DOCUMENT,
  DOCUMENT_SAVED,
  DOCUMENTS_ALL_SAVED,
  SYNC_NOTEBOOK,
  LAST_SIGNAL
};

static guint codeslayer_notebook_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerNotebook, codeslayer_notebook, GTK_TYPE_NOTEBOOK)

static void
codeslayer_notebook_class_init (CodeSlayerNotebookClass *klass)
{
  /**
   * CodeSlayerNotebook::select-document
   * @codeslayernotebook: the notebook that received the signal
   *
   * The ::document-saved signal is emitted when an document is saved successfully
   */
  codeslayer_notebook_signals[SELECT_DOCUMENT] =
    g_signal_new ("select-document", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookClass, select_document), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
                  
  /**
   * CodeSlayerNotebook::document-saved
   * @codeslayernotebook: the notebook that received the signal
   *
   * The ::document-saved signal is emitted when an document is saved successfully
   */
  codeslayer_notebook_signals[DOCUMENT_SAVED] =
    g_signal_new ("document-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookClass, document_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayerNotebook::documents-all-saved
   * @codeslayernotebook: the notebook that received the signal
   * @documents: a #GList of #CodeSlayerSourceView objects that were saved
   *
   * The ::documents-all-saved signal is emitted when all the documents have been saved successfully
   */
  codeslayer_notebook_signals[DOCUMENTS_ALL_SAVED] =
    g_signal_new ("documents-all-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookClass, documents_all_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_POINTER);
                  
  /**
   * CodeSlayerNotebook::sync-notebook
   * @codeslayernotebook: the notebook that received the signal
   *
   * The ::sync-notebook signal is emitted when the notebook should sync up.
   */
  codeslayer_notebook_signals[SYNC_NOTEBOOK] =
    g_signal_new ("sync-notebook", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerNotebookClass, sync_notebook),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

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
 * @profile: a #CodeSlayerProfile.
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
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);
  
  /* create page and source view */

  source_view = codeslayer_source_view_new (priv->window, document, priv->profile);
  notebook_page = codeslayer_notebook_page_new (source_view);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(source_view));
  file_path = codeslayer_document_get_file_path (document);
  name = codeslayer_document_get_name (document);

  /* create tab */
  
  notebook_tab = codeslayer_notebook_tab_new (GTK_WIDGET (notebook), name);
  codeslayer_notebook_tab_set_notebook_page (CODESLAYER_NOTEBOOK_TAB (notebook_tab), 
                                             notebook_page);

  g_signal_connect (G_OBJECT (notebook_tab), "select-document",
                    G_CALLBACK (select_document_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-document",
                    G_CALLBACK (close_document_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-all-documents",
                    G_CALLBACK (close_all_documents_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-other-documents",
                    G_CALLBACK (close_other_documents_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-right-documents",
                    G_CALLBACK (close_right_documents_action), notebook);
  g_signal_connect (G_OBJECT (notebook_tab), "close-left-documents",
                    G_CALLBACK (close_left_documents_action), notebook);

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
  
  if (file_path != NULL)
    {
      codeslayer_notebook_page_load_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      codeslayer_profile_add_recent_document (priv->profile, file_path);
      codeslayer_profile_recent_document_changed (priv->profile);
    }
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
      GtkWidget *source_view;
      CodeSlayerDocument *current_document;
      const gchar *current_file_path;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      current_document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view));
      current_file_path = codeslayer_document_get_file_path (current_document);

      if (g_strcmp0 (current_file_path, file_path) == 0)
        {
          gint line_number;
        
          gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);

          line_number = codeslayer_document_get_line_number (document);
          if (line_number > 0)
            {
              GtkWidget *source_view;
              source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
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
  GList *source_views = NULL;
  GtkWidget *source_view;
  
  source_view = save_document (notebook, page_num);
  
  if (source_view != NULL)
    source_views = g_list_prepend (source_views, source_view);
  
  if (source_views != NULL)
    {
      g_signal_emit_by_name((gpointer)notebook, "documents-all-saved", source_views);
      g_list_free (source_views);
    }
}

/**
 * codeslayer_notebook_save_all_documents:
 * @notebook: a #CodeSlayerNotebook.
 */
void
codeslayer_notebook_save_all_documents (CodeSlayerNotebook *notebook)
{
  GList *source_views = NULL;
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (i = 0; i < pages; i++)
    {
      GtkWidget *source_view;
      source_view = save_document (notebook, i);
      if (source_view != NULL)
        source_views = g_list_prepend (source_views, source_view);
    }
  
  if (source_views != NULL)
    {
      source_views = g_list_reverse (source_views);
      g_signal_emit_by_name((gpointer)notebook, "documents-all-saved", source_views);
      g_list_free (source_views);
    }
}

static GtkWidget*
save_document (CodeSlayerNotebook *notebook, 
               gint                page_num)
{
  GtkWidget *notebook_page;
  GtkWidget *source_view = NULL;
  GtkTextBuffer *buffer;
  GtkWidget *notebook_tab;
  
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);

  source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  if (gtk_text_buffer_get_modified (buffer))
    {
      CodeSlayerDocument *document;
      const gchar *file_path;
      GTimeVal *modification_time;

      document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view));
      file_path = codeslayer_document_get_file_path (document);
      
      if (file_path == NULL)
        {
          save_as_dialog (notebook, notebook_page, document);
          file_path = codeslayer_document_get_file_path (document);
        }

      if (file_path != NULL)
        {
          if (!codeslayer_notebook_page_save_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page)))
            return NULL;
          
          modification_time = codeslayer_utils_get_modification_time (file_path);
          codeslayer_source_view_set_modification_time (CODESLAYER_SOURCE_VIEW (source_view), modification_time);
              
          g_signal_emit_by_name((gpointer)notebook, "document-saved", source_view);          
          gtk_text_buffer_set_modified (buffer, FALSE);
        }      
    }

  notebook_tab = gtk_notebook_get_tab_label (GTK_NOTEBOOK (notebook),
                                             GTK_WIDGET (notebook_page));
  codeslayer_notebook_tab_show_buffer_clean (CODESLAYER_NOTEBOOK_TAB (notebook_tab));
  
  return source_view;
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
                                        _("Cancel"), GTK_RESPONSE_CANCEL,
                                        _("Save"), GTK_RESPONSE_OK, 
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
 * Returns: is TRUE unless the document needs to be saved. If the document needs 
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
        g_signal_emit_by_name((gpointer)notebook, "select-document", --page_num);

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
 * Returns: is FALSE unless there are documents that need to saved.
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
 * codeslayer_notebook_has_open_documents:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Returns: is TRUE if there are documents open in the notebook.
 */
gboolean
codeslayer_notebook_has_open_documents (CodeSlayerNotebook *notebook)
{
  gint pages;
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
  return pages > 0;
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
 * codeslayer_notebook_get_active_source_view:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Returns: the active source view.
 */
GtkWidget*
codeslayer_notebook_get_active_source_view (CodeSlayerNotebook *notebook)
{
  gint page_num;
  GtkWidget *notebook_page;

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 
                                             page_num);
  
  if (notebook_page == NULL)
    return NULL;

  return codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
}

/**
 * codeslayer_notebook_get_all_source_views:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Returns: a #GList of #CodeSlayerSourceView. Note: you need to call g_list_free
 * when you are done with the list.
 */
GList*
codeslayer_notebook_get_all_source_views (CodeSlayerNotebook *notebook)
{
  GList *results = NULL;
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
  
  for (i = 0; i < pages; i++)
    {
      GtkWidget *notebook_page;
      GtkWidget *source_view; 
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i);
      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      results = g_list_prepend (results, source_view);
    }
    
  return g_list_reverse (results);
}

static void
select_document_action (CodeSlayerNotebookTab *notebook_tab,
                        CodeSlayerNotebook    *notebook)
{
  GtkWidget *notebook_page;
  gint page;
  
  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                GTK_WIDGET (notebook_page));
  g_signal_emit_by_name((gpointer)notebook, "select-document", page);
}

static void
close_document_action (CodeSlayerNotebookTab *notebook_tab,
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
close_all_documents_action (CodeSlayerNotebookTab *notebook_tab,
                            CodeSlayerNotebook    *notebook)
{
  if (!codeslayer_notebook_has_unsaved_documents (notebook))
    codeslayer_notebook_close_all_documents (notebook);
}

static void
close_other_documents_action (CodeSlayerNotebookTab *notebook_tab,
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
close_right_documents_action (CodeSlayerNotebookTab *notebook_tab,
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
close_left_documents_action (CodeSlayerNotebookTab *notebook_tab,
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
  GtkWidget *source_view;
  GtkTextBuffer *buffer;

  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
  source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
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
      GtkWidget *source_view;
      CodeSlayerDocument *document;
      const gchar *name;
      gchar *text;
      GtkWidget *dialog;
      gint response;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), *page);
      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view));
      name = codeslayer_document_get_name (document);
      
      text = g_strdup_printf (_("Save changes to %s?"), name);

      dialog = gtk_message_dialog_new_with_markup (priv->window, GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_WARNING,
                                                   GTK_BUTTONS_NONE, text, NULL);
      gtk_dialog_add_button (GTK_DIALOG (dialog), _("No"), GTK_RESPONSE_NO);

      if (dirty_pages_length > 1)
        gtk_dialog_add_button (GTK_DIALOG (dialog), _("No To All"), 100);

      gtk_dialog_add_button (GTK_DIALOG (dialog), _("Cancel"), GTK_RESPONSE_CANCEL);
      gtk_dialog_add_button (GTK_DIALOG (dialog), _("Yes"), GTK_RESPONSE_YES);
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
  gchar *source_view_value;
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  source_view_value = codeslayer_registry_get_string (registry, CODESLAYER_REGISTRY_TAB_POSITION);

  if (g_strcmp0 (source_view_value, "left") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
  else if (g_strcmp0 (source_view_value, "right") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_RIGHT);
  else if (g_strcmp0 (source_view_value, "top") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  else if (g_strcmp0 (source_view_value, "bottom") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);
    
  if (source_view_value)                                             
    g_free (source_view_value);
}
