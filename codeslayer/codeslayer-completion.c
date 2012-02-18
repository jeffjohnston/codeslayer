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

#include <string.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-completion.h>
#include <codeslayer/codeslayer-completion-provider.h>
#include <codeslayer/codeslayer-completion-proposal.h>

static void codeslayer_completion_class_init  (CodeSlayerCompletionClass    *klass);
static void codeslayer_completion_init        (CodeSlayerCompletion         *completion);
static void codeslayer_completion_finalize    (CodeSlayerCompletion         *completion);

static void create_window                     (CodeSlayerCompletion         *completion);
static void move_window                       (CodeSlayerCompletion         *completion, 
                                               GtkTextView                  *text_view, 
                                               GtkTextIter                   iter);
static void resize_window                     (CodeSlayerCompletion         *completion, 
                                               GtkTextView                  *text_view,
                                               GtkTextIter                   iter);
static void add_proposals                     (CodeSlayerCompletion         *completion,
                                               gchar                        *filter);                                               
static void add_proposal                      (CodeSlayerCompletionProposal *proposal, 
                                               GtkListStore                 *store, 
                                               gchar                        *filter);
static void row_activated_action              (CodeSlayerCompletion         *completion,
                                               GtkTreePath                  *path,
                                               GtkTreeViewColumn            *column);
                                       
#define CODESLAYER_COMPLETION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_COMPLETION_TYPE, CodeSlayerCompletionPrivate))

typedef struct _CodeSlayerCompletionPrivate CodeSlayerCompletionPrivate;

struct _CodeSlayerCompletionPrivate
{
  GList        *providers;
  GtkWindow    *window;
  GtkWidget    *popup;
  GtkWidget    *scrolled_window;
  GtkWidget    *tree;
  GtkListStore *store;
  GtkTextMark  *mark;
  GList        *proposals;
};

enum
{
  LABEL = 0,
  TEXT,
  COLUMNS
};

G_DEFINE_TYPE (CodeSlayerCompletion, codeslayer_completion, G_TYPE_OBJECT)

enum
{
  ROW_SELECTED,
  LAST_SIGNAL
};

static guint codeslayer_completion_signals[LAST_SIGNAL] = { 0 };      
     
static void 
codeslayer_completion_class_init (CodeSlayerCompletionClass *klass)
{
  GObjectClass *gobject_class;

  /**
   * CodeSlayerCompletion::row-selected
   * @completion: the completion that received the signal
   *
   * The ::row-selected signal enables the current selection to be set in the editor.
   */
  codeslayer_completion_signals[ROW_SELECTED] =
    g_signal_new ("row-selected", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerCompletionClass, row_selected),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_completion_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerCompletionPrivate));
}

static void
codeslayer_completion_init (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion); 
  priv->popup = NULL;
}

static void
codeslayer_completion_finalize (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion); 

  if (priv->proposals != NULL)
    {
      g_list_foreach (priv->proposals, (GFunc)g_object_unref, NULL);
      g_list_free (priv->proposals);
      priv->proposals = NULL;
    }

  if (priv->store != NULL)
    gtk_list_store_clear (priv->store);
  
  if (priv->providers != NULL)
    {
      g_list_foreach (priv->providers, (GFunc)g_object_unref, NULL);
      g_list_free (priv->providers);  
      priv->providers = NULL;
    }
   
  G_OBJECT_CLASS (codeslayer_completion_parent_class)->finalize (G_OBJECT (completion));
}

CodeSlayerCompletion*
codeslayer_completion_new (GtkWindow *window)
{
  CodeSlayerCompletionPrivate *priv;
  CodeSlayerCompletion *completion;

  completion = CODESLAYER_COMPLETION (g_object_new (codeslayer_completion_get_type (), NULL));
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  priv->window = window;

  return completion;
}

void
codeslayer_completion_add_provider  (CodeSlayerCompletion         *completion, 
                                     CodeSlayerCompletionProvider *provider)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  g_object_ref_sink (provider);
  priv->providers = g_list_append (priv->providers, provider);
}                                     

