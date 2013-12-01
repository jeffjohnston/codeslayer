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

#include <gtksourceview/gtksource.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-sourceview.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-notebook-page
 * @short_description: Contains the source view and document.
 * @title: CodeSlayerNotebookPage
 * @include: codeslayer/codeslayer-notebook-page.h
 */
 
static void codeslayer_notebook_page_class_init    (CodeSlayerNotebookPageClass *klass);
static void codeslayer_notebook_page_init          (CodeSlayerNotebookPage      *notebook_page);
static void codeslayer_notebook_page_finalize      (CodeSlayerNotebookPage      *notebook_page);

static void external_changes_response_action       (CodeSlayerNotebookPage      *notebook_page,
                                                    gint                         response_id);

#define CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_PAGE_TYPE, CodeSlayerNotebookPagePrivate))

typedef struct _CodeSlayerNotebookPagePrivate CodeSlayerNotebookPagePrivate;

struct _CodeSlayerNotebookPagePrivate
{
  GtkWidget *source_view;
  GtkWidget *document_not_found_info_bar;
  GtkWidget *external_changes_info_bar;
};

enum
{
  PROP_0,
  PROP_DOCUMENT,
  PROP_EDITOR
};

G_DEFINE_TYPE (CodeSlayerNotebookPage, codeslayer_notebook_page, GTK_TYPE_VBOX)

static void
codeslayer_notebook_page_class_init (CodeSlayerNotebookPageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_notebook_page_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerNotebookPagePrivate));
}

static void
codeslayer_notebook_page_init (CodeSlayerNotebookPage *notebook_page) 
{
  CodeSlayerNotebookPagePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);
  priv->document_not_found_info_bar = NULL;
  priv->external_changes_info_bar = NULL;
}

static void
codeslayer_notebook_page_finalize (CodeSlayerNotebookPage *notebook_page)
{
  G_OBJECT_CLASS (codeslayer_notebook_page_parent_class)->finalize (G_OBJECT(notebook_page));
}

/**
 * codeslayer_notebook_page_new:
 * @source_view: a #CodeSlayerSourceView.
 *
 * Creates a new #CodeSlayerNotebookPage.
 *
 * Returns: a new #CodeSlayerNotebookPage. 
 */
GtkWidget*
codeslayer_notebook_page_new (GtkWidget *source_view)
{
  CodeSlayerNotebookPagePrivate *priv;
  GtkWidget *notebook_page;
  GtkWidget *scrolled_window;
  
  notebook_page = g_object_new (codeslayer_notebook_page_get_type (), NULL);

  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);
  priv->source_view = source_view;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (source_view));

  gtk_box_pack_start (GTK_BOX (notebook_page), scrolled_window, TRUE, TRUE, 0);

  return notebook_page;
}

/**
 * codeslayer_notebook_page_get_source_view:
 * @notebook_page: a #CodeSlayerNotebookPage.
 * 
 * Returns: the #CodeSlayerSourceView in the page.
 */
GtkWidget*
codeslayer_notebook_page_get_source_view (CodeSlayerNotebookPage *notebook_page)
{
  return CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page)->source_view;
}

/**
 * codeslayer_notebook_page_show_document_not_found_info_bar:
 * @notebook_page: a #CodeSlayerNotebookPage.
 * 
 * Show the information bar at the top of the page to inform the user the that 
 * document is not found. This will happen if the document is moved 
 * (or deleted) in the projects tree while the page is opened.
 */
