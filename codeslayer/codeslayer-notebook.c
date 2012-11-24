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

#include "encoding.h"
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-notebook-tab.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-utils.h>

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
static gboolean has_clean_buffer            (CodeSlayerNotebook      *notebook, 
                                             gint                     page);
static void preferences_changed_action      (CodeSlayerNotebook      *notebook);
static GtkWidget* save_editor               (CodeSlayerNotebook      *notebook, 
                                             gint                     page_num);

#define CODESLAYER_NOTEBOOK_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_TYPE, CodeSlayerNotebookPrivate))

typedef struct _CodeSlayerNotebookPrivate CodeSlayerNotebookPrivate;

struct _CodeSlayerNotebookPrivate
{
  GtkWindow             *window;
  CodeSlayerPreferences *preferences;
  CodeSlayerSettings    *settings;
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
   * @editors: a #GList of #CodeSlayerEditor objects that were saved
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
 * @preferences: a #CodeSlayerPreferences.
 * @settings: a #CodeSlayerSettings.
 *
 * Creates a new #CodeSlayerNotebook.
 *
 * Returns: a new #CodeSlayerNotebook. 
 */
GtkWidget*
codeslayer_notebook_new (GtkWindow             *window, 
                         CodeSlayerPreferences *preferences,
                         CodeSlayerSettings    *settings)
{
  CodeSlayerNotebookPrivate *priv;
  GtkWidget *notebook;
  notebook = g_object_new (codeslayer_notebook_get_type (), NULL);
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);
  priv->preferences = preferences;
  priv->settings = settings;
  priv->window = window;
  
  g_signal_connect_swapped (G_OBJECT (preferences), "initialize-preferences",
                            G_CALLBACK (preferences_changed_action), CODESLAYER_NOTEBOOK (notebook));
  
  g_signal_connect_swapped (G_OBJECT (preferences), "notebook-preferences-changed",
                            G_CALLBACK (preferences_changed_action), CODESLAYER_NOTEBOOK (notebook));
  
  return notebook;
}

/**
 * codeslayer_notebook_add_editor:
 * @notebook: a #CodeSlayerNotebook.
 * @document: a #CodeSlayerDocument.
 * 
 * Add a new page to the notebook based on the document. This method is 
 * for internal use only. The correct way for  plugins to add a page is to call 
 * the codeslayer_projects_select_document() method. That will 
 * select the document in the tree and then call this method to add the page 
 * to the notebook.
 */
void
codeslayer_notebook_add_editor (CodeSlayerNotebook *notebook,
                                CodeSlayerDocument *document)
{
  CodeSlayerNotebookPrivate *priv;
  const gchar *file_path;
  gchar *file_name;
  GtkTextBuffer *buffer;
  GtkWidget *editor;
  GtkWidget *notebook_page;
  GtkWidget *notebook_tab;
  gint page_num;
  gchar *contents;
  gint line_number;
  GTimeVal *modification_time;
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);

  /* create page, editor and buffer */

  file_path = codeslayer_document_get_file_path (document);
  file_name = g_path_get_basename (file_path);

  editor = codeslayer_editor_new (priv->window, document, priv->preferences, priv->settings);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(editor));

  contents = codeslayer_utils_get_utf8_text (file_path);
  if (contents != NULL)
    {
      gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (buffer));
      gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), contents, -1);
      gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (buffer));
      gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (buffer), FALSE);
      g_free (contents);
    }
  
  modification_time = codeslayer_utils_get_modification_time (file_path);
  codeslayer_editor_set_modification_time (CODESLAYER_EDITOR (editor), modification_time);

  notebook_page = codeslayer_notebook_page_new (editor);

  /* create tab */

  notebook_tab = codeslayer_notebook_tab_new (GTK_WIDGET (notebook), file_name);
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

  g_free (file_name);

  gtk_widget_show_all (GTK_WIDGET (notebook_tab));
  gtk_widget_show_all (GTK_WIDGET (notebook_page));

  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page_num);
  gtk_widget_grab_focus (editor);

  line_number = codeslayer_document_get_line_number (document);
  if (line_number > 0)
    codeslayer_editor_scroll_to_line (CODESLAYER_EDITOR (editor), line_number);
}

/**
 * codeslayer_notebook_save_editor:
 * @notebook: a #CodeSlayerNotebook.
 * @page_num: the notebook page to save. Pages begin with 0 starting 
 *                        from the left.
 */
void
codeslayer_notebook_save_editor (CodeSlayerNotebook *notebook, 
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
 * codeslayer_notebook_save_all_editors:
 * @notebook: a #CodeSlayerNotebook.
 */
void
codeslayer_notebook_save_all_editors (CodeSlayerNotebook *notebook)
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

      if (!g_file_set_contents (file_path, contents, -1, NULL))
        {
          g_free (contents);
          return NULL;
        }
      g_free (contents);
      
      modification_time = codeslayer_utils_get_modification_time (file_path);
      codeslayer_editor_set_modification_time (CODESLAYER_EDITOR (editor), modification_time);
          
      g_signal_emit_by_name((gpointer)notebook, "editor-saved", editor);          
      gtk_text_buffer_set_modified (buffer, FALSE);
    }

  notebook_tab = gtk_notebook_get_tab_label (GTK_NOTEBOOK (notebook),
                                             GTK_WIDGET (notebook_page));
  codeslayer_notebook_tab_show_buffer_clean (CODESLAYER_NOTEBOOK_TAB (notebook_tab));
  
  return editor;
}

