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

#include <glib/gstdio.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-plugins-selector.h>
#include <codeslayer/codeslayer-repository.h>
#include "codeslayer-utils.h"

/**
 * SECTION:codeslayer-plugins
 * @short_description: The plugins container.
 * @title: CodeSlayerPlugins
 * @include: codeslayer/codeslayer-plugins.h
 */

static void               codeslayer_plugins_class_init  (CodeSlayerPluginsClass *klass);
static void               codeslayer_plugins_init        (CodeSlayerPlugins      *plugins);
static void               codeslayer_plugins_finalize    (CodeSlayerPlugins      *plugins);

#define CODESLAYER_PLUGINS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PLUGINS_TYPE, CodeSlayerPluginsPrivate))

typedef struct _CodeSlayerPluginsPrivate CodeSlayerPluginsPrivate;

struct _CodeSlayerPluginsPrivate
{
  GtkWidget *window;
  GList *list;
};

G_DEFINE_TYPE (CodeSlayerPlugins, codeslayer_plugins, G_TYPE_OBJECT)

static void 
codeslayer_plugins_class_init (CodeSlayerPluginsClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_plugins_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPluginsPrivate));
}

static void
codeslayer_plugins_init (CodeSlayerPlugins *plugins)
{
  CodeSlayerPluginsPrivate *priv;
  priv = CODESLAYER_PLUGINS_GET_PRIVATE (plugins);
  priv->list = NULL;
}

static void
codeslayer_plugins_finalize (CodeSlayerPlugins *plugins)
{
  CodeSlayerPluginsPrivate *priv;
  priv = CODESLAYER_PLUGINS_GET_PRIVATE (plugins);

  if (priv->list != NULL)
    {
      g_list_foreach (priv->list, (GFunc) g_object_unref, NULL);
      g_list_free (priv->list);
    }

  G_OBJECT_CLASS (codeslayer_plugins_parent_class)->finalize (G_OBJECT (plugins));
}

/**
 * codeslayer_plugins_new:
 *
 * Creates a new #CodeSlayerPlugins.
 *
 * Returns: a new #CodeSlayerPlugins. 
 */
CodeSlayerPlugins*
codeslayer_plugins_new (GtkWidget *window)
{
  CodeSlayerPluginsPrivate *priv;
  CodeSlayerPlugins *plugins;

  plugins = g_object_new (codeslayer_plugins_get_type (), NULL);
  priv = CODESLAYER_PLUGINS_GET_PRIVATE (plugins);
  priv->window = window;
  
  return plugins;
}

/**
 * codeslayer_plugins_load:
 * @plugins: a #CodeSlayerPlugins.
 * @data: a #CodeSlayer.
 */
void
codeslayer_plugins_load (CodeSlayerPlugins *plugins, 
                         GObject           *data)
{
  CodeSlayerPluginsPrivate *priv;
  GList *list;                    
  priv = CODESLAYER_PLUGINS_GET_PRIVATE (plugins);
  list = codeslayer_repository_get_plugins (data);
  priv->list = list;
}

/**
 * codeslayer_plugins_activate:
 * @plugins: a #CodeSlayerPlugins.
 * @group: a #CodeSlayerGroup.
 */
void 
codeslayer_plugins_activate (CodeSlayerPlugins *plugins, 
                             CodeSlayerGroup   *group)
{
  CodeSlayerPluginsPrivate *priv;
  GList *list;
 
  priv = CODESLAYER_PLUGINS_GET_PRIVATE (plugins);

  list = priv->list;

  while (list != NULL)
    {
      CodeSlayerPlugin *plugin = list->data;
      const gchar *lib;
      
      lib = codeslayer_plugin_get_lib (plugin);
      
      if (codeslayer_group_contains_lib (group, lib))
        {
          if (!codeslayer_plugin_get_enabled (plugin))
            {
              codeslayer_plugin_set_enabled (plugin, TRUE);
              codeslayer_plugin_activate (plugin);
            }
        }
      else
        {
          if (codeslayer_plugin_get_enabled (plugin))
            {
              codeslayer_plugin_set_enabled (plugin, FALSE);
              codeslayer_plugin_deactivate (plugin);
            }
        }
      
      list = g_list_next (list);
    }
}

/**
 * codeslayer_plugins_deactivate:
 * @plugins: a #CodeSlayerPlugins.
 */
void 
codeslayer_plugins_deactivate (CodeSlayerPlugins *plugins)
{
  CodeSlayerPluginsPrivate *priv;
  GList *list;
 
  priv = CODESLAYER_PLUGINS_GET_PRIVATE (plugins);

  list = priv->list;

  while (list != NULL)
    {
      CodeSlayerPlugin *plugin = list->data;
      if (codeslayer_plugin_get_enabled (plugin))
        codeslayer_plugin_deactivate (plugin);
      list = g_list_next (list);
    }
}

/**
 * codeslayer_plugins_get_list:
 * @plugins: a #CodeSlayerPlugins.
 *
 * Returns: The list of #CodeSlayerPlugin objects.
 */
GList*
codeslayer_plugins_get_list (CodeSlayerPlugins *plugins)
{
  return CODESLAYER_PLUGINS_GET_PRIVATE (plugins)->list;
}

/**
 * codeslayer_plugins_run_dialog:
 * @plugins: a #CodeSlayerPlugins.
 * @group: a #CodeSlayerGroup.
 *
 * Show the plugins dialog.
 */
void
codeslayer_plugins_run_dialog (CodeSlayerPlugins *plugins, 
                               CodeSlayerGroup   *group)
{
  CodeSlayerPluginsPrivate *priv;
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *plugins_selector;
  
  priv = CODESLAYER_PLUGINS_GET_PRIVATE (plugins);

  dialog = gtk_dialog_new_with_buttons (_("Plugins"), 
                                        GTK_WINDOW (priv->window),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                        NULL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  gtk_widget_set_size_request (content_area, 350, 400);

  plugins_selector = codeslayer_plugins_selector_new (plugins, group);
  gtk_box_pack_start (GTK_BOX (content_area), plugins_selector, TRUE, TRUE, 0);  

  gtk_widget_show_all (content_area);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}
