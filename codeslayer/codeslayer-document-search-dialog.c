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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <codeslayer/codeslayer-document-search-dialog.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-document-search-dialog
 * @short_description: Used to search for documents.
 * @title: CodeSlayerDocumentSearchDialog
 * @include: codeslayer/codeslayer-document-search-dialog.h
 */

static void codeslayer_document_search_dialog_class_init  (CodeSlayerDocumentSearchDialogClass *klass);
static void codeslayer_document_search_dialog_init        (CodeSlayerDocumentSearchDialog      *dialog);
static void codeslayer_document_search_dialog_finalize    (CodeSlayerDocumentSearchDialog      *dialog);

static gboolean key_release_action                        (CodeSlayerDocumentSearchDialog      *dialog,
                                                           GdkEventKey                         *event);
static gboolean key_press_action                          (CodeSlayerDocumentSearchDialog      *dialog,
                                                           GdkEventKey                         *event);
static gboolean can_refilter                              (CodeSlayerDocumentSearchDialog      *dialog, 
                                                           const gchar                         *text);
static void render_indexes                                (CodeSlayerDocumentSearchDialog      *dialog);
static void select_tree                                   (CodeSlayerDocumentSearchDialog      *dialog, 
                                                           GdkEventKey                         *event);
static void row_activated_action                          (CodeSlayerDocumentSearchDialog      *dialog);
static gboolean filter_callback                           (GtkTreeModel                        *model,
                                                           GtkTreeIter                         *iter,
                                                           CodeSlayerDocumentSearchDialog      *dialog);
static gchar* get_globbing                                (const gchar                         *entry, 
                                                           gboolean                             match_case);
static gint sort_compare                                  (GtkTreeModel                        *model, 
                                                           GtkTreeIter                         *a,
                                                           GtkTreeIter                         *b, 
                                                           gpointer                             userdata);

#define CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_DOCUMENT_SEARCH_DIALOG_TYPE, CodeSlayerDocumentSearchDialogPrivate))
  
#define MAX_RESULTS 50

typedef struct _CodeSlayerDocumentSearchDialogPrivate CodeSlayerDocumentSearchDialogPrivate;

struct _CodeSlayerDocumentSearchDialogPrivate
{
  GtkWindow          *window;
  CodeSlayerProfile  *profile;
  CodeSlayerProjects *projects;
  GtkWidget          *dialog;
  GtkWidget          *entry;
  GtkWidget          *tree;
  GtkListStore       *store;
  GtkTreeModel       *filter;
  gchar              *find_text; 
  gchar              *find_globbing;
  GPatternSpec       *find_pattern; 
};

enum
{
  FILE_NAME = 0,
  FILE_PATH,
  COLUMNS
};

G_DEFINE_TYPE (CodeSlayerDocumentSearchDialog, codeslayer_document_search_dialog, G_TYPE_OBJECT)

static void 
codeslayer_document_search_dialog_class_init (CodeSlayerDocumentSearchDialogClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_document_search_dialog_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerDocumentSearchDialogPrivate));
}

static void
codeslayer_document_search_dialog_init (CodeSlayerDocumentSearchDialog *dialog)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);
  priv->dialog = NULL;
  priv->filter = NULL;
  priv->find_text = NULL;
  priv->find_globbing = NULL;
  priv->find_pattern = NULL;
}

static void
codeslayer_document_search_dialog_finalize (CodeSlayerDocumentSearchDialog *dialog)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);
  
  if (priv->dialog != NULL)
    gtk_widget_destroy (priv->dialog);

  if (priv->find_pattern != NULL)
    g_pattern_spec_free (priv->find_pattern);

  if (priv->find_text != NULL)
    g_free (priv->find_text);
  
  if (priv->find_globbing != NULL)
    g_free (priv->find_globbing);
  
  G_OBJECT_CLASS (codeslayer_document_search_dialog_parent_class)-> finalize (G_OBJECT (dialog));
}

/**
 * codeslayer_document_search_index_dialog_new:
 * @window: a #GtkWindow.
 * @profile: a #CodeSlayerProfile.
 * @projects: a #CodeSlayerProjects.
 *
 * Creates a new #CodeSlayerDocumentSearchDialog.
 *
 * Returns: a new #CodeSlayerDocumentSearchDialog. 
 */
