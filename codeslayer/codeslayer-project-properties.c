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

#include <codeslayer/codeslayer-project-properties.h>

/**
 * SECTION:codeslayer-project-properties
 * @short_description: The project properties.
 * @title: CodeSlayerProjectProperties
 * @include: codeslayer/codeslayer-project-properties.h
 */

static void codeslayer_project_properties_class_init  (CodeSlayerProjectPropertiesClass *klass);
static void codeslayer_project_properties_init        (CodeSlayerProjectProperties      *project_properties);
static void codeslayer_project_properties_finalize    (CodeSlayerProjectProperties      *project_properties);

static void reorder_plugins                           (CodeSlayerProjectProperties      *project_properties);

#define CODESLAYER_PROJECT_PROPERTIES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROJECT_PROPERTIES_TYPE, CodeSlayerProjectPropertiesPrivate))

typedef struct _CodeSlayerProjectPropertiesPrivate CodeSlayerProjectPropertiesPrivate;

struct _CodeSlayerProjectPropertiesPrivate
{
  GtkWidget *notebook;
  GList     *plugins;
};

G_DEFINE_TYPE (CodeSlayerProjectProperties, codeslayer_project_properties, GTK_TYPE_VBOX)

static void
codeslayer_project_properties_class_init (CodeSlayerProjectPropertiesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_project_properties_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerProjectPropertiesPrivate));
}

static void
codeslayer_project_properties_init (CodeSlayerProjectProperties *project_properties)
{
  CodeSlayerProjectPropertiesPrivate *priv;
  priv = CODESLAYER_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  gtk_box_set_homogeneous (GTK_BOX (project_properties), FALSE);
  gtk_box_set_spacing (GTK_BOX (project_properties), 0);
  priv->plugins = NULL;
}

static void
codeslayer_project_properties_finalize (CodeSlayerProjectProperties *project_properties)
{
  CodeSlayerProjectPropertiesPrivate *priv;
  priv = CODESLAYER_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  g_list_free (priv->plugins);
  G_OBJECT_CLASS (codeslayer_project_properties_parent_class)->finalize (G_OBJECT (project_properties));
}

/**
 * codeslayer_project_properties_new:
 *
 * Creates a new #CodeSlayerProjectProperties.
 *
 * Returns: a new #CodeSlayerProjectProperties. 
 */
GtkWidget*
codeslayer_project_properties_new ()
{
  CodeSlayerProjectPropertiesPrivate *priv;
  GtkWidget *project_properties;
  GtkWidget *notebook;
  
  project_properties = g_object_new (codeslayer_project_properties_get_type (), NULL);
  priv = CODESLAYER_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  priv->notebook = notebook;
  
  gtk_box_pack_start (GTK_BOX (project_properties), notebook, TRUE, TRUE, 1);  

  return project_properties;
}

/**
 * codeslayer_project_properties_add:
 * @project_properties: a #CodeSlayerProjectProperties.
 * @widget: a #GtkWidget.
 * @title: the title for the tab.
 *
 * Add the widget to the project properties.
 */
void
codeslayer_project_properties_add  (CodeSlayerProjectProperties *project_properties, 
                                    GtkWidget                   *widget, 
                                    const gchar                 *title)
{
  CodeSlayerProjectPropertiesPrivate *priv;
  priv = CODESLAYER_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 0)
    {
      gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook), widget, NULL);
      gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (priv->notebook), widget, title);
    }
  else
    {
      gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook), widget, NULL);
      gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (priv->notebook), widget, title);
      priv->plugins = g_list_append (priv->plugins, widget);
      reorder_plugins (project_properties);
    }

  gtk_widget_show_all (widget);
}

/**
 * codeslayer_project_properties_remove:
 * @project_properties: a #CodeSlayerProjectProperties.
 * @widget: a #GtkWidget.
 *
 * Remove the widget from the project properties.
 */
void
codeslayer_project_properties_remove (CodeSlayerProjectProperties *project_properties, 
                                      GtkWidget                   *widget)
{
  CodeSlayerProjectPropertiesPrivate *priv;
  gint page_num;
    
  priv = CODESLAYER_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);

  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (priv->notebook), widget);

  if (page_num == -1)
    g_critical ("The widget you are trying to remove from the project properties does not exist.");
  
  gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook), page_num);
  priv->plugins = g_list_remove (priv->plugins, widget);  
  reorder_plugins (project_properties);
}

static gint
compare_plugins (GtkWidget *item1, 
                 GtkWidget *item2, 
                 GtkWidget *notebook)
{
  return g_strcmp0 (gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (notebook), item1),
                    gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (notebook), item2));
}

static void
reorder_plugins (CodeSlayerProjectProperties *project_properties)
{
  CodeSlayerProjectPropertiesPrivate *priv;
  GList *tmp;
  int pos = 1;
  
  priv = CODESLAYER_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  priv->plugins = g_list_sort_with_data (priv->plugins, 
                                         (GCompareDataFunc) compare_plugins,
                                         priv->notebook);
  tmp = priv->plugins;

  while (tmp != NULL)
    {
      GtkWidget *item = tmp->data;
      gtk_notebook_reorder_child (GTK_NOTEBOOK (priv->notebook), item, pos);
      pos++;
      tmp = g_list_next (tmp);
    }
}
