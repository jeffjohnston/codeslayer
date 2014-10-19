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

#include <gdk/gdkkeysyms.h>
#include <codeslayer/codeslayer-notebook-pane.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-notebook-search.h>

/**
 * SECTION:codeslayer-notebook-pane
 * @short_description: Container for the notebook and inline search.
 * @title: CodeSlayerNotebookPane
 * @include: codeslayer/codeslayer-notebook-pane.h
 */

static void codeslayer_notebook_pane_class_init    (CodeSlayerNotebookPaneClass *klass);
static void codeslayer_notebook_pane_init          (CodeSlayerNotebookPane      *notebook_pane);
static void codeslayer_notebook_pane_finalize      (CodeSlayerNotebookPane      *notebook_pane);
static void codeslayer_notebook_pane_get_property  (GObject                     *object, 
                                                    guint                        prop_id, 
                                                    GValue                      *value, 
                                                    GParamSpec                  *pspec);
static void codeslayer_notebook_pane_set_property  (GObject                     *object, 
                                                    guint                        prop_id, 
                                                    const GValue                *value, 
                                                    GParamSpec                  *pspec);

static void show_search                            (CodeSlayerNotebookPane      *notebook_pane);
static void close_search                           (CodeSlayerNotebookSearch    *notebook_search);

static void find_next_action                       (CodeSlayerNotebookPane      *notebook_pane);
                        
#define CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_PANE_TYPE, CodeSlayerNotebookPanePrivate))

typedef struct _CodeSlayerNotebookPanePrivate CodeSlayerNotebookPanePrivate;

struct _CodeSlayerNotebookPanePrivate
{
  GtkWidget *notebook;
  GtkWidget *notebook_search;
};

enum
{
  PROP_0,
  PROP_NOTEBOOK,
  PROP_NOTEBOOK_SEARCH
};

enum
{
  FIND_NEXT,
  FIND_UNDO_INCREMENTAL,
  LAST_SIGNAL
};

static guint codeslayer_notebook_pane_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerNotebookPane, codeslayer_notebook_pane, GTK_TYPE_BOX)

static void
codeslayer_notebook_pane_class_init (CodeSlayerNotebookPaneClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkBindingSet *binding_set;

  klass->find_next = find_next_action;

  /**
   * CodeSlayerNotebookPane::find-next
   * @codeslayernotebookpane: the pane that received the signal
   *
   * The ::find-next signal enables the (F3) keystroke to search for 
   * the next value in the inline search.
   */
  codeslayer_notebook_pane_signals[FIND_NEXT] =
    g_signal_new ("find-next", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerNotebookPaneClass, find_next), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_notebook_pane_finalize;

  binding_set = gtk_binding_set_by_class (klass);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_F3, 0, "find-next", 0);

  gobject_class->get_property = codeslayer_notebook_pane_get_property;
  gobject_class->set_property = codeslayer_notebook_pane_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerNotebookPanePrivate));

  /**
   * CodeSlayerNotebookPane:notebook:
   *
   * A #CodeSlayerNotebook that makes up the top of the pane.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_NOTEBOOK,
                                   g_param_spec_pointer ("notebook",
                                                         "Notebook",
                                                         "Notebook Object",
                                                         G_PARAM_READWRITE));

  /**
   * CodeSlayerNotebookPane:notebook-search:
   *
   * A #CodeSlayerNotebookSearch that makes up the bottom of the pane.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_NOTEBOOK_SEARCH,
                                   g_param_spec_pointer ("notebook_search",
                                                         "Notebook Search",
                                                         "Notebook Search Object",
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_notebook_pane_init (CodeSlayerNotebookPane *notebook_pane)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (notebook_pane), GTK_ORIENTATION_VERTICAL);
  gtk_box_set_homogeneous (GTK_BOX (notebook_pane), FALSE);
  gtk_box_set_spacing (GTK_BOX (notebook_pane), 0);
}

static void
codeslayer_notebook_pane_finalize (CodeSlayerNotebookPane *notebook_pane)
{
  G_OBJECT_CLASS (codeslayer_notebook_pane_parent_class)->finalize (G_OBJECT (notebook_pane));
}

static void
codeslayer_notebook_pane_get_property (GObject    *object, 
                                       guint       prop_id,
                                       GValue     *value, 
                                       GParamSpec *pspec)
{
  CodeSlayerNotebookPanePrivate *priv;
  CodeSlayerNotebookPane *notebook_pane;
  
  notebook_pane = CODESLAYER_NOTEBOOK_PANE (object);
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);

  switch (prop_id)
    {
    case PROP_NOTEBOOK:
      g_value_set_pointer (value, priv->notebook);
      break;
    case PROP_NOTEBOOK_SEARCH:
      g_value_set_pointer (value, priv->notebook_search);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_notebook_pane_set_property (GObject      *object, 
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  CodeSlayerNotebookPane *notebook_pane;
  
  notebook_pane = CODESLAYER_NOTEBOOK_PANE (object);

  switch (prop_id)
    {
    case PROP_NOTEBOOK:
      codeslayer_notebook_pane_set_notebook (notebook_pane, 
                                             g_value_get_pointer (value));
      break;
    case PROP_NOTEBOOK_SEARCH:
      codeslayer_notebook_pane_set_notebook_search (notebook_pane,
                                                    g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_notebook_pane_new:
 *
 * Creates a new #CodeSlayerNotebookPane.
 *
 * Returns: a new #CodeSlayerNotebookPane. 
 */
