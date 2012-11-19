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

#include <glib/gprintf.h>
#include <string.h>
#include <codeslayer/codeslayer-search.h>
#include <codeslayer/codeslayer-search-page.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-search
 * @short_description: The global search.
 * @title: CodeSlayerSearch
 * @include: codeslayer/codeslayer-search.h
 *
 * Holds the search projects.
 */

static void codeslayer_search_class_init    (CodeSlayerSearchClass *klass);
static void codeslayer_search_init          (CodeSlayerSearch      *search);
static void codeslayer_search_finalize      (CodeSlayerSearch      *search);
static void codeslayer_search_get_property  (GObject               *object, 
                                             guint                  prop_id,
                                             GValue                *value,
                                             GParamSpec            *pspec);
static void codeslayer_search_set_property  (GObject               *object,
                                             guint                  prop_id,
                                             const GValue          *value,
                                             GParamSpec            *pspec);                                             
static void close_action                    (CodeSlayerSearch      *search);
static void open_document_action            (CodeSlayerSearch      *search,
                                             CodeSlayerDocument    *document);
                                             
static void add_notebook                    (CodeSlayerSearch      *search);
                                             
#define CODESLAYER_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_SEARCH_TYPE, CodeSlayerSearchPrivate))

typedef struct _CodeSlayerSearchPrivate CodeSlayerSearchPrivate;

struct _CodeSlayerSearchPrivate
{
  GtkWindow             *parent;
  GtkWidget             *search_page;
  CodeSlayerPreferences *preferences;
  CodeSlayerGroups      *groups;
  GtkWidget             *vbox;
};

enum
{
  SELECT_DOCUMENT,
  CLOSE,
  LAST_SIGNAL
};

static guint codeslayer_search_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerSearch, codeslayer_search, GTK_TYPE_WINDOW)

enum
{
  PROP_0,
  PROP_FILE_PATHS
};

static void 
codeslayer_search_class_init (CodeSlayerSearchClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  /**
   * CodeSlayerSearchPage::select-document
   * @codeslayersearch: the search that received the signal
   *
   * Note: for internal use only.
   *
   * The ::select-document signal is a request to select the document in the 
   * tree (which in turn adds a page in the notebook).
   */
  codeslayer_search_signals[SELECT_DOCUMENT] =
    g_signal_new ("select-document", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerSearchClass, select_document),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayerSearchPage::close
   * @codeslayersearch: the search that received the signal
   *
   * Note: for internal use only.
   *
   * The ::close signal is a request to close the search box.
   */
  codeslayer_search_signals[CLOSE] =
    g_signal_new ("close", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerSearchClass, close),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_search_finalize;
  
  gobject_class->get_property = codeslayer_search_get_property;
  gobject_class->set_property = codeslayer_search_set_property;
  
  g_type_class_add_private (klass, sizeof (CodeSlayerSearchPrivate));
}

static void
codeslayer_search_init (CodeSlayerSearch *search)
{
  gtk_window_set_title (GTK_WINDOW (search), _("Search"));  
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (search), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (search), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (search), 3);
}

static void
codeslayer_search_finalize (CodeSlayerSearch *search)
{
  G_OBJECT_CLASS (codeslayer_search_parent_class)-> finalize (G_OBJECT (search));
}

static void
codeslayer_search_get_property (GObject    *object, 
                                guint       prop_id,
                                GValue     *value, 
                                GParamSpec *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_search_set_property (GObject      *object, 
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
 * codeslayer_search_new:
 * @window: the main application window.
 * @preferences: a #CodeSlayerPreferences.
 * @groups: a #CodeSlayerGroups.
 *
 * Creates a new #CodeSlayerSearch.
 *
 * Returns: a new #CodeSlayerSearch. 
 */
GtkWidget*
codeslayer_search_new (GtkWindow             *window,
                       CodeSlayerPreferences *preferences,
                       CodeSlayerGroups      *groups)
{
  CodeSlayerSearchPrivate *priv;
  GtkWidget *search;
  
  search = g_object_new (codeslayer_search_get_type (), NULL);
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  priv->parent = window;
  priv->preferences = preferences;
  priv->groups = groups;
  
  gtk_window_set_transient_for (GTK_WINDOW (search), window);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (search), TRUE);
  
  add_notebook (CODESLAYER_SEARCH (search));
                            
  return search;
}

static void
add_notebook (CodeSlayerSearch *search)
{
  CodeSlayerSearchPrivate *priv;
  GtkWidget *vbox;
  GtkWidget *button_box;
  GtkWidget *close_button;
  GtkWidget *search_page;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
  priv->vbox = vbox;
  
  search_page = codeslayer_search_page_new (priv->preferences, priv->groups);
  priv->search_page = search_page;

  gtk_box_pack_start (GTK_BOX(vbox), search_page, TRUE, TRUE, 2);
  
  button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box), GTK_BUTTONBOX_END);
  gtk_container_set_border_width (GTK_CONTAINER (button_box), 4);
  close_button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_box_pack_start (GTK_BOX(button_box), close_button, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), button_box, FALSE, FALSE, 0);
  
  g_signal_connect_swapped (G_OBJECT (close_button), "clicked",
                            G_CALLBACK (close_action), search);

  g_signal_connect_swapped (G_OBJECT (search_page), "select-document",
                            G_CALLBACK (open_document_action), search);

  gtk_container_add (GTK_CONTAINER (search), vbox);
}

/**
 * codeslayer_search_add_page:
 * @search: a #CodeSlayerSearch.
 * @file_paths: the file paths as a comma separated list. 
 */
void
codeslayer_search_add_page (CodeSlayerSearch *search, 
                            const gchar      *file_paths)
{
}

/**
 * codeslayer_search_clear:
 * @search: a #CodeSlayerSearch.
 */
void
codeslayer_search_clear (CodeSlayerSearch *search)
{
}

static void
open_document_action (CodeSlayerSearch   *search,
                      CodeSlayerDocument *document)
{
  g_signal_emit_by_name ((gpointer) search, "select-document", document);
}

static void
close_action (CodeSlayerSearch *search)
{
  g_signal_emit_by_name ((gpointer) search, "close");
}
