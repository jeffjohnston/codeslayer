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

#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-notebook-page
 * @short_description: Contains the editor and document.
 * @title: CodeSlayerNotebookPage
 * @include: codeslayer/codeslayer-notebook-page.h
 */

static void codeslayer_notebook_page_class_init    (CodeSlayerNotebookPageClass *klass);
static void codeslayer_notebook_page_init          (CodeSlayerNotebookPage      *notebook_page);
static void codeslayer_notebook_page_finalize      (CodeSlayerNotebookPage      *notebook_page);
static void codeslayer_notebook_page_get_property  (GObject                     *object, 
                                                    guint                        prop_id, 
                                                    GValue                      *value, 
                                                    GParamSpec                  *pspec);
static void codeslayer_notebook_page_set_property  (GObject                     *object, 
                                                    guint                        prop_id, 
                                                    const GValue                *value, 
                                                    GParamSpec                  *pspec);

#define CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_PAGE_TYPE, CodeSlayerNotebookPagePrivate))

typedef struct _CodeSlayerNotebookPagePrivate CodeSlayerNotebookPagePrivate;

struct _CodeSlayerNotebookPagePrivate
{
  GtkWidget          *editor;
  CodeSlayerDocument *document;
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

  gobject_class->get_property = codeslayer_notebook_page_get_property;
  gobject_class->set_property = codeslayer_notebook_page_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerNotebookPagePrivate));

  /**
   * CodeSlayerNotebookPage:editor:
   *
   * A #CodeSlayerEditor that backs the page.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_DOCUMENT,
                                   g_param_spec_pointer ("editor", 
                                                         "Editor",
                                                         "Editor Object",
                                                         G_PARAM_READWRITE));

  /**
   * CodeSlayerNotebookPage:document:
   *
   * A #CodeSlayerDocument that backs the page.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_DOCUMENT,
                                   g_param_spec_pointer ("document",
                                                         "Document",
                                                         "Document Object",
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_notebook_page_init (CodeSlayerNotebookPage *notebook_page) {}

static void
codeslayer_notebook_page_finalize (CodeSlayerNotebookPage *notebook_page)
{
  CodeSlayerNotebookPagePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);
  g_object_unref (priv->document);
  G_OBJECT_CLASS (codeslayer_notebook_page_parent_class)->finalize (G_OBJECT(notebook_page));
}

static void
codeslayer_notebook_page_get_property (GObject    *object, 
                                       guint       prop_id,
                                       GValue     *value, 
                                       GParamSpec *pspec)
{
  CodeSlayerNotebookPagePrivate *priv;
  CodeSlayerNotebookPage *notebook_page;
  
  notebook_page = CODESLAYER_NOTEBOOK_PAGE (object);
  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);

  switch (prop_id)
    {
    case PROP_DOCUMENT:
      g_value_set_pointer (value, priv->document);
      break;
    case PROP_EDITOR:
      g_value_set_pointer (value, priv->editor);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_notebook_page_set_property (GObject      *object, 
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_notebook_page_new:
 * @editor: a #CodeSlayerEditor.
 * @document: a #CodeSlayerDocument.
 *
 * Creates a new #CodeSlayerNotebookPage.
 *
 * Returns: a new #CodeSlayerNotebookPage. 
 */
GtkWidget*
codeslayer_notebook_page_new (GtkWidget   *editor,
                              CodeSlayerDocument *document)
{
  CodeSlayerNotebookPagePrivate *priv;
  GtkWidget *notebook_page;
  GtkWidget *scrolled_window;
  
  notebook_page = g_object_new (codeslayer_notebook_page_get_type (), NULL);

  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);
  priv->editor = editor;
  priv->document = document;
  g_object_ref_sink (G_OBJECT (document));

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (editor));

  gtk_box_pack_start (GTK_BOX (notebook_page), scrolled_window, TRUE, TRUE, 0);

  return notebook_page;
}

/**
 * codeslayer_notebook_page_get_editor:
 * @notebook_page: a #CodeSlayerNotebookPage.
 * 
 * Returns: the #CodeSlayerEditor in the page.
 */
GtkWidget*
codeslayer_notebook_page_get_editor (CodeSlayerNotebookPage *notebook_page)
{
  return CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page)->editor;
}

/**
 * codeslayer_notebook_page_get_document:
 * @notebook_page: a #CodeSlayerNotebookPage.
 * 
 * Returns: the #CodeSlayerDocument in the page.
 */
CodeSlayerDocument *
codeslayer_notebook_page_get_document (CodeSlayerNotebookPage *notebook_page)
{
  return CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page)->document;
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
  GList *children;
  
  priv = CODESLAYER_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);

  children = gtk_container_get_children (GTK_CONTAINER (notebook_page));
  if (g_list_length (children) == 1)
    {
      GtkWidget *info_bar;
      GtkWidget *content_area;
      const gchar *document_file_path;
      gchar *text;
      GtkWidget *label;
      
      info_bar = gtk_info_bar_new ();
      gtk_info_bar_set_message_type (GTK_INFO_BAR (info_bar), GTK_MESSAGE_ERROR);

      content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (info_bar));

      document_file_path =  codeslayer_document_get_file_path (priv->document);
      text = g_strconcat (_("The document "), document_file_path,
                                    _(" no longer exists."), NULL);
      label = gtk_label_new (text);
      gtk_container_add (GTK_CONTAINER (content_area), label);
      g_free (text);

      gtk_box_pack_start (GTK_BOX (notebook_page), info_bar, FALSE, FALSE, 0);
      gtk_box_reorder_child (GTK_BOX (notebook_page), info_bar, 0);

      gtk_widget_show_all (GTK_WIDGET (notebook_page));
    }
  g_list_free (children);
}