GtkWidget*
codeslayer_notebook_pane_new ()
{
  GtkWidget *notebook_pane;
  notebook_pane = g_object_new (codeslayer_notebook_pane_get_type (), NULL);
  
  return notebook_pane;
}

/**
 * codeslayer_notebook_pane_set_notebook:
 * @notebook_pane: a #CodeSlayerNotebookPane.
 * @notebook: a #CodeSlayerNotebook.
 */
void
codeslayer_notebook_pane_set_notebook (CodeSlayerNotebookPane *notebook_pane,
                                       GtkWidget              *notebook)
{
  CodeSlayerNotebookPanePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);
  priv->notebook = notebook;
  gtk_box_pack_start (GTK_BOX (notebook_pane), GTK_WIDGET (notebook), 
                      TRUE, TRUE, 0);
}

/**
 * codeslayer_notebook_pane_set_notebook_search:
 * @notebook_pane: a #CodeSlayerNotebookPane.
 * @notebook_search: a #CodeSlayerNotebookSearch.
 */
void
codeslayer_notebook_pane_set_notebook_search (CodeSlayerNotebookPane   *notebook_pane,
                                              GtkWidget                *notebook_search)
{
  CodeSlayerNotebookPanePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);
  priv->notebook_search = notebook_search;
  g_signal_connect (G_OBJECT (notebook_search), "close-search",
                    G_CALLBACK (close_search), NULL);
}

/**
 * codeslayer_notebook_pane_search_find:
 * @notebook_pane: a #CodeSlayerNotebookPane.
 *
 * Open up the inline search and give focus to the find entry field.
 */
void
codeslayer_notebook_pane_search_find (CodeSlayerNotebookPane *notebook_pane)
{
  CodeSlayerNotebookPanePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);
  show_search (notebook_pane);
  codeslayer_notebook_search_find (CODESLAYER_NOTEBOOK_SEARCH (priv->notebook_search));
}

/**
 * codeslayer_notebook_pane_search_replace:
 * @notebook_pane: a #CodeSlayerNotebookPane.
 *
 * Open up the inline search and give focus to the replace entry field.
 */
void
codeslayer_notebook_pane_search_replace (CodeSlayerNotebookPane * notebook_pane)
{
  CodeSlayerNotebookPanePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);
  show_search (notebook_pane);
  codeslayer_notebook_search_replace (CODESLAYER_NOTEBOOK_SEARCH (priv->notebook_search));
}

/**
 * codeslayer_notebook_pane_search_find_next:
 * @notebook_pane: a #CodeSlayerNotebookPane.
 *
 * Delegate to the inline search to find the next search value.
 */
void
codeslayer_notebook_pane_search_find_next (CodeSlayerNotebookPane *notebook_pane)
{
  CodeSlayerNotebookPanePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);
  codeslayer_notebook_search_find_next (CODESLAYER_NOTEBOOK_SEARCH (priv->notebook_search));
}

/**
 * codeslayer_notebook_pane_search_find_previous:
 * @notebook_pane: a #CodeSlayerNotebookPane.
 *
 * Delegate to the inline search to find the previous search value.
 */
void
codeslayer_notebook_pane_search_find_previous (CodeSlayerNotebookPane *notebook_pane)
{
  CodeSlayerNotebookPanePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);
  codeslayer_notebook_search_find_previous (CODESLAYER_NOTEBOOK_SEARCH (priv->notebook_search));
}

static void
find_next_action (CodeSlayerNotebookPane *notebook_pane)
{
  CodeSlayerNotebookPanePrivate *priv;
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);
  codeslayer_notebook_search_find_next (CODESLAYER_NOTEBOOK_SEARCH (priv->notebook_search));
}

static void
show_search (CodeSlayerNotebookPane *notebook_pane)
{
  CodeSlayerNotebookPanePrivate *priv;
  GList *children;
  
  priv = CODESLAYER_NOTEBOOK_PANE_GET_PRIVATE (notebook_pane);

  children = gtk_container_get_children (GTK_CONTAINER (notebook_pane));
  if (g_list_length (children) == 1)
    {
      gtk_box_pack_start (GTK_BOX (notebook_pane),
                          GTK_WIDGET (CODESLAYER_NOTEBOOK_SEARCH (priv->notebook_search)), 
                          FALSE, FALSE, 3);
      gtk_widget_show_all (GTK_WIDGET (notebook_pane));
    }
  else
    {
      gtk_widget_show (GTK_WIDGET (CODESLAYER_NOTEBOOK_SEARCH (priv->notebook_search)));
    }
  g_list_free (children);
}

static void
close_search (CodeSlayerNotebookSearch *notebook_search)
{
  gtk_widget_hide (GTK_WIDGET (notebook_search));
}