CodeSlayerDocumentSearchDialog*
codeslayer_document_search_dialog_new (GtkWindow          *window, 
                                       CodeSlayerProfile  *profile, 
                                       CodeSlayerProjects *projects)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  CodeSlayerDocumentSearchDialog *dialog;

  dialog = CODESLAYER_DOCUMENT_SEARCH_DIALOG (g_object_new (codeslayer_document_search_dialog_get_type (), NULL));
  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);
  priv->window = window;
  priv->profile = profile;
  priv->projects = projects;

  return dialog;
}

/**
 * codeslayer_document_search_dialog_run:
 * @dialog: a #CodeSlayerDocumentSearchDialog.
 */
void
codeslayer_document_search_dialog_run (CodeSlayerDocumentSearchDialog *dialog)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);

  if (priv->dialog == NULL)
    {
      GtkWidget *content_area;
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *label;
      GtkTreeSortable *sortable;
      GtkWidget *scrolled_window;
      GtkTreeViewColumn *column;
      GtkCellRenderer *renderer;

      priv->dialog = gtk_dialog_new_with_buttons ("Search For Document", 
                                                  GTK_WINDOW (priv->window),
                                                  GTK_DIALOG_MODAL,
                                                  _("Close"), GTK_RESPONSE_OK,
                                                  NULL);
      gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->dialog), TRUE);
      gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->dialog), TRUE);

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (priv->dialog));
      
      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
      gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
      
      /* the completion box */
      
      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
      gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);
      
      label = gtk_label_new ("File: ");
      priv->entry = gtk_entry_new ();
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
      gtk_box_pack_start (GTK_BOX (hbox), priv->entry, TRUE, TRUE, 2);
      
      /* the tree view */   
         
      priv->store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
      priv->tree =  gtk_tree_view_new ();
      gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree), FALSE);
      gtk_tree_view_set_enable_search (GTK_TREE_VIEW (priv->tree), FALSE);
      
      priv->filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (priv->store), NULL);
      gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (priv->filter),
                                              (GtkTreeModelFilterVisibleFunc) filter_callback,
                                               dialog, NULL);
      
      gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree), GTK_TREE_MODEL (priv->filter));
      g_object_unref (priv->store);
      
      sortable = GTK_TREE_SORTABLE (priv->store);
      gtk_tree_sortable_set_sort_func (sortable, FILE_NAME, sort_compare,
                                       GINT_TO_POINTER (FILE_NAME), NULL);
      gtk_tree_sortable_set_sort_column_id (sortable, FILE_NAME, GTK_SORT_ASCENDING);

      column = gtk_tree_view_column_new ();
      gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
      renderer = gtk_cell_renderer_text_new ();
      gtk_tree_view_column_pack_start (column, renderer, FALSE);
      gtk_tree_view_column_add_attribute (column, renderer, "text", FILE_NAME);
      gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree), column);
      
      column = gtk_tree_view_column_new ();
      gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
      renderer = gtk_cell_renderer_text_new ();
      gtk_tree_view_column_pack_start (column, renderer, FALSE);
      gtk_tree_view_column_add_attribute (column, renderer, "text", FILE_PATH);
      gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree), column);
      
      scrolled_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
      gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (priv->tree));
      
      /* hook up the signals */
      
      g_signal_connect_swapped (G_OBJECT (priv->entry), "key-release-event",
                                G_CALLBACK (key_release_action), dialog);
                                
      g_signal_connect_swapped (G_OBJECT (priv->entry), "key-press-event",
                                G_CALLBACK (key_press_action), dialog);
                                
      g_signal_connect_swapped (G_OBJECT (priv->tree), "row-activated",
                                G_CALLBACK (row_activated_action), dialog);                                
      
      /* render everything */
      
      gtk_widget_set_size_request (content_area, 600, 400);
      
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (content_area), vbox, TRUE, TRUE, 0);

      gtk_widget_show_all (content_area);
    }
    
  gtk_widget_grab_focus (priv->entry);
  gtk_dialog_run (GTK_DIALOG (priv->dialog));
  gtk_widget_hide (priv->dialog);
}

static gboolean
key_press_action (CodeSlayerDocumentSearchDialog *dialog,
                  GdkEventKey                    *event)
{
  if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
    {
      select_tree (dialog, event);    
      return TRUE;    
    }
  if (event->keyval == GDK_KEY_Return)
    {
      row_activated_action (dialog);
      return TRUE;    
    }
    
  return FALSE;
}

static gboolean
key_release_action (CodeSlayerDocumentSearchDialog *dialog,
                    GdkEventKey                    *event)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  gint text_length;
  
  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);
  
  if (event->keyval == GDK_KEY_Up ||
      event->keyval == GDK_KEY_Down ||
      event->keyval == GDK_KEY_Left ||
      event->keyval == GDK_KEY_Right ||
      event->keyval == GDK_KEY_Shift_L || 
      event->keyval == GDK_KEY_Shift_R)
    return FALSE;

  text_length = gtk_entry_get_text_length (GTK_ENTRY (priv->entry));
  
  if (text_length == 0)
    {
      gtk_list_store_clear (priv->store);
    }
  else if (text_length >= 1)
    {
      const gchar *text;
      gboolean refilter = FALSE;
      
      text = gtk_entry_get_text (GTK_ENTRY (priv->entry));
      
      if (g_strcmp0 (text, "*") == 0 || g_strcmp0 (text, "?") == 0)
        return FALSE;
        
      refilter = can_refilter (dialog, text);

      if (priv->find_globbing != NULL)
        g_free (priv->find_globbing);
        
      priv->find_globbing = get_globbing (text, TRUE);
      
      if (priv->find_pattern != NULL)
        g_pattern_spec_free (priv->find_pattern);
      
      priv->find_pattern = g_pattern_spec_new (priv->find_globbing);

      if (refilter)
        {
          gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (priv->filter));
        }
      else
        {
          gtk_list_store_clear (priv->store);
          render_indexes (dialog);
        }
    }

  return FALSE;
}

static gboolean
can_refilter (CodeSlayerDocumentSearchDialog *dialog,
              const gchar                    *text)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  GtkTreeModel *tree_model;
  gint count = 0;

  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);

  if (priv->find_text != NULL && !g_str_has_prefix (text, priv->find_text))
    return FALSE;
    
  tree_model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (priv->filter));
  count = gtk_tree_model_iter_n_children (tree_model, NULL);
  return count > 0 && count != MAX_RESULTS;
}

static void
render_indexes (CodeSlayerDocumentSearchDialog *dialog)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  GIOChannel *channel = NULL;
  gchar *line;
  gsize len;
  gint count = 0;

  gchar *profile_folder_path;
  gchar *profile_indexes_file;
  
  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);

  profile_folder_path = codeslayer_profile_get_config_folder_path (priv->profile);
  profile_indexes_file = g_strconcat (profile_folder_path, G_DIR_SEPARATOR_S, CODESLAYER_DOCUMENT_SEARCH_FILE, NULL);
  
  channel = g_io_channel_new_file (profile_indexes_file, "r", NULL);
  if (channel == NULL)
    {
      GtkWidget *dialog;
      dialog =  gtk_message_dialog_new (NULL, 
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                        "The documentsearch file does not exist.");
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_free (profile_folder_path);
      g_free (profile_indexes_file);
      return;
    }
  
  if (priv->find_text != NULL)
    g_free (priv->find_text);
  
  priv->find_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry)));
  
  while (g_io_channel_read_line (channel, &line, &len, NULL, NULL) != G_IO_STATUS_EOF)
    {
      gchar **split;
      gchar **tmp;
      gchar *file_name;  
      gchar *file_path;  

      if (!codeslayer_utils_has_text (line))
        continue;
    
      split = g_strsplit (line, "\t", -1);
      if (split == NULL)
        continue;
      
      tmp = split;
      file_name = *tmp;
      file_path = *++tmp;
      
      if (file_path != NULL)
        g_strstrip(file_path);
      
      if (g_pattern_match_string (priv->find_pattern, file_name))
        {
          GtkTreeIter iter;
          gtk_list_store_append (priv->store, &iter);
          gtk_list_store_set (priv->store, &iter, FILE_NAME, file_name, FILE_PATH, file_path, -1);
          count++;
        }
        
      g_strfreev (split);
      g_free (line);
      
      if (count >= MAX_RESULTS)
        break;
    }
    
  g_free (profile_folder_path);
  g_free (profile_indexes_file);
  g_io_channel_shutdown (channel, FALSE, NULL);
  g_io_channel_unref (channel);  
}

