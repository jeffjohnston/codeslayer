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

#include <gtksourceview/gtksourceview.h>
#include <codeslayer/codeslayer-statusbar.h>
#include <codeslayer/codeslayer-process.h>

static void codeslayer_statusbar_class_init  (CodeSlayerStatusbarClass *klass);
static void codeslayer_statusbar_init        (CodeSlayerStatusbar      *statusbar);
static void codeslayer_statusbar_finalize    (CodeSlayerStatusbar      *statusbar);

static void process_started_action           (CodeSlayerStatusbar      *statusbar, 
                                              CodeSlayerProcess        *process);
static void process_finished_action          (CodeSlayerStatusbar      *statusbar, 
                                              CodeSlayerProcess        *process);
                                                   
static gboolean remove_finished_process      (GtkTreeModel             *model,
                                              GtkTreePath              *path,
                                              GtkTreeIter              *iter,
                                              CodeSlayerProcess        *process);
static void stop_action                      (CodeSlayerStatusbar      *statusbar);
static gboolean show_popup_menu              (CodeSlayerStatusbar      *statusbar, 
                                              GdkEventButton           *event);
                                              
static void set_label_text                   (CodeSlayerStatusbar      *statusbar);
static void expanded_action                  (CodeSlayerStatusbar      *statusbar);

#define CODESLAYER_STATUSBAR_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_STATUSBAR_TYPE, CodeSlayerStatusbarPrivate))

typedef struct _CodeSlayerStatusbarPrivate CodeSlayerStatusbarPrivate;

struct _CodeSlayerStatusbarPrivate
{
  CodeSlayerProcesses *processes;
  GtkWidget           *tree;
  GtkListStore        *store;
  GtkWidget           *menu;
  GtkWidget           *expander;
  GtkWidget           *label;
  GtkWidget           *stop_item;
};

enum
{
  ICON,
  TEXT,
  PROCESS,
  COLUMNS
};

G_DEFINE_TYPE (CodeSlayerStatusbar, codeslayer_statusbar, GTK_TYPE_VBOX)

static void
codeslayer_statusbar_class_init (CodeSlayerStatusbarClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_statusbar_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerStatusbarPrivate));
}

static void
codeslayer_statusbar_init (CodeSlayerStatusbar *statusbar) 
{
  CodeSlayerStatusbarPrivate *priv;
  GtkWidget *tree;
  GtkListStore *store;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;
  GtkWidget *scrolled_window;
  
  priv = CODESLAYER_STATUSBAR_GET_PRIVATE (statusbar);
  
  tree = gtk_tree_view_new ();
  priv->tree = tree;
  store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
  priv->store = store;
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), GTK_TREE_MODEL (store));
  g_object_unref (store);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  
  column = gtk_tree_view_column_new ();

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer, "stock-id", ICON, NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer, "text", TEXT, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), tree);
  
  g_signal_connect_swapped (G_OBJECT (priv->tree), "button_press_event",
                            G_CALLBACK (show_popup_menu), statusbar);

  priv->menu = gtk_menu_new ();

  priv->stop_item = gtk_menu_item_new_with_label ("Stop Process");
  g_signal_connect_swapped (G_OBJECT (priv->stop_item), "activate",
                            G_CALLBACK (stop_action), statusbar);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->stop_item);
  
  priv->expander = gtk_expander_new (NULL);
  priv->label = gtk_label_new (NULL);
  gtk_expander_set_label_widget (GTK_EXPANDER (priv->expander), priv->label);
  gtk_expander_set_expanded (GTK_EXPANDER (priv->expander), FALSE);
  
  gtk_container_add (GTK_CONTAINER (priv->expander), scrolled_window);  

  gtk_box_pack_start (GTK_BOX (statusbar), priv->expander, FALSE, FALSE, 0);
  
  g_signal_connect_swapped (G_OBJECT (priv->store), "row-deleted",
                            G_CALLBACK (set_label_text), CODESLAYER_STATUSBAR(statusbar));

  g_signal_connect_swapped (G_OBJECT (priv->expander), "notify::expanded",
                            G_CALLBACK (expanded_action), CODESLAYER_STATUSBAR(statusbar));
}

static void
codeslayer_statusbar_finalize (CodeSlayerStatusbar *statusbar)
{
  G_OBJECT_CLASS (codeslayer_statusbar_parent_class)->finalize (G_OBJECT(statusbar));
}

GtkWidget*
codeslayer_statusbar_new (CodeSlayerProcesses *processes)
{
  CodeSlayerStatusbarPrivate *priv;
  GtkWidget *statusbar;

  statusbar = g_object_new (codeslayer_statusbar_get_type (), NULL);
  priv = CODESLAYER_STATUSBAR_GET_PRIVATE (statusbar);
  priv->processes = processes;

  g_signal_connect_swapped (G_OBJECT (priv->processes), "process-started",
                            G_CALLBACK (process_started_action), CODESLAYER_STATUSBAR(statusbar));

  g_signal_connect_swapped (G_OBJECT (priv->processes), "process-finished",
                            G_CALLBACK (process_finished_action), CODESLAYER_STATUSBAR(statusbar));

  return statusbar;
}

