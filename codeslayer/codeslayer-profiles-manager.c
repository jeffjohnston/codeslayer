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
#include <codeslayer/codeslayer-registry.h>
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
static void load_profiles                           (CodeSlayerProfilesManager      *profiles_manager);
static void select_current_profile                  (CodeSlayerProfilesManager      *profiles_manager);
static gint sort_compare                            (GtkTreeModel                   *model, 
                                                     GtkTreeIter                    *a,
                                                     GtkTreeIter                    *b, 
                                                     gpointer                        userdata);
static void select_row_action                       (GtkTreeSelection               *selection, 
                                                     CodeSlayerProfilesManager      *profiles_manager);
static void add_profile_action                      (CodeSlayerProfilesManager      *profiles_manager);
static void edit_profile_action                     (CodeSlayerProfilesManager      *profiles_manager);
static void delete_profile_action                   (CodeSlayerProfilesManager      *profiles_manager);

#define CODESLAYER_PROFILES_MANAGER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROFILES_MANAGER_TYPE, CodeSlayerProfilesManagerPrivate))

typedef struct _CodeSlayerProfilesManagerPrivate CodeSlayerProfilesManagerPrivate;

struct _CodeSlayerProfilesManagerPrivate
{
  GtkWidget          *window;
  GtkWidget          *dialog;
  GtkWidget          *edit_button;
  GtkWidget          *delete_button;
  CodeSlayerProfiles *profiles;
  CodeSlayerEngine   *engine;
  GtkWidget          *tree;
  GtkListStore       *store;
};

enum
{
  TEXT = 0,
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
                                 CodeSlayerEngine         *engine)
{
  CodeSlayerProfilesManagerPrivate *priv;
  CodeSlayerProfilesManager *profiles_manager;

  profiles_manager = g_object_new (codeslayer_profiles_manager_get_type (), NULL);
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  priv->window = window;
  priv->profiles = profiles;
  priv->engine = engine;
  
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
  gint response;
  GtkWidget *content_area;
  GtkWidget *hbox;
  
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  
  priv->dialog = gtk_dialog_new_with_buttons (_("Profiles"), 
                                              GTK_WINDOW (priv->window),
                                              GTK_DIALOG_MODAL,
                                              GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                              GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                              NULL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->dialog), TRUE);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (priv->dialog));
  gtk_widget_set_size_request (content_area, 350, -1);
  
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  
  gtk_container_add (GTK_CONTAINER (content_area), hbox);
  add_profiles_pane (profiles_manager, hbox);
  add_buttons_pane (profiles_manager, hbox);
  
  load_profiles (profiles_manager);
  select_current_profile (profiles_manager);
  
  gtk_widget_show_all (content_area);
  
  response = gtk_dialog_run (GTK_DIALOG (priv->dialog));
  if (response == GTK_RESPONSE_OK)
    {
      if (codeslayer_abstract_engine_save_profile (CODESLAYER_ABSTRACT_ENGINE (priv->engine)))
        {
          GtkTreeSelection *selection;
          GtkTreeIter iter;          
          selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
          if (gtk_tree_selection_get_selected (selection, NULL, &iter))
            {
              CodeSlayerProfile *profile;
              gchar *name;
                    
              gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, TEXT, &name, -1);
              
              profile = codeslayer_profiles_retrieve_profile (priv->profiles, name);
              
              codeslayer_profiles_set_current_profile (priv->profiles, profile);

              codeslayer_engine_load_profile (CODESLAYER_ENGINE (priv->engine));
              
              g_free (name);
            }
        }
    }

  gtk_widget_destroy (priv->dialog);
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
  CodeSlayerProfilesManagerPrivate *priv;
  GtkWidget *vbox;
  GtkWidget *add_button;

  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  
  add_button = gtk_button_new_from_stock (GTK_STOCK_ADD);
  priv->edit_button = gtk_button_new_from_stock (GTK_STOCK_EDIT);
  priv->delete_button = gtk_button_new_from_stock (GTK_STOCK_DELETE);
  
  gtk_box_pack_start (GTK_BOX (vbox), add_button, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), priv->edit_button, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), priv->delete_button, FALSE, FALSE, 2);
  
  g_signal_connect_swapped (G_OBJECT (add_button), "clicked", 
                            G_CALLBACK (add_profile_action), profiles_manager);

  g_signal_connect_swapped (G_OBJECT (priv->edit_button), "clicked", 
                            G_CALLBACK (edit_profile_action), profiles_manager);

  g_signal_connect_swapped (G_OBJECT (priv->delete_button), "clicked", 
                            G_CALLBACK (delete_profile_action), profiles_manager);

  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 10);
}