/**
 * codeslayer_notebook_close_editor:
 * @notebook: a #CodeSlayerNotebook.
 * @page_num: the notebook page to close. Pages begin with 0 starting 
 *            from the left.
 *
 * Returns: is TRUE unless the editor needs to be saved. If the editor needs 
 *          to be saved then the page will not be closed and this method 
 *          will return FALSE.
 */
gboolean
codeslayer_notebook_close_editor (CodeSlayerNotebook *notebook, 
                                  gint                page_num)
{
  gint current_page_num;

  if (!has_clean_buffer (notebook, page_num))
      return FALSE;

  current_page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));

  gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);
  
  if (current_page_num == page_num)
    g_signal_emit_by_name((gpointer)notebook, "select-editor", --page_num);

  return TRUE;
}

/**
 * codeslayer_notebook_close_all_editors:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Returns: is TRUE unless there are editors that need to saved. If there are 
 *          editors that still need to be saved then the pages will not be 
 *          closed and this method will return FALSE.
 */
gboolean
codeslayer_notebook_close_all_editors (CodeSlayerNotebook *notebook)
{
  gint pages;
  gint i;
  gint page_num;
  GtkWidget *notebook_page;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (i = 0; i < pages; i++)
      if (!has_clean_buffer (notebook, i))
          return FALSE;

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

  return TRUE;
}

/**
 * codeslayer_notebook_get_active_editor:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Returns: the active editor.
 */
GtkWidget*
codeslayer_notebook_get_active_editor (CodeSlayerNotebook *notebook)
{
  gint page_num;
  GtkWidget *notebook_page;

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 
                                             page_num);
  return codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
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
  codeslayer_notebook_close_editor (notebook, page);
}

static void
close_all_editors_action (CodeSlayerNotebookTab *notebook_tab,
                          CodeSlayerNotebook    *notebook)
{
  codeslayer_notebook_close_all_editors (notebook);
}

static void
close_other_editors_action (CodeSlayerNotebookTab *notebook_tab,
                            CodeSlayerNotebook    *notebook)
{
  gint pages;
  gint page;
  gint i;
  GtkWidget *notebook_page;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                GTK_WIDGET (notebook_page));

  for (i = 0; i < pages; i++)
    {
      if (i != page && !has_clean_buffer (notebook, i)) 
        return;
    }

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

static void
close_right_editors_action (CodeSlayerNotebookTab *notebook_tab,
                            CodeSlayerNotebook    *notebook)
{
  gint pages;
  gint page;
  gint i;
  GtkWidget *notebook_page;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                GTK_WIDGET (notebook_page));

  for (i = page; i < pages; i++)
      if (!has_clean_buffer (notebook, i)) 
        return;

  while (pages > page + 1)
    {
      gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), -1);
      pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    }
}

static void
close_left_editors_action (CodeSlayerNotebookTab *notebook_tab,
                           CodeSlayerNotebook    *notebook)
{
  GtkWidget *notebook_page;
  gint page;
  gint i;
  
  notebook_page = codeslayer_notebook_tab_get_notebook_page (notebook_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                GTK_WIDGET (notebook_page));

  for (i = 0; i < page; i++)
    {
      if (!has_clean_buffer (notebook, i)) 
        return;
    }

  while (page != 0)
    {
      gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), 0);
      page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                    GTK_WIDGET (notebook_page));
    }
}

static gboolean
has_clean_buffer (CodeSlayerNotebook *notebook, 
                  gint                page)
{
  GtkWidget *notebook_page;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  CodeSlayerDocument *document;
  const gchar *file_path;
  gchar *base_name;
  gchar *text;
  GtkWidget *dialog;
  gint response;
  
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  if (!gtk_text_buffer_get_modified (buffer)) 
    return TRUE;

  document = codeslayer_notebook_page_get_document (
                                            CODESLAYER_NOTEBOOK_PAGE (notebook_page));
  file_path = codeslayer_document_get_file_path (document);
  base_name = g_path_get_basename (file_path);
  text = g_strdup_printf (_("Save changes to %s?"), base_name);

  dialog = gtk_message_dialog_new_with_markup (NULL, GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_WARNING,
                                               GTK_BUTTONS_NONE, text, NULL);
  gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_NO, GTK_RESPONSE_NO);
  gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_YES, GTK_RESPONSE_YES);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

  g_free (base_name);
  g_free (text);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_YES)
    {
      codeslayer_notebook_save_editor (notebook, page);
    }
  else if (response == GTK_RESPONSE_CANCEL)
    {
      gtk_widget_destroy (dialog);
      return FALSE;
    }
  gtk_widget_destroy (dialog);
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
preferences_changed_action (CodeSlayerNotebook *notebook)
{
  CodeSlayerNotebookPrivate *priv;
  gchar *editor_value;
  
  priv = CODESLAYER_NOTEBOOK_GET_PRIVATE (notebook);

  editor_value = codeslayer_preferences_get_string (priv->preferences,
                                                    CODESLAYER_PREFERENCES_EDITOR_TAB_POSITION);

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