void
codeslayer_completion_show (CodeSlayerCompletion *completion, 
                            GtkTextView          *text_view,
                            GtkTextIter           iter)
{
  CodeSlayerCompletionPrivate *priv;
  GList *list;

  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
      
  if (priv->proposals != NULL)
    {
      g_list_foreach (priv->proposals, (GFunc)g_object_unref, NULL);
      g_list_free (priv->proposals);
      priv->proposals = NULL;
    }

  if (priv->store != NULL)
    gtk_list_store_clear (priv->store);

  list = priv->providers;
  while (list != NULL)
    {
      CodeSlayerCompletionProvider *provider = list->data;
      if (codeslayer_completion_provider_has_match (provider, iter))
        {
          GList *proposals;
          proposals = codeslayer_completion_provider_get_proposals (provider, iter);
          priv->proposals = g_list_concat (priv->proposals, proposals);
        }
      list = g_list_next (list);
    }
    
  if (priv->proposals != NULL)
    {
      GtkTextBuffer *buffer;
      buffer = gtk_text_view_get_buffer (text_view);
      priv->mark = gtk_text_buffer_create_mark (buffer, NULL, &iter, TRUE);

      if (priv->popup == NULL)
        create_window (completion);
        
      add_proposals (completion, NULL);
      resize_window (completion, text_view, iter);
      move_window (completion, text_view, iter);
      gtk_widget_show_all (priv->popup);
    }
}

void
codeslayer_completion_filter (CodeSlayerCompletion *completion, 
                              GtkTextView          *text_view,
                              GtkTextIter           iter)
{
  CodeSlayerCompletionPrivate *priv;
  
  GtkTextBuffer *buffer;
  GtkTextIter start;
  gchar *filter;

  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);

  if (priv->store != NULL)
    gtk_list_store_clear (priv->store);

  buffer = gtk_text_view_get_buffer (text_view);
  gtk_text_buffer_get_iter_at_mark (buffer, &start, priv->mark);
  
  filter = gtk_text_buffer_get_text (buffer, &start, &iter, FALSE);
  if (filter != NULL)
    {
      add_proposals (completion, filter);
      resize_window (completion, text_view, iter);
      gtk_widget_show_all (priv->popup);
      g_free (filter);
    }
}

void
codeslayer_completion_select (CodeSlayerCompletion *completion, 
                              GtkTextView          *text_view, 
                              GtkTextIter           iter)
{
  CodeSlayerCompletionPrivate *priv;
  GtkTreeSelection *tree_selection;
  GtkTreeModel *tree_model;
  GList *selected_rows = NULL;
  GList *tmp = NULL;  
  
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->tree));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);

  tmp = selected_rows;
  
  if (tmp != NULL)
    {
      GtkTreeIter treeiter;
      GtkTextIter start;
      gchar *text;
      GtkTextBuffer *buffer;
      GtkTreePath *tree_path = tmp->data;

      buffer = gtk_text_view_get_buffer (text_view);
      gtk_tree_model_get_iter (tree_model, &treeiter, tree_path);
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &treeiter, TEXT, &text, -1);

      gtk_text_buffer_begin_user_action (buffer);
      gtk_text_buffer_get_iter_at_mark (buffer, &start, priv->mark);
      gtk_text_buffer_delete (buffer, &start, &iter);
      gtk_text_buffer_insert (buffer, &start, text, -1);
      gtk_text_buffer_end_user_action (buffer);
    
      g_free (text);
      gtk_tree_path_free (tree_path);
    }

  g_list_free (selected_rows);
}

void
codeslayer_completion_hide (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  if (priv->popup != NULL && gtk_widget_get_visible (priv->popup))
    gtk_widget_hide (priv->popup);
}

gboolean
codeslayer_completion_get_visible (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  return priv->popup != NULL && gtk_widget_get_visible (priv->popup);
}

gboolean
codeslayer_completion_toggle_up (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GtkTreePath *path;

  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  
  if (!gtk_widget_get_visible (priv->popup))
    return FALSE;
  
  tree_model = GTK_TREE_MODEL (priv->store);
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));  
  if (gtk_tree_selection_get_selected (selection, &tree_model, &iter))
    {
      path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->store), &iter);
      gtk_tree_path_prev (path);
    }
  else
    {
      path = gtk_tree_path_new_from_string ("0");      
    }

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->tree), path, NULL, FALSE);
  
  return TRUE;
}

gboolean 
codeslayer_completion_toggle_down (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GtkTreePath *path;

  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  
  if (!gtk_widget_get_visible (priv->popup))
    return FALSE;
  
  tree_model = GTK_TREE_MODEL (priv->store);
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));  
  if (gtk_tree_selection_get_selected (selection, &tree_model, &iter))
    {
      path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->store), &iter);
      gtk_tree_path_next (path);
    }
  else
    {
      path = gtk_tree_path_new_from_string ("0");      
    }

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->tree), path, NULL, FALSE);
  
  return TRUE;
}