static void
select_row_action (GtkTreeSelection          *selection, 
                   CodeSlayerProfilesManager *profiles_manager)
{
  CodeSlayerProfilesManagerPrivate *priv;
  GtkTreeIter iter;  

  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gchar *name;
            
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, TEXT, &name, -1);

      if (g_strcmp0 (name, CODESLAYER_PROFILES_DEFAULT) == 0)
        {
          gtk_widget_set_sensitive (priv->edit_button, FALSE);
          gtk_widget_set_sensitive (priv->delete_button, FALSE);
        }
      else
        {
          gtk_widget_set_sensitive (priv->edit_button, TRUE);
          gtk_widget_set_sensitive (priv->delete_button, TRUE);
        }        
      
      g_free (name);
    }
}

static void
add_profile_action (CodeSlayerProfilesManager *profiles_manager)
{
  CodeSlayerProfilesManagerPrivate *priv;
  GtkWidget *dialog;
  GtkWidget *content_area;
  gint response;
  GtkWidget *grid;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *toggle_button;

  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  
  dialog = gtk_dialog_new_with_buttons (_("Add Profile"), 
                                        GTK_WINDOW (priv->dialog),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                                        NULL);

  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
  
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  
  grid = gtk_grid_new ();

  label = gtk_label_new (_("Name:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (label), 4, 0);
  entry = gtk_entry_new ();
  toggle_button = gtk_check_button_new_with_label (_("Enable Projects?"));
  
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gtk_grid_attach_next_to (GTK_GRID (grid), entry, label, GTK_POS_RIGHT, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), toggle_button, 1, 1, 1, 1);
  
  gtk_container_add (GTK_CONTAINER (content_area), grid);
  
  gtk_widget_show_all (content_area);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      const gchar *name;
      name = gtk_entry_get_text (GTK_ENTRY (entry));
      if (codeslayer_utils_has_text (name))
        {
          CodeSlayerProfile *profile;
          CodeSlayerRegistry *registry;
          gboolean active;
          GtkTreeIter iter;

          profile = codeslayer_profiles_create_profile (priv->profiles, name);
          registry = codeslayer_profile_get_registry (profile);
          
          active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button));      
          codeslayer_registry_set_boolean (registry, 
                                           CODESLAYER_REGISTRY_ENABLE_PROJECTS, 
                                           active);
                                           
          codeslayer_profiles_save_profile (priv->profiles, profile);
          
          gtk_list_store_append (priv->store, &iter);
          gtk_list_store_set (priv->store, &iter, TEXT, name, -1);
                                        
          g_object_unref (profile);
        }
    }
  
  gtk_widget_destroy (dialog);
}

