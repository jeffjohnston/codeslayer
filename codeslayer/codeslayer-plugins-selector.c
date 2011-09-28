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

#include <codeslayer/codeslayer-plugins-selector.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-repository.h>

/**
 * SECTION:codeslayer-plugins-selector
 * @short_description: The tools plugins.
 * @title: CodeSlayerPluginsSelector
 * @include: codeslayer/codeslayer-plugins-selector.h
 */

static void codeslayer_plugins_selector_class_init  (CodeSlayerPluginsSelectorClass *klass);
static void codeslayer_plugins_selector_init        (CodeSlayerPluginsSelector      *plugins_selector);
static void codeslayer_plugins_selector_finalize    (CodeSlayerPluginsSelector      *plugins_selector);

static void plugin_enabled_action                   (CodeSlayerPluginsSelector      *plugins_selector,
                                                     gchar                          *path);
static void add_plugins_list                        (CodeSlayerPluginsSelector      *plugins_selector);
static void add_plugins_buttons                     (CodeSlayerPluginsSelector      *plugins_selector);
static void load_plugins                            (CodeSlayerPluginsSelector      *plugins_selector);
static void about_plugin_action                     (CodeSlayerPluginsSelector      *plugins_selector);
static void configure_plugin_action                 (CodeSlayerPluginsSelector      *plugins_selector);
static void select_row_action                       (GtkTreeSelection               *selection, 
                                                     CodeSlayerPluginsSelector      *plugins_selector);
static gint sort_compare                            (GtkTreeModel                   *model, 
                                                     GtkTreeIter                    *a,
                                                     GtkTreeIter                    *b, 
                                                     gpointer                        userdata);
                                                      

#define CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PLUGINS_SELECTOR_TYPE, CodeSlayerPluginsSelectorPrivate))

typedef struct _CodeSlayerPluginsSelectorPrivate CodeSlayerPluginsSelectorPrivate;

struct _CodeSlayerPluginsSelectorPrivate
{
  GtkWidget         *about_button;
  GtkWidget         *configure_button;
  GtkWidget         *tree;
  GtkListStore      *store;
  CodeSlayerPlugins *plugins;
  CodeSlayerGroup   *group;
};

enum
{
  ENABLED = 0,
  TEXT,
  PLUGIN,
  COLUMNS
};

G_DEFINE_TYPE (CodeSlayerPluginsSelector, codeslayer_plugins_selector, GTK_TYPE_VBOX)

static void
codeslayer_plugins_selector_class_init (CodeSlayerPluginsSelectorClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_plugins_selector_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPluginsSelectorPrivate));
}

static void
codeslayer_plugins_selector_init (CodeSlayerPluginsSelector *plugins_selector)
{
  gtk_container_set_border_width (GTK_CONTAINER (plugins_selector), 2);
  gtk_box_set_homogeneous (GTK_BOX (plugins_selector), FALSE);
  gtk_box_set_spacing (GTK_BOX (plugins_selector), 0);
  add_plugins_list (plugins_selector);
  add_plugins_buttons (plugins_selector);
}

static void
codeslayer_plugins_selector_finalize (CodeSlayerPluginsSelector *plugins_selector)
{
  G_OBJECT_CLASS (codeslayer_plugins_selector_parent_class)->finalize (G_OBJECT (plugins_selector));
}

GtkWidget*
codeslayer_plugins_selector_new (CodeSlayerPlugins *plugins, 
                                  CodeSlayerGroup   *group)
{
  CodeSlayerPluginsSelectorPrivate *priv;
  GtkWidget *plugins_selector;

  plugins_selector = g_object_new (codeslayer_plugins_selector_get_type (), 
                                      NULL);
  priv = CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE (plugins_selector);
  priv->plugins = plugins;
  priv->group = group;
  
  load_plugins (CODESLAYER_PLUGINS_SELECTOR (plugins_selector));
  
  return plugins_selector;
}

