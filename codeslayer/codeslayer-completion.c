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
static gboolean focus_out_action              (CodeSlayerCompletion         *completion, 
                                               GdkEvent                     *event);
                                       
#define CODESLAYER_COMPLETION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_COMPLETION_TYPE, CodeSlayerCompletionPrivate))

typedef struct _CodeSlayerCompletionPrivate CodeSlayerCompletionPrivate;

struct _CodeSlayerCompletionPrivate
{
  GList              *providers;
  GtkWindow          *window;
  GtkWidget          *popup;
  GtkWidget          *entry;
  GtkEntryCompletion *entry_completion;
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
codeslayer_completion_invoke (CodeSlayerCompletion *completion, 
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

static gboolean
focus_out_action (CodeSlayerCompletion *completion, 
                  GdkEvent             *event)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  gtk_widget_hide (priv->popup);
  return FALSE;
}

static void
create_window (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);

  priv->popup = gtk_window_new (GTK_WINDOW_TOPLEVEL);  
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->popup), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->popup), TRUE);
  gtk_window_set_decorated (GTK_WINDOW (priv->popup), FALSE);
  
  gtk_window_set_transient_for (GTK_WINDOW (priv->popup), priv->window);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (priv->popup), TRUE);  
  
  priv->store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_STRING);  

  priv->entry =  gtk_entry_new ();
  priv->entry_completion = gtk_entry_completion_new ();
  gtk_entry_set_completion (GTK_ENTRY (priv->entry), priv->entry_completion);  
  gtk_entry_completion_set_model (priv->entry_completion, GTK_TREE_MODEL (priv->store));  
  gtk_entry_completion_set_text_column (priv->entry_completion, 0);
  
  g_signal_connect_swapped (G_OBJECT (priv->entry), "focus-out-event",
                            G_CALLBACK (focus_out_action), completion);

  gtk_container_add (GTK_CONTAINER (priv->popup), priv->entry);
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
  /*gtk_window_present (GTK_WINDOW (priv->popup));*/
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
}