static void
edit_profile_action (CodeSlayerProfilesManager *profiles_manager)
{
  CodeSlayerProfilesManagerPrivate *priv;
  CodeSlayerProfile *profile;
  CodeSlayerRegistry *registry;
  GtkWidget *dialog;
  GtkWidget *content_area;
  gint response;
  GtkWidget *grid;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *toggle_button;
  GtkTreeSelection *selection;
  GtkTreeIter iter;

  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  
  dialog = gtk_dialog_new_with_buttons (_("Edit Profile"), 
                                        GTK_WINDOW (priv->dialog),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                                        NULL);

  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
  
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  
  grid = gtk_grid_new ();

  label = gtk_label_new (_("Name:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (label), 4, 0);
  entry = gtk_entry_new ();
  toggle_button = gtk_check_button_new_with_label (_("Enable Projects?"));
  
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gtk_grid_attach_next_to (GTK_GRID (grid), entry, label, GTK_POS_RIGHT, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), toggle_button, 1, 1, 1, 1);
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gchar *name;
      gboolean active;
            
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, TEXT, &name, -1);
      
      profile = codeslayer_profiles_retrieve_profile (priv->profiles, name);
      registry = codeslayer_profile_get_registry (profile);
      
      gtk_entry_set_text (GTK_ENTRY (entry), name);
      
      active = codeslayer_registry_get_boolean (registry, 
                                                CODESLAYER_REGISTRY_ENABLE_PROJECTS);

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_button), active);
      
      g_free (name);
    }
  
  gtk_container_add (GTK_CONTAINER (content_area), grid);
  
  gtk_widget_show_all (content_area);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      const gchar *name;
      name = gtk_entry_get_text (GTK_ENTRY (entry));
      if (codeslayer_utils_has_text (name))
        {
          gboolean active;

          active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button));      
          codeslayer_registry_set_boolean (registry, 
                                           CODESLAYER_REGISTRY_ENABLE_PROJECTS, 
                                           active);
                                           
          codeslayer_profiles_save_profile (priv->profiles, profile);
        }
    }
    
  g_object_unref (profile);
  gtk_widget_destroy (dialog);
}

static void
delete_profile_action (CodeSlayerProfilesManager *profiles_manager)
{
  CodeSlayerProfilesManagerPrivate *priv;
  GtkWidget *dialog;
  gint response;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);

  dialog = gtk_message_dialog_new (GTK_WINDOW (priv->dialog), 
                                   GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_WARNING,
                                   GTK_BUTTONS_OK_CANCEL,
                                   _("Are you sure you want to remove the profile?"));
  gtk_window_set_title (GTK_WINDOW (dialog), _("Remove Profile"));
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_CANCEL)
    {
      gtk_widget_destroy (dialog);
      return;
    }
  gtk_widget_destroy (dialog);

  /* confirmed that will remove the project */

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gchar *name;
      gchar *file_path;
      GFile *file;
      GError *error = NULL;
            
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, TEXT, &name, -1);
      gtk_list_store_remove (GTK_LIST_STORE (priv->store), &iter);
      
      file_path = g_build_filename (g_get_home_dir (),
                                    CODESLAYER_HOME,
                                    CODESLAYER_PROFILES_DIR,
                                    name,
                                    NULL);

      file = g_file_new_for_path (file_path);
      codeslayer_utils_file_delete (file, &error);
      
      if (error != NULL)
        {
          g_printerr ("not able to delete profile error: %s\n", error->message);
          g_error_free (error);
        }

      g_free (name);
      g_free (file_path);
      g_object_unref (file);
    }
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
      if (profile != NULL)
        {
          gtk_list_store_append (priv->store, &iter);
          gtk_list_store_set (priv->store, &iter, TEXT, name, -1);
        }

      list = g_list_next (list);
    }
    
  g_list_free_full (names, g_free);
}

static void
select_current_profile (CodeSlayerProfilesManager *profiles_manager)
{
  CodeSlayerProfilesManagerPrivate *priv;
  CodeSlayerProfile *profile;
  gchar *profile_name;
  GtkTreeIter iter;

  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  
  profile = codeslayer_profiles_get_current_profile  (priv->profiles);
  profile_name = codeslayer_profile_get_name (profile);
  
  gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->store), &iter, NULL);
  
  do
    {
      gchar *name;
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, TEXT, &name, -1);

      if (g_strcmp0 (profile_name, name) == 0)
        {
          GtkTreePath *path;
          path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->store), &iter);
          gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->tree), path, NULL, FALSE);
          gtk_tree_path_free (path);
        }

      g_free (name);
    }
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->store), &iter));

  g_free (profile_name);
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