static void
load_plugins (CodeSlayerPluginsSelector *plugins_selector)
{
  CodeSlayerPluginsSelectorPrivate *priv;
  GList *list;
  
  priv = CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE (plugins_selector);

  list = codeslayer_plugins_get_list (priv->plugins);

  while (list != NULL)
    {
      CodeSlayerPlugin *plugin = list->data;
      GtkTreeIter iter;

      gtk_list_store_append (priv->store, &iter);
      gtk_list_store_set (priv->store, &iter,
                          ENABLED, codeslayer_plugin_get_enabled (plugin), 
                          TEXT, codeslayer_plugin_get_name (plugin), 
                          PLUGIN, plugin, -1);

      list = g_list_next (list);
    }
}

static void
add_plugins_list (CodeSlayerPluginsSelector *plugins_selector)
{
  CodeSlayerPluginsSelectorPrivate *priv;
  GtkListStore *store;
  GtkTreeSortable *sortable;
  GtkTreeSelection *selection;
  GtkWidget *tree;
  GtkTreeViewColumn *enabled_column;
  GtkTreeViewColumn *details_column;
  GtkCellRenderer *enabled_renderer;
  GtkCellRenderer *details_renderer;
  GtkWidget *scrolled_window;
  
  priv = CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE (plugins_selector);
  
  store = gtk_list_store_new (COLUMNS, G_TYPE_BOOLEAN, 
                              G_TYPE_STRING, G_TYPE_POINTER);
  priv->store = store;

  tree = gtk_tree_view_new ();
  priv->tree = tree;

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), GTK_TREE_MODEL (store));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
  
  sortable = GTK_TREE_SORTABLE (store);
  gtk_tree_sortable_set_sort_func (sortable, TEXT, sort_compare,
                                   GINT_TO_POINTER (TEXT), NULL);
  gtk_tree_sortable_set_sort_column_id (sortable, TEXT, GTK_SORT_ASCENDING);

  enabled_column = gtk_tree_view_column_new ();
  details_column = gtk_tree_view_column_new ();
  enabled_renderer = gtk_cell_renderer_toggle_new ();
  details_renderer = gtk_cell_renderer_text_new ();
  gtk_cell_renderer_toggle_set_activatable (GTK_CELL_RENDERER_TOGGLE(enabled_renderer), 
                                            TRUE);

  gtk_tree_view_column_pack_start (enabled_column, enabled_renderer, FALSE);
  gtk_tree_view_column_pack_start (details_column, details_renderer, FALSE);
  gtk_tree_view_column_set_attributes (enabled_column, enabled_renderer, 
                                       "active", ENABLED, NULL);
  gtk_tree_view_column_set_attributes (details_column, details_renderer, 
                                       "text", TEXT, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), enabled_column);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), details_column);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (tree));

  gtk_box_pack_start (GTK_BOX (plugins_selector),
                      GTK_WIDGET (scrolled_window), TRUE, TRUE, 0);
  
  g_signal_connect_swapped (G_OBJECT (enabled_renderer), "toggled",
                            G_CALLBACK (plugin_enabled_action), plugins_selector);
                            
  g_signal_connect (G_OBJECT (selection), "changed",
                    G_CALLBACK (select_row_action), plugins_selector);
}

static void
add_plugins_buttons (CodeSlayerPluginsSelector *plugins_selector)
{
  CodeSlayerPluginsSelectorPrivate *priv;
  GtkWidget *container;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *about_button;
  GtkWidget *configure_button;

  priv = CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE (plugins_selector);
  container = gtk_hbox_new (FALSE, 5);
  hbox = gtk_hbox_new (FALSE, 3);
  
  about_button = gtk_button_new_with_label (_("About Plugin"));
  priv->about_button = about_button;
  gtk_box_pack_start (GTK_BOX (hbox), about_button, FALSE, FALSE, 0);

  configure_button = gtk_button_new_with_label (_("Configure Plugin"));
  priv->configure_button = configure_button;
  gtk_widget_set_sensitive (configure_button, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), configure_button, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (container), label, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (container), hbox, FALSE, FALSE, 0);
  
  gtk_box_pack_start (GTK_BOX (plugins_selector), container, FALSE, FALSE, 5);
  
  g_signal_connect_swapped (G_OBJECT (about_button), "clicked",
                            G_CALLBACK (about_plugin_action), plugins_selector);

  g_signal_connect_swapped (G_OBJECT (configure_button), "clicked",
                            G_CALLBACK (configure_plugin_action), plugins_selector);
}