static void
stop_action (CodeSlayerStatusbar *statusbar)
{
  CodeSlayerStatusbarPrivate *priv;
  GtkTreeSelection *tree_selection;
  CodeSlayerProcess *process;
  GtkTreeModel *tree_model;
  GtkTreeIter iter;

  priv = CODESLAYER_STATUSBAR_GET_PRIVATE (statusbar);

  tree_model = GTK_TREE_MODEL (priv->store);
  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  gtk_tree_selection_get_selected (tree_selection, &tree_model, &iter);
  
  gtk_tree_model_get (tree_model, &iter, PROCESS, &process, -1);
  
  if (process != NULL)
    codeslayer_process_stop (process);
}

static gboolean
show_popup_menu (CodeSlayerStatusbar *statusbar, 
                 GdkEventButton      *event)
{
  CodeSlayerStatusbarPrivate *priv;
  
  priv = CODESLAYER_STATUSBAR_GET_PRIVATE (statusbar);

  if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
      GtkTreeSelection *tree_selection;
      GtkTreeModel *tree_model;
      GtkTreeIter iter;

      tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
      if (gtk_tree_selection_get_selected (tree_selection, &tree_model, &iter))
        {
          CodeSlayerProcess *process;
          GtkTreePath *path;

          gtk_tree_model_get (tree_model, &iter, PROCESS, &process, -1);
          
          if (codeslayer_process_get_func (process) == NULL)
            return FALSE;

          /* Get tree path for row that was clicked */
          if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (priv->tree),
                                            (gint) event->x, 
                                            (gint) event->y,
                                            &path, NULL, NULL, NULL))
            {
              gtk_tree_selection_unselect_all (tree_selection);
              gtk_tree_selection_select_path (tree_selection, path);
              gtk_tree_path_free (path);
            }
        }

      if (gtk_tree_model_iter_n_children (GTK_TREE_MODEL (priv->store), NULL) > 0)
        {
          gtk_widget_show_all (priv->stop_item);
          
          gtk_menu_popup (GTK_MENU (priv->menu), NULL, NULL, NULL, NULL, 
                          (event != NULL) ? event->button : 0,
                          gdk_event_get_time ((GdkEvent *) event));
                          
          return TRUE;
        }
    }

  return FALSE;
}

static void
process_started_action (CodeSlayerStatusbar *statusbar, 
                        CodeSlayerProcess   *process)
{
  CodeSlayerStatusbarPrivate *priv;
  const gchar* name;
  GtkTreeIter iter;

  priv = CODESLAYER_STATUSBAR_GET_PRIVATE (statusbar);
  
  name = codeslayer_process_get_name (process);
  
  if (!gtk_expander_get_expanded (GTK_EXPANDER (priv->expander)))
    gtk_label_set_text (GTK_LABEL (priv->label), name);
  
  gtk_list_store_append (priv->store, &iter);
  gtk_list_store_set (priv->store, &iter, ICON, GTK_STOCK_EXECUTE, TEXT, name, PROCESS, process, -1);  
}

static void
process_finished_action (CodeSlayerStatusbar *statusbar, 
                         CodeSlayerProcess   *process)
{
  CodeSlayerStatusbarPrivate *priv;
  priv = CODESLAYER_STATUSBAR_GET_PRIVATE (statusbar);
  gtk_tree_model_foreach (GTK_TREE_MODEL (priv->store), 
                          (GtkTreeModelForeachFunc) remove_finished_process, process);
  
}

static gboolean 
remove_finished_process (GtkTreeModel      *model,
                         GtkTreePath       *path,
                         GtkTreeIter       *iter,
                         CodeSlayerProcess *process)
{
  CodeSlayerProcess *model_process;
  gtk_tree_model_get (model, iter, PROCESS, &model_process, -1);
  
  if (model_process == process)
    {
      gtk_list_store_remove (GTK_LIST_STORE (model), iter);
      return TRUE;
    }
  
  return FALSE;
}

static void
expanded_action (CodeSlayerStatusbar *statusbar)
{
  CodeSlayerStatusbarPrivate *priv;
  priv = CODESLAYER_STATUSBAR_GET_PRIVATE (statusbar);

  if (gtk_expander_get_expanded (GTK_EXPANDER (priv->expander)))
    gtk_label_set_text (GTK_LABEL (priv->label), NULL);
  else
    set_label_text (statusbar);
}

static void
set_label_text (CodeSlayerStatusbar *statusbar)
{
  CodeSlayerStatusbarPrivate *priv;
  GtkTreeIter iter;
  GtkTreeIter tmp;
  gchar *text;
  
  priv = CODESLAYER_STATUSBAR_GET_PRIVATE (statusbar);

  if (gtk_expander_get_expanded (GTK_EXPANDER (priv->expander)))
    {
      gtk_label_set_text (GTK_LABEL (priv->label), NULL);
      return;    
    }

  if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->store), &iter))
    {
      gtk_label_set_text (GTK_LABEL (priv->label), NULL);
      return;
    }

  tmp = iter;
  
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->store), &iter))
    tmp = iter;
  
  gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &tmp, TEXT, &text, -1);
  gtk_label_set_text (GTK_LABEL (priv->label), text);  
  g_free (text);  
}
