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
#include <codeslayer/codeslayer-search-tab.h>
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
static void close_search_page_action        (CodeSlayerSearchTab   *search_tab,
                                             CodeSlayerSearch      *search);                                             
static void open_document_action            (CodeSlayerSearch      *search,
                                             CodeSlayerDocument    *document);
                                             
#define CODESLAYER_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_SEARCH_TYPE, CodeSlayerSearchPrivate))

typedef struct _CodeSlayerSearchPrivate CodeSlayerSearchPrivate;

struct _CodeSlayerSearchPrivate
{
  GtkWindow             *parent;
  CodeSlayerPreferences *preferences;
  CodeSlayerGroups      *groups;
  GtkWidget             *vbox;
  GtkWidget             *notebook;
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
  CodeSlayerSearchPrivate *priv;
  GtkWidget *vbox;
  GtkWidget *button_box;
  GtkWidget *notebook;
  GtkWidget *close_button;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  
  gtk_window_set_title (GTK_WINDOW (search), _("Search"));  
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (search), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (search), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (search), 3);
  
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
  priv->vbox = vbox;
  
  notebook = gtk_notebook_new ();
  priv->notebook = notebook;
  
  gtk_box_pack_start (GTK_BOX(vbox), notebook, TRUE, TRUE, 2);
  
  button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box), GTK_BUTTONBOX_END);
  gtk_container_set_border_width (GTK_CONTAINER (button_box), 4);
  close_button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_box_pack_start (GTK_BOX(button_box), close_button, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), button_box, FALSE, FALSE, 0);
  
  g_signal_connect_swapped (G_OBJECT (close_button), "clicked",
                            G_CALLBACK (close_action), CODESLAYER_SEARCH (search));

  gtk_container_add (GTK_CONTAINER (search), vbox);
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
  GtkWidget *search_page;
  GtkWidget *search_tab;
  
  search = g_object_new (codeslayer_search_get_type (), NULL);
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  priv->parent = window;
  priv->preferences = preferences;
  priv->groups = groups;
  
  gtk_window_set_transient_for (GTK_WINDOW (search), window);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (search), TRUE);
  
  search_page = codeslayer_search_page_new (priv->preferences, priv->groups);
  search_tab = codeslayer_search_tab_new (FALSE);
  codeslayer_search_tab_set_label_name (CODESLAYER_SEARCH_TAB (search_tab),
                                        _("Find In Projects"));
  codeslayer_search_tab_set_search_page (CODESLAYER_SEARCH_TAB (search_tab), 
                                         search_page);

  g_signal_connect_swapped (G_OBJECT (search_page), "select-document",
                            G_CALLBACK (open_document_action), CODESLAYER_SEARCH (search));

  gtk_widget_show_all (GTK_WIDGET (search_tab));
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook), 
                            search_page, search_tab);
                            
  return search;
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
  CodeSlayerSearchPrivate *priv;
  GtkWidget *search_page;
  GtkWidget *search_tab;
  gchar *tooltip;
  gint page;

  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  
  search_page = codeslayer_search_page_new (priv->preferences, priv->groups);
  codeslayer_search_page_set_file_paths (CODESLAYER_SEARCH_PAGE(search_page), 
                                         file_paths);

  search_tab = codeslayer_search_tab_new (TRUE);
  codeslayer_search_tab_set_search_page (CODESLAYER_SEARCH_TAB (search_tab), 
                                         search_page);
  codeslayer_search_page_set_search_tab (CODESLAYER_SEARCH_PAGE(search_page), 
                                         search_tab);
       
  tooltip = codeslayer_utils_strreplace (file_paths, ";", "\n");
  codeslayer_search_tab_set_tooltip (CODESLAYER_SEARCH_TAB (search_tab), tooltip);
  g_free (tooltip);

  page = gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook), 
                                   search_page, search_tab);

  g_signal_connect_swapped (G_OBJECT (search_page), "select-document",
                            G_CALLBACK (open_document_action), search);
  
  g_signal_connect (G_OBJECT (search_tab), "close-page",
                    G_CALLBACK (close_search_page_action), search);

  gtk_widget_show_all (GTK_WIDGET (search_page));
  gtk_widget_show_all (GTK_WIDGET (search_tab));

  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), page);
  codeslayer_search_page_grab_focus (CODESLAYER_SEARCH_PAGE(search_page));
}

/**
 * codeslayer_search_default_page:
 * @search: a #CodeSlayerSearch.
 */
void
codeslayer_search_default_page (CodeSlayerSearch *search)
{
  CodeSlayerSearchPrivate *priv;
  GtkWidget *search_page;
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);
  
  search_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), 0);  
  codeslayer_search_page_grab_focus (CODESLAYER_SEARCH_PAGE(search_page));
}

/**
 * codeslayer_search_clear:
 * @search: a #CodeSlayerSearch.
 */
void
codeslayer_search_clear (CodeSlayerSearch *search)
{
  CodeSlayerSearchPrivate *priv;
  GtkWidget *fixed_search;
  gint pages;

  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));
  while (pages > 1)
    {
      gtk_notebook_remove_page (GTK_NOTEBOOK(priv->notebook), -1);
      pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));
    }
    
  fixed_search = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), 0);
  codeslayer_search_page_clear (CODESLAYER_SEARCH_PAGE (fixed_search));
}

static void
close_search_page_action (CodeSlayerSearchTab *search_tab,
                          CodeSlayerSearch    *search)
{
  CodeSlayerSearchPrivate *priv;
  GtkWidget *search_page;
  gint page;

  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  search_page = codeslayer_search_tab_get_search_page (search_tab);
  page = gtk_notebook_page_num (GTK_NOTEBOOK(priv->notebook), search_page);
  gtk_notebook_remove_page (GTK_NOTEBOOK(priv->notebook), page);
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
