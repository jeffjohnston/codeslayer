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

#include <codeslayer/codeslayer-search-tab.h>
#include <codeslayer/codeslayer-search-page.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-search-tab
 * @short_description: The global search page label.
 * @title: CodeSlayerSearchTab
 * @include: codeslayer/codeslayer-search-tab.h
 *
 * The tab is appended to the search page as a way to give the page a 
 * label. The tab itself allows a page to be closed.
 */

static void codeslayer_search_tab_class_init    (CodeSlayerSearchTabClass *klass);
static void codeslayer_search_tab_init          (CodeSlayerSearchTab      *search_tab);
static void codeslayer_search_tab_finalize      (CodeSlayerSearchTab      *search_tab);
static void codeslayer_search_tab_get_property  (GObject                  *object, 
                                                 guint                     prop_id, 
                                                 GValue                   *value, 
                                                 GParamSpec               *pspec);
static void codeslayer_search_tab_set_property  (GObject                  *object, 
                                                 guint                     prop_id, 
                                                 const GValue             *value, 
                                                 GParamSpec               *pspec);
static void close_page_action                   (CodeSlayerSearchTab      *search_tab);

#define CODESLAYER_SEARCH_TAB_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_SEARCH_TAB_TYPE, CodeSlayerSearchTabPrivate))

typedef struct _CodeSlayerSearchTabPrivate CodeSlayerSearchTabPrivate;

struct _CodeSlayerSearchTabPrivate
{
  GtkWidget *search_page;
  GtkWidget *label;
  GtkWidget *button;
  gboolean   closeable;
};

enum
{
  PROP_0,
  PROP_SEARCH_PAGE
};

enum
{
  CLOSE_PAGE,
  LAST_SIGNAL
};

static guint codeslayer_search_tab_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerSearchTab, codeslayer_search_tab, GTK_TYPE_HBOX)
     
static void
codeslayer_search_tab_class_init (CodeSlayerSearchTabClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  /**
   * CodeSlayerSearchTab::close-page
   * @codeslayersearchtab: the tab that received the signal
   *
   * The ::close-tab signal is a request to close the active page.
   */
  codeslayer_search_tab_signals[CLOSE_PAGE] =
    g_signal_new ("close-page", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerSearchTabClass, close_page),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_search_tab_finalize;

  gobject_class->get_property = codeslayer_search_tab_get_property;
  gobject_class->set_property = codeslayer_search_tab_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerSearchTabPrivate));

  /**
   * CodeSlayerSearchTab:search-page:
   *
   * A #CodeSlayerSearchPage.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_SEARCH_PAGE,
                                   g_param_spec_pointer ("search_page",
                                                         "Search Page",
                                                         "Search Page",
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_search_tab_init (CodeSlayerSearchTab *search_tab)
{
  gtk_box_set_homogeneous (GTK_BOX (search_tab), FALSE);
  gtk_box_set_spacing (GTK_BOX (search_tab), 0);
}

static void
codeslayer_search_tab_finalize (CodeSlayerSearchTab *search_tab)
{
  G_OBJECT_CLASS (codeslayer_search_tab_parent_class)->finalize ( G_OBJECT (search_tab));
}

static void
codeslayer_search_tab_get_property (GObject    *object, 
                                    guint       prop_id, 
                                    GValue     *value, 
                                    GParamSpec *pspec)
{
  CodeSlayerSearchTabPrivate *priv;
  CodeSlayerSearchTab *search_tab;
  
  search_tab = CODESLAYER_SEARCH_TAB (object);
  priv = CODESLAYER_SEARCH_TAB_GET_PRIVATE (search_tab);

  switch (prop_id)
    {
    case PROP_SEARCH_PAGE:
      g_value_set_pointer (value, priv->search_page);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_search_tab_set_property (GObject      *object, 
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  CodeSlayerSearchTab *search_tab;
  
  search_tab = CODESLAYER_SEARCH_TAB (object);

  switch (prop_id)
    {
    case PROP_SEARCH_PAGE:
      codeslayer_search_tab_set_search_page (search_tab, 
                                             g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_search_tab_new:
 * @closeable: is TRUE if this tab can be closed.
 *
 * Creates a new #CodeSlayerSearchTab.
 *
 * Returns: a new #CodeSlayerSearchTab. 
 */
GtkWidget*
codeslayer_search_tab_new (gboolean closeable)
{
  CodeSlayerSearchTabPrivate *priv;
  GtkWidget *search_tab;
  GtkWidget *label;
  
  search_tab = g_object_new (codeslayer_search_tab_get_type (), NULL);
  priv = CODESLAYER_SEARCH_TAB_GET_PRIVATE (search_tab);

  label = gtk_label_new (_("New Search"));
  priv->label = label;

  gtk_box_pack_start (GTK_BOX (search_tab), label, TRUE, TRUE, 1);

  priv->closeable = closeable;
  if (priv->closeable)
    {
      GtkWidget *button;
      GtkWidget *image;

      button = gtk_button_new ();
      priv->button = button;
      
      gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
      gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
      image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
      gtk_container_add (GTK_CONTAINER (button), image);      
      codeslayer_utils_style_close_button (button);

      g_signal_connect_swapped (G_OBJECT (button), "clicked",
                                G_CALLBACK (close_page_action), search_tab);
      
      gtk_box_pack_start (GTK_BOX (search_tab), button, FALSE, FALSE, 1);
    }

  return search_tab;
}

static void
close_page_action (CodeSlayerSearchTab *search_tab)
{
  g_signal_emit_by_name ((gpointer) search_tab, "close-page");
}

/**
 * codeslayer_search_tab_get_search_page:
 * @search_tab: a #CodeSlayerSearchTab.
 *
 * Returns: the search page contained with the tab.
 */
GtkWidget*
codeslayer_search_tab_get_search_page (CodeSlayerSearchTab *search_tab)
{
  return CODESLAYER_SEARCH_TAB_GET_PRIVATE (search_tab)->search_page;
}

/**
 * codeslayer_search_tab_set_search_page:
 * @search_tab: a #CodeSlayerSearchTab.
 * @search_page: a #CodeSlayerSearchPage.
 */
void
codeslayer_search_tab_set_search_page (CodeSlayerSearchTab  *search_tab,
                                       GtkWidget            *search_page)
{
  CodeSlayerSearchTabPrivate *priv;
  priv = CODESLAYER_SEARCH_TAB_GET_PRIVATE (search_tab);
  priv->search_page = search_page;
}

/**
 * codeslayer_search_tab_set_label_name:
 * @search_tab: a #CodeSlayerSearchTab.
 * @name: the display name for the tab.
 *
 * Change the label name of the tab.
 */
void
codeslayer_search_tab_set_label_name (CodeSlayerSearchTab *search_tab,
                                      const gchar         *name)
{
  CodeSlayerSearchTabPrivate *priv;
  priv = CODESLAYER_SEARCH_TAB_GET_PRIVATE (search_tab);
  gtk_label_set_text (GTK_LABEL (priv->label), name);
}

/**
 * codeslayer_search_tab_set_tooltip:
 * @search_tab: a #CodeSlayerSearchTab.
 * @tooltip: the tooltip for the tab.
 *
 * Change the tooltip of the tab.
 */
void
codeslayer_search_tab_set_tooltip (CodeSlayerSearchTab  *search_tab, 
                                   const gchar          *tooltip)
{
  CodeSlayerSearchTabPrivate *priv;
  priv = CODESLAYER_SEARCH_TAB_GET_PRIVATE (search_tab);
  gtk_widget_set_tooltip_text (priv->label, tooltip);
}
                                                                                                        