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
#include <codeslayer/codeslayer-profiles-manager.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-profiles-manager
 * @short_description: The profiles.
 * @title: CodeSlayerProfilesManager
 * @include: codeslayer/codeslayer-profiles-manager.h
 */

static void codeslayer_profiles_manager_class_init  (CodeSlayerProfilesManagerClass *klass);
static void codeslayer_profiles_manager_init        (CodeSlayerProfilesManager      *profiles_manager);
static void codeslayer_profiles_manager_finalize    (CodeSlayerProfilesManager      *profiles_manager);

static void add_profiles_pane                       (CodeSlayerProfilesManager      *profiles_manager, 
                                                     GtkWidget                      *hpaned);
static void add_buttons_pane                        (CodeSlayerProfilesManager      *profiles_manager, 
                                                     GtkWidget                      *hpaned);
static void select_row_action                       (GtkTreeSelection               *selection, 
                                                     CodeSlayerProfilesManager      *profiles_manager);
static void load_profiles                           (CodeSlayerProfilesManager      *profiles_manager);
static gint sort_compare                            (GtkTreeModel                   *model, 
                                                     GtkTreeIter                    *a,
                                                     GtkTreeIter                    *b, 
                                                     gpointer                        userdata);

#define CODESLAYER_PROFILES_MANAGER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROFILES_MANAGER_TYPE, CodeSlayerProfilesManagerPrivate))

typedef struct _CodeSlayerProfilesManagerPrivate CodeSlayerProfilesManagerPrivate;

struct _CodeSlayerProfilesManagerPrivate
{
  GtkWidget                *window;
  CodeSlayerProfiles       *profiles;
  CodeSlayerEngine         *engine;
  CodeSlayerProjectsEngine *projects_engine;
  GtkWidget                *tree;
  GtkListStore             *store;
};

enum
{
  TEXT = 0,
  PROFILE,
  COLUMNS
};

G_DEFINE_TYPE (CodeSlayerProfilesManager, codeslayer_profiles_manager, G_TYPE_OBJECT)

static void 
codeslayer_profiles_manager_class_init (CodeSlayerProfilesManagerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_profiles_manager_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerProfilesManagerPrivate));
}

static void
codeslayer_profiles_manager_init (CodeSlayerProfilesManager *profiles_manager)
{
}

static void
codeslayer_profiles_manager_finalize (CodeSlayerProfilesManager *profiles_manager)
{
  G_OBJECT_CLASS (codeslayer_profiles_manager_parent_class)->finalize (G_OBJECT (profiles_manager));
}

/**
 * codeslayer_profiles_manager_new:
 * @window: a #GtkWindow.
 *
 * Creates a new #CodeSlayerProfilesManager.
 *
 * Returns: a new #CodeSlayerProfilesManager. 
 */
CodeSlayerProfilesManager*
codeslayer_profiles_manager_new (GtkWidget                *window, 
                                 CodeSlayerProfiles       *profiles, 
                                 CodeSlayerEngine         *engine, 
                                 CodeSlayerProjectsEngine *projects_engine)
{
  CodeSlayerProfilesManagerPrivate *priv;
  CodeSlayerProfilesManager *profiles_manager;

  profiles_manager = g_object_new (codeslayer_profiles_manager_get_type (), NULL);
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  priv->window = window;
  priv->profiles = profiles;
  priv->engine = engine;
  priv->projects_engine = projects_engine;
  
  return profiles_manager;
}

/**
 * codeslayer_profiles_manager_run_dialog:
 * @profiles: a #CodeSlayerProfilesManager.
 *
 * Show the profiles dialog.
 */
void
codeslayer_profiles_manager_run_dialog (CodeSlayerProfilesManager *profiles_manager)
{
  CodeSlayerProfilesManagerPrivate *priv;
  /*CodeSlayerProfile *profile;*/
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *hbox;
  
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  /*profile = codeslayer_profiles_get_profile (priv->profiles);*/
  
  dialog = gtk_dialog_new_with_buttons (_("Profiles"), 
                                        GTK_WINDOW (priv->window),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                        NULL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_widget_set_size_request (content_area, 350, -1);
  
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  
  gtk_container_add (GTK_CONTAINER (content_area), hbox);
  add_profiles_pane (profiles_manager, hbox);
  add_buttons_pane (profiles_manager, hbox);
  
  load_profiles (profiles_manager);
  
  gtk_widget_show_all (content_area);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void
add_profiles_pane (CodeSlayerProfilesManager *profiles_manager, 
                   GtkWidget                 *hbox)
{
  CodeSlayerProfilesManagerPrivate *priv;
  GtkWidget *vbox;
  GtkWidget *tree;
  GtkListStore *store;
  GtkTreeSortable *sortable;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;
  GtkWidget *scrolled_window;
  
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  
  /* the tree */

  tree = gtk_tree_view_new ();
  priv->tree = tree;
  
  store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
  priv->store = store;
  
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), GTK_TREE_MODEL (store));
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

  sortable = GTK_TREE_SORTABLE (store);
  gtk_tree_sortable_set_sort_func (sortable, TEXT, sort_compare,
                                   GINT_TO_POINTER (TEXT), NULL);
  gtk_tree_sortable_set_sort_column_id (sortable, TEXT, GTK_SORT_ASCENDING);                                   
                           
  column = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_text_new ();
  
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer, "text", TEXT, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (tree));
  gtk_widget_set_size_request (scrolled_window, -1, 275);
  
  g_signal_connect (G_OBJECT (selection), "changed",
                    G_CALLBACK (select_row_action), profiles_manager);

  /* pack everything in */  

  gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
}

static void
add_buttons_pane (CodeSlayerProfilesManager *profiles_manager, 
                  GtkWidget                 *hbox)
{
  GtkWidget *vbox;
  GtkWidget *add;
  GtkWidget *edit;
  GtkWidget *delete;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  
  add = gtk_button_new_from_stock (GTK_STOCK_ADD);
  edit = gtk_button_new_from_stock (GTK_STOCK_EDIT);
  delete = gtk_button_new_from_stock (GTK_STOCK_DELETE);
  
  gtk_box_pack_start (GTK_BOX (vbox), add, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), edit, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), delete, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 10);
}

static void
select_row_action (GtkTreeSelection          *selection, 
                   CodeSlayerProfilesManager *profiles_manager)
{
 
}

static void
load_profiles (CodeSlayerProfilesManager *profiles_manager)
{
  CodeSlayerProfilesManagerPrivate *priv;
  GList *names;
  GList *list;
  GtkTreeIter iter;
  
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);

  names = codeslayer_profiles_get_profile_names (priv->profiles);

  list = names;
  while (list != NULL)
    {
      gchar *name = list->data;
      CodeSlayerProfile *profile;

      profile = codeslayer_profiles_retrieve_profile (priv->profiles, name);

      gtk_list_store_append (priv->store, &iter);
      gtk_list_store_set (priv->store, &iter, 
                          TEXT, name, 
                          PROFILE, profile,
                          -1);

      list = g_list_next (list);
    }
    
  g_list_free_full (names, g_free);
}

static gint
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

