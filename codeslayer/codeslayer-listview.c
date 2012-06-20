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

#include <codeslayer/codeslayer-listview.h>
#include "codeslayer-utils.h"

/**
 * SECTION:codeslayer-listview
 * @short_description: The generic list widget used throughout CodeSlayer.
 * @title: CodeSlayerListView
 * @include: codeslayer/codeslayer-listview.h
 */

static void codeslayer_list_view_class_init  (CodeSlayerListViewClass *klass);
static void codeslayer_list_view_init        (CodeSlayerListView      *list_view);
static void codeslayer_list_view_finalize    (CodeSlayerListView      *list_view);

static void tree_remove_action               (CodeSlayerListView      *list_view);
static void remove_row                       (GtkTreeRowReference     *tree_row_reference,
                                              GtkTreeModel            *model);
static void tree_add_action                  (CodeSlayerListView      *list_view);
static void add_view                         (CodeSlayerListView      *list_view);
static void tree_edited_action               (CodeSlayerListView      *list_view,
                                              gchar                   *path, 
                                              gchar                   *new_text);
static GList* get_list_values                (CodeSlayerListView      *list_view);
static gboolean add_value                    (GtkTreeModel            *model,
                                              GtkTreePath             *path,
                                              GtkTreeIter             *iter,
                                              GList                   **values);
static gint sort_compare                     (GtkTreeModel            *model, 
                                              GtkTreeIter             *a,
                                              GtkTreeIter             *b, 
                                              gpointer                 userdata);

#define CODESLAYER_LIST_VIEW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_LIST_VIEW_TYPE, CodeSlayerListViewPrivate))

typedef struct _CodeSlayerListViewPrivate CodeSlayerListViewPrivate;

struct _CodeSlayerListViewPrivate
{
  GtkWidget    *tree;
  GtkListStore *store;
};

enum
{
  TEXT,
  COLUMNS
};

enum
{
  LIST_CHANGED,
  LAST_SIGNAL
};

static guint codeslayer_list_view_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerListView, codeslayer_list_view, GTK_TYPE_HBOX)

static void 
codeslayer_list_view_class_init (CodeSlayerListViewClass *klass)
{
  /**
   * CodeSlayerListView::list-changed
   * @codeslayerlistview: the list view that received the signal
   *
   * The ::list-changed signal is emitted when the list values are 
   * edited, added, or removed.
   */
  codeslayer_list_view_signals[LIST_CHANGED] =
    g_signal_new ("list-changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerListViewClass, list_changed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_list_view_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerListViewPrivate));
}

static void
codeslayer_list_view_init (CodeSlayerListView *list_view)
{
  gtk_box_set_homogeneous (GTK_BOX (list_view), FALSE);
  gtk_box_set_spacing (GTK_BOX (list_view), 3);
  add_view (list_view);
}

static void
codeslayer_list_view_finalize (CodeSlayerListView *list_view)
{
  G_OBJECT_CLASS (codeslayer_list_view_parent_class)->finalize (G_OBJECT (list_view));
}

/**
 * codeslayer_list_view_new:
 *
 * Creates a new #CodeSlayerListView.
 *
 * Returns: a new #CodeSlayerListView. 
 */
GtkWidget*
codeslayer_list_view_new (void)
{
  GtkWidget *list_view;
  list_view = g_object_new (codeslayer_list_view_get_type (), NULL);
  return list_view;
}

static void
add_view (CodeSlayerListView *list_view)
{
  CodeSlayerListViewPrivate *priv;
  GtkWidget *vbox;
  GtkWidget *add_button, *remove_button;
  GtkWidget *tree;
  GtkListStore *store;
  GtkTreeSortable *sortable;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;
  GtkWidget *scrolled_window;
  
  priv = CODESLAYER_LIST_VIEW_GET_PRIVATE (list_view);
  
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
  
  /* create the tree */  

  tree = gtk_tree_view_new ();
  priv->tree = tree;
  store = gtk_list_store_new (COLUMNS, G_TYPE_STRING);
  priv->store = store;
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), GTK_TREE_MODEL (store));
  g_object_unref (store);
                           
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
                                                            
  sortable = GTK_TREE_SORTABLE (store);
  gtk_tree_sortable_set_sort_func (sortable, TEXT, sort_compare,
                                   GINT_TO_POINTER (TEXT), NULL);
  gtk_tree_sortable_set_sort_column_id (sortable, TEXT, GTK_SORT_ASCENDING);

  column = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
  
  g_signal_connect_swapped (G_OBJECT (renderer), "edited",
                            G_CALLBACK (tree_edited_action), list_view);
  
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer, "text", TEXT, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (tree));

  /* create the buttons */
  
  add_button = gtk_button_new_from_stock (GTK_STOCK_ADD);
  remove_button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
  
  g_signal_connect_swapped (G_OBJECT (add_button), "clicked",
                            G_CALLBACK (tree_add_action), list_view);
  
  g_signal_connect_swapped (G_OBJECT (remove_button), "clicked",
                            G_CALLBACK (tree_remove_action), list_view);
  
  gtk_box_pack_start (GTK_BOX (vbox), add_button, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), remove_button, FALSE, FALSE, 0);
  
  gtk_box_pack_start (GTK_BOX (list_view), scrolled_window, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (list_view), vbox, FALSE, FALSE, 0);
}