static void 
plugin_enabled_action (CodeSlayerPluginsSelector *plugins_selector,
                       gchar                      *path) 
{
  CodeSlayerPluginsSelectorPrivate *priv;
  GtkTreeIter iter;

  priv = CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE (plugins_selector);
  
  if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (priv->store), 
                                           &iter, path))
    {
      gboolean value;
      CodeSlayerPlugin *plugin;
      const gchar *lib;
      
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, 
                          ENABLED, &value,
                          PLUGIN, &plugin, -1);
                          
      codeslayer_plugin_set_enabled (plugin, !value);
      lib = codeslayer_plugin_get_lib (plugin);

      gtk_list_store_set (GTK_LIST_STORE (priv->store), &iter, 
                          ENABLED, !value, -1);
                          
      if (codeslayer_plugin_get_enabled (plugin))
        {
          codeslayer_group_add_lib (priv->group, lib);  
          codeslayer_plugin_activate (plugin);
          if (codeslayer_plugin_is_configurable (plugin))
            gtk_widget_set_sensitive (priv->configure_button, TRUE);
        }      
      else
        {
          codeslayer_group_remove_lib (priv->group, lib);
          codeslayer_plugin_deactivate (plugin);
          gtk_widget_set_sensitive (priv->configure_button, FALSE);    
        }
      
      codeslayer_repository_save_libs (priv->group);
    } 
}

static void
about_plugin_action (CodeSlayerPluginsSelector *plugins_selector)
{
  CodeSlayerPluginsSelectorPrivate *priv;
  GtkTreeSelection *treeselection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkWidget *dialog;
  
  priv = CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE (plugins_selector);
  dialog = gtk_about_dialog_new ();
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);

  treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(priv->tree));
  
  if (gtk_tree_selection_get_selected (treeselection, &model, &iter))
    {
      CodeSlayerPlugin *plugin;
      const gchar *name;    
      const gchar *description;    
      const gchar *version;    
      const gchar *authors;
      const gchar *website;
      
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter,
                          PLUGIN, &plugin, -1);
      
      name = codeslayer_plugin_get_name (plugin);
      version = codeslayer_plugin_get_version (plugin);
      description = codeslayer_plugin_get_description (plugin);
      authors = codeslayer_plugin_get_authors (plugin);
      website = codeslayer_plugin_get_website (plugin);
    
      gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (dialog), name);
      gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), version);
      gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (dialog), description);
      gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog), authors);
      gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog), website);
      gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (dialog), website);

      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (GTK_WIDGET (dialog));
    }   
}

static void
select_row_action (GtkTreeSelection          *selection, 
                   CodeSlayerPluginsSelector *plugins_selector)
{
  CodeSlayerPluginsSelectorPrivate *priv;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  priv = CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE (plugins_selector);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      CodeSlayerPlugin *plugin;
    
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter,
                          PLUGIN, &plugin, -1);
    
      if (codeslayer_plugin_get_enabled (plugin))
        {
          if (codeslayer_plugin_is_configurable (plugin))
            gtk_widget_set_sensitive (priv->configure_button, TRUE);
          else
            gtk_widget_set_sensitive (priv->configure_button, FALSE);
        }
      else
        {
          gtk_widget_set_sensitive (priv->configure_button, FALSE);
        }
    }
}

static void
configure_plugin_action (CodeSlayerPluginsSelector *plugins_selector)
{
  CodeSlayerPluginsSelectorPrivate *priv;
  GtkTreeSelection *treeselection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  priv = CODESLAYER_PLUGINS_SELECTOR_GET_PRIVATE (plugins_selector);
  treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(priv->tree));
  
  if (gtk_tree_selection_get_selected (treeselection, &model, &iter))
    {
      CodeSlayerPlugin *plugin;
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, 
                          PLUGIN, &plugin, -1);
      codeslayer_plugin_configure (plugin);
    }
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
