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
static void process_proposals                 (CodeSlayerCompletion         *completion, 
                                               CodeSlayerCompletionProvider *provider, 
                                               GtkTextIter                   iter);
                                       
#define CODESLAYER_COMPLETION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_COMPLETION_TYPE, CodeSlayerCompletionPrivate))

typedef struct _CodeSlayerCompletionPrivate CodeSlayerCompletionPrivate;

struct _CodeSlayerCompletionPrivate
{
  GList              *providers;
  GtkWindow          *window;
  GtkWidget          *popup;
  GtkWidget          *tree;
  GtkListStore       *store;
};

enum
{
  LABEL = 0,
  TEXT,
  COLUMNS
};

G_DEFINE_TYPE (CodeSlayerCompletion, codeslayer_completion, G_TYPE_OBJECT)
     
static void 
codeslayer_completion_class_init (CodeSlayerCompletionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
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
  
  list = priv->providers;
  while (list != NULL)
    {
      CodeSlayerCompletionProvider *provider = list->data;
      if (codeslayer_completion_provider_has_match (provider, iter))
        {
          if (priv->popup == NULL)
            create_window (completion);
            
          move_window (completion, text_view, iter);
            
          process_proposals (completion, provider, iter);
        }
      list = g_list_next (list);
    }
}

void
codeslayer_completion_hide (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  if (priv->popup != NULL && 
      gtk_widget_get_visible (priv->popup))
    gtk_widget_hide (priv->popup);
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
      path = gtk_tree_path_new_from_string ("1");      
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
      path = gtk_tree_path_new_from_string ("1");      
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
  GtkWidget *scrolled_window;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);

  priv->popup = gtk_window_new (GTK_WINDOW_TOPLEVEL);  
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->popup), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->popup), TRUE);
  gtk_window_set_decorated (GTK_WINDOW (priv->popup), FALSE);  
  gtk_window_set_transient_for (GTK_WINDOW (priv->popup), priv->window);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (priv->popup), TRUE);  
  
  priv->store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_STRING);  
  priv->tree =  gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (priv->tree), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree), GTK_TREE_MODEL (priv->store));
  g_object_unref (priv->store);
  
  column = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer, "text", LABEL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree), column);
  
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (priv->tree));
  gtk_widget_set_size_request (scrolled_window, 400, 300);

  gtk_container_add (GTK_CONTAINER (priv->popup), scrolled_window);
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
process_proposals (CodeSlayerCompletion         *completion, 
                   CodeSlayerCompletionProvider *provider, 
                   GtkTextIter                   iter)
{
  CodeSlayerCompletionPrivate *priv;
  GList *proposals;
  GList *list;

  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);

  proposals = codeslayer_completion_provider_get_proposals (provider, iter);
  list = proposals;
  
  /*g_print ("*****************************************************************\n");*/

  while (list != NULL)
    {
      CodeSlayerCompletionProposal *proposal = list->data;
      const gchar *label;
      const gchar *text;
      GtkTreeIter tree_iter;

      label = codeslayer_completion_proposal_get_label (proposal);
      text = codeslayer_completion_proposal_get_text (proposal);
      
      gtk_list_store_append (priv->store, &tree_iter);
      gtk_list_store_set (priv->store, &tree_iter, 
                          LABEL, label, 
                          TEXT, text, 
                          -1);
      
      /*g_print ("label %s \n", label);*/

      list = g_list_next (list);
    }
    
  g_list_foreach (proposals, (GFunc)g_object_unref, NULL);
  g_list_free (proposals);
  
  /*gtk_tree_view_columns_autosize (GTK_TREE_VIEW (priv->tree));
  gtk_widget_show_all (priv->popup);*/
}