void
codeslayer_list_view_add_text (CodeSlayerListView *list_view, 
                               const gchar        *text)
{
  CodeSlayerListViewPrivate *priv;
  GtkTreeIter iter;

  priv = CODESLAYER_LIST_VIEW_GET_PRIVATE (list_view);

  gtk_list_store_append (priv->store, &iter);
  gtk_list_store_set (priv->store, &iter, TEXT, text, -1);
}

static void                
tree_add_action (CodeSlayerListView *list_view)
{
  CodeSlayerListViewPrivate *priv;
  GtkTreeViewColumn *column;
  GtkTreePath *child_path;
  GtkTreeIter iter;
  
  priv = CODESLAYER_LIST_VIEW_GET_PRIVATE (list_view);

  gtk_list_store_append (priv->store, &iter);
  gtk_list_store_set (priv->store, &iter, TEXT, "", -1);  
  
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->tree), 0);
  child_path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->store), &iter);
  gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->tree), child_path, column, TRUE);  
  
  gtk_tree_path_free (child_path);
}

static void                
tree_remove_action (CodeSlayerListView *list_view)
{
  CodeSlayerListViewPrivate *priv;
  GtkTreeSelection *selection;
  GtkTreeRowReference *tree_row_reference;
  GtkTreeModel *model;
  GList *rows, *tmp, *values, *references = NULL;

  priv = CODESLAYER_LIST_VIEW_GET_PRIVATE (list_view);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  model = GTK_TREE_MODEL (priv->store);
  rows = gtk_tree_selection_get_selected_rows (selection, &model);
  tmp = rows;
  
  while (tmp != NULL)
    {
      tree_row_reference = gtk_tree_row_reference_new (model, 
                                                       (GtkTreePath*) tmp->data);
      references = g_list_prepend (references, 
                                   gtk_tree_row_reference_copy (tree_row_reference));
      gtk_tree_row_reference_free (tree_row_reference);                                   
      tmp = tmp->next;
    }
    
  g_list_foreach (references, (GFunc) remove_row, model);
  g_list_foreach (references, (GFunc) gtk_tree_row_reference_free, NULL);
  g_list_foreach (rows, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (references);
  g_list_free (rows);

  values = get_list_values (list_view);
  g_signal_emit_by_name ((gpointer) list_view, "list-changed", values);
  g_list_foreach (values, (GFunc) g_free, NULL);
  g_list_free (values);
}

static void
remove_row (GtkTreeRowReference *tree_row_reference,
            GtkTreeModel        *model)
{
  GtkTreeIter iter;
  GtkTreePath *tree_path;
  tree_path = gtk_tree_row_reference_get_path (tree_row_reference);
  gtk_tree_model_get_iter (model, &iter, tree_path);
  gtk_list_store_remove (GTK_LIST_STORE (model), &iter);    
  gtk_tree_path_free (tree_path);
}

void 
tree_edited_action (CodeSlayerListView *list_view, 
                    gchar              *path, 
                    gchar              *text)
{
  CodeSlayerListViewPrivate *priv;
  GtkTreeIter iter;
  GtkTreeModel *model;
  
  priv = CODESLAYER_LIST_VIEW_GET_PRIVATE (list_view);

  if (!codeslayer_utils_has_text (text))
    return; 
  
  model = GTK_TREE_MODEL (priv->store);
  if (gtk_tree_model_get_iter_from_string (model, &iter, path))
    {
      GList *values;
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, TEXT, text, -1);
      values = get_list_values (list_view);
      g_signal_emit_by_name ((gpointer) list_view, "list-changed", values);
      g_list_foreach (values, (GFunc) g_free, NULL);
      g_list_free (values);
    }
}

static GList*
get_list_values (CodeSlayerListView *list_view)
{
  CodeSlayerListViewPrivate *priv;
  GList *values = NULL;
  
  priv = CODESLAYER_LIST_VIEW_GET_PRIVATE (list_view);
  
  gtk_tree_model_foreach (GTK_TREE_MODEL (priv->store), 
                          (GtkTreeModelForeachFunc) add_value, &values);
                          
  return values;
}

static gboolean 
add_value (GtkTreeModel *model,
           GtkTreePath  *path,
           GtkTreeIter  *iter,
           GList        **values)
{
  gchar *text;
  gtk_tree_model_get (model, iter, TEXT, &text, -1); 
  *values = g_list_append (*values, text);
  return FALSE;
}

gint
sort_compare (GtkTreeModel *model, 
              GtkTreeIter  *a,
              GtkTreeIter  *b, 
              gpointer      userdata)
{
  gint sortcol;
  gint ret = 0;

  sortcol = GPOINTER_TO_INT (userdata);
  
  switch (sortcol)
    {
    case TEXT:
      {
        gchar *text1, *text2;

        gtk_tree_model_get (model, a, TEXT, &text1, -1);
        gtk_tree_model_get (model, b, TEXT, &text2, -1);

        ret = g_strcmp0 (text1, text2);

        g_free (text1);
        g_free (text2);
      }
      break;
    }

  return ret;
}