static gboolean
filter_callback (GtkTreeModel                   *model,
                 GtkTreeIter                    *iter,
                 CodeSlayerDocumentSearchDialog *dialog)
{  
  CodeSlayerDocumentSearchDialogPrivate *priv;
  gchar *value = NULL;

  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);
  
  if (priv->find_pattern == NULL)
    return FALSE;
  
  gtk_tree_model_get (model, iter, FILE_NAME, &value, -1);
  
  if (value == NULL)
    return FALSE;

  if (g_pattern_match_string (priv->find_pattern, value))
    {
      g_free (value);    
      return TRUE;
    }

  g_free (value);
  
  return FALSE;
}

static gchar*
get_globbing (const gchar *entry, 
              gboolean     match_case)
{
  gchar *entry_text;
  gchar *result;
  
  entry_text = g_strconcat (entry, "*", NULL);
  
  if (match_case)
    {
      result = entry_text;
    }
  else
    {
      result = codeslayer_utils_to_lowercase (entry_text);
      g_free (entry_text);
    }
  
  return result;
}

static void
select_tree (CodeSlayerDocumentSearchDialog *dialog, 
             GdkEventKey                    *event)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  
  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);
  
  if (gtk_tree_model_iter_n_children (GTK_TREE_MODEL (priv->filter), NULL) <= 0)
    return;  

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  
  if (gtk_tree_selection_get_selected (selection, &priv->filter, &iter))
    {
      GtkTreePath *path;
      path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->filter), &iter);

      if (event->keyval == GDK_KEY_Up)
        gtk_tree_path_prev (path);
      else if (event->keyval == GDK_KEY_Down)
        gtk_tree_path_next (path);

      if (path != NULL)
        {
          if (gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->filter), &iter, path))
            {
              gtk_tree_selection_select_iter (selection, &iter);
              gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->tree), path, 
                                            NULL, FALSE, 0.0, 0.0);
            }
          gtk_tree_path_free (path);
        }
    }
  else
    {
      if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->filter), &iter))
        gtk_tree_selection_select_iter (selection, &iter);
    }
}

static void
row_activated_action (CodeSlayerDocumentSearchDialog  *dialog)
{
  CodeSlayerDocumentSearchDialogPrivate *priv;
  GtkTreeSelection *tree_selection;
  GtkTreeModel *tree_model;
  GList *selected_rows = NULL;
  GList *tmp = NULL;  
  
  priv = CODESLAYER_DOCUMENT_SEARCH_DIALOG_GET_PRIVATE (dialog);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->tree));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);

  tmp = selected_rows;
  
  if (tmp != NULL)
    {
      GtkTreeIter treeiter;
      gchar *file_path;
      GtkTreePath *tree_path = tmp->data;
      CodeSlayerProject *project;
      CodeSlayerDocument *document;
      
      gtk_tree_model_get_iter (tree_model, &treeiter, tree_path);
      gtk_tree_model_get (GTK_TREE_MODEL (priv->filter), &treeiter, FILE_PATH, &file_path, -1);
      
      document = codeslayer_document_new ();
      codeslayer_document_set_file_path (document, file_path);
      codeslayer_document_set_line_number (document, 0);
      
      project = codeslayer_profile_get_project_by_file_path (priv->profile, file_path);
      codeslayer_document_set_project (document, project);
      codeslayer_projects_select_document (priv->projects, document);
      
      g_object_unref (document);

      gtk_widget_hide (priv->dialog);
      
      g_free (file_path);
      gtk_tree_path_free (tree_path);
    }

  g_list_free (selected_rows);
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
    case FILE_NAME:
      {
        gchar *text1, *text2;

        gtk_tree_model_get (model, a, FILE_NAME, &text1, -1);
        gtk_tree_model_get (model, b, FILE_NAME, &text2, -1);

        ret = g_strcmp0 (text1, text2);

        g_free (text1);
        g_free (text2);
      }
      break;
    }

  return ret;
}