gboolean
codeslayer_completion_mouse_within_popup (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  GdkWindow *window;
  gint x, y, w, h;
  
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  
  if (priv->popup == NULL)
    return FALSE;
    
  window = gtk_widget_get_window (priv->popup);
  gdk_window_get_pointer (window, &x, &y, NULL);
  w = gdk_window_get_width (window);
  h = gdk_window_get_height (window);
  return x > 0 && x <= w && y > 0 && y <= h;
}                                     

static void
create_window (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);

  priv->popup = gtk_window_new (GTK_WINDOW_TOPLEVEL);  
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->popup), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->popup), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (priv->popup), priv->window);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (priv->popup), TRUE); 
  gtk_window_set_type_hint (GTK_WINDOW (priv->popup), GDK_WINDOW_TYPE_HINT_MENU);
  
  priv->store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
  priv->tree =  gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (priv->tree), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree), GTK_TREE_MODEL (priv->store));
  g_object_unref (priv->store);
  
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer, "text", LABEL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree), column);
  
  priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (priv->scrolled_window), GTK_WIDGET (priv->tree));
  gtk_container_add (GTK_CONTAINER (priv->popup), priv->scrolled_window);
  
  g_signal_connect_swapped (G_OBJECT (priv->tree), "row-activated",
                            G_CALLBACK (row_activated_action), completion);
}

static void
resize_window (CodeSlayerCompletion *completion, 
               GtkTextView          *text_view,
               GtkTextIter           iter)
{
  CodeSlayerCompletionPrivate *priv;
  GtkRequisition requisition;
  gint width;
  gint height;
  gint slider_width;
  GtkWidget *vscrollbar;
  
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);

  gtk_widget_get_preferred_size (priv->tree, &requisition, NULL);

  vscrollbar = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (priv->scrolled_window));

  gtk_widget_style_get (vscrollbar, "slider-width", &slider_width, NULL);

  width = requisition.width + slider_width + 2;
  if (width > 500)
    width = 500;

  height = requisition.height + slider_width + 2;
  if (height > 300)
    height = 300;

  gtk_window_resize (GTK_WINDOW (priv->popup), width, height);
}

static void
move_window (CodeSlayerCompletion *completion, 
             GtkTextView          *text_view, 
             GtkTextIter           iter)
{
  CodeSlayerCompletionPrivate *priv;
  GdkWindow *win;
  GdkRectangle buf_loc;
  gint x, y;
  gint win_x, win_y;
  
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
    
  gtk_text_view_get_iter_location (GTK_TEXT_VIEW (text_view), &iter, &buf_loc);
  
  gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (text_view),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         buf_loc.x, buf_loc.y,
                                         &win_x, &win_y);
                                         
  win = gtk_text_view_get_window (GTK_TEXT_VIEW (text_view), 
                                  GTK_TEXT_WINDOW_WIDGET);                                         
                                         
  gdk_window_get_origin (win, &x, &y);                                         
  gtk_window_move (GTK_WINDOW (priv->popup), win_x + x, win_y + y + buf_loc.height);                                         
  gtk_widget_show_all (priv->popup);
}

static void
add_proposals (CodeSlayerCompletion *completion, 
               gchar                *filter)
{
  CodeSlayerCompletionPrivate *priv;
  GList *list;

  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  
  list = priv->proposals;  
  while (list != NULL)
    {
      CodeSlayerCompletionProposal *proposal = list->data;
      add_proposal (proposal, priv->store, filter);
      list = g_list_next (list);
    }
}

static void
add_proposal (CodeSlayerCompletionProposal *proposal, 
              GtkListStore                 *store, 
              gchar                        *filter)
{
  const gchar *label;
  const gchar *text;
  GtkTreeIter tree_iter;
  
  label = codeslayer_completion_proposal_get_label (proposal);

  if (filter != NULL && !g_str_has_prefix (label, filter))
    return;

  text = codeslayer_completion_proposal_get_text (proposal);

  gtk_list_store_append (store, &tree_iter);
  gtk_list_store_set (store, &tree_iter, 
                      LABEL, label, 
                      TEXT, text, 
                      -1);
}

static void
row_activated_action (CodeSlayerCompletion *completion,
                      GtkTreePath          *path,
                      GtkTreeViewColumn    *column)
{
  g_signal_emit_by_name ((gpointer) completion, "row-selected");
}                     