void
codeslayer_notebook_page_show_document_not_found_info_bar (CodeSlayerNotebookPage *notebook_page)
{
  CodeSlayerNotebookPagePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);

  if (priv->document_not_found_info_bar == NULL)
    {
      GtkWidget *content_area;
      CodeSlayerDocument *document;
      const gchar *file_path;
      gchar *text;
      GtkWidget *label;
      
      priv->document_not_found_info_bar = gtk_info_bar_new ();
      gtk_info_bar_set_message_type (GTK_INFO_BAR (priv->document_not_found_info_bar), GTK_MESSAGE_ERROR);

      content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (priv->document_not_found_info_bar));
      document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (priv->source_view));
      file_path =  codeslayer_document_get_file_path (document);
      text = g_strdup_printf(_("The document %s no longer exists."), file_path);
      label = gtk_label_new (text);
      gtk_container_add (GTK_CONTAINER (content_area), label);
      g_free (text);

      gtk_box_pack_start (GTK_BOX (notebook_page), priv->document_not_found_info_bar, FALSE, FALSE, 0);
      gtk_box_reorder_child (GTK_BOX (notebook_page), priv->document_not_found_info_bar, 0);

      gtk_widget_show_all (GTK_WIDGET (notebook_page));
    }
}

void 
codeslayer_notebook_page_show_external_changes_info_bar (CodeSlayerNotebookPage *notebook_page)
{
  CodeSlayerNotebookPagePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);

  if (priv->external_changes_info_bar == NULL)
    {
      GtkWidget *content_area;
      CodeSlayerDocument *document;
      const gchar *file_path;
      gchar *text;
      GtkWidget *label;
      
      priv->external_changes_info_bar = gtk_info_bar_new_with_buttons  (_("Reload"), GTK_RESPONSE_OK,
                                                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
      
      gtk_info_bar_set_message_type (GTK_INFO_BAR (priv->external_changes_info_bar), GTK_MESSAGE_WARNING);

      content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (priv->external_changes_info_bar));

      document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (priv->source_view));
      file_path =  codeslayer_document_get_file_path (document);
      text = g_strdup_printf(_("The document %s changed on disk."), file_path);
      label = gtk_label_new (text);
      gtk_container_add (GTK_CONTAINER (content_area), label);
      g_free (text);
      
      g_signal_connect_swapped (G_OBJECT (priv->external_changes_info_bar), "response",
                                G_CALLBACK (external_changes_response_action), notebook_page);      

      gtk_box_pack_start (GTK_BOX (notebook_page), priv->external_changes_info_bar, FALSE, FALSE, 0);
      gtk_box_reorder_child (GTK_BOX (notebook_page), priv->external_changes_info_bar, 0);

      gtk_widget_show_all (GTK_WIDGET (notebook_page));
    }
}

static void
external_changes_response_action (CodeSlayerNotebookPage *notebook_page, 
                                  gint                    response_id)
{
  CodeSlayerNotebookPagePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);

  if (response_id == GTK_RESPONSE_CANCEL)
    {
      gtk_container_remove (GTK_CONTAINER (notebook_page), priv->external_changes_info_bar);
      priv->external_changes_info_bar = NULL;
    }
  else if (response_id == GTK_RESPONSE_OK)
    {
      codeslayer_notebook_page_load_source_view (notebook_page);
      gtk_container_remove (GTK_CONTAINER (notebook_page), priv->external_changes_info_bar);
      priv->external_changes_info_bar = NULL;
    }
}

void
codeslayer_notebook_page_load_source_view (CodeSlayerNotebookPage *notebook_page)
{
  CodeSlayerNotebookPagePrivate *priv;
  CodeSlayerDocument *document;
  const gchar *file_path;
  GtkTextBuffer *buffer;
  GTimeVal *modification_time;
  gchar *contents;
  gint line_number;
  
  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));
  document = codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (priv->source_view));
  file_path = codeslayer_document_get_file_path (document);
  line_number = codeslayer_document_get_line_number (document);
  
  contents = codeslayer_utils_get_utf8_text (file_path);  
  if (contents != NULL)
    {
      gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (buffer));
      codeslayer_source_view_set_text (CODESLAYER_SOURCE_VIEW (priv->source_view), contents);
      gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (buffer));
      gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (buffer), FALSE);
      g_free (contents);
    }
  
  modification_time = codeslayer_utils_get_modification_time (file_path);
  codeslayer_source_view_set_modification_time (CODESLAYER_SOURCE_VIEW (priv->source_view), modification_time);
  
  if (line_number > 0)
    codeslayer_source_view_scroll_to_line (CODESLAYER_SOURCE_VIEW (priv->source_view), line_number);
}
