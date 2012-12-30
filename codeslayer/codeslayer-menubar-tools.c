/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.remove_group_item
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-menubar-tools.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-menubar-tools
 * @short_description: The tools menu.
 * @title: CodeSlayerMenuBarTools
 * @include: codeslayer/codeslayer-menu-tools.h
 */

static void codeslayer_menu_bar_tools_class_init  (CodeSlayerMenuBarToolsClass *klass);
static void codeslayer_menu_bar_tools_init        (CodeSlayerMenuBarTools      *menu_bar_tools);
static void codeslayer_menu_bar_tools_finalize    (CodeSlayerMenuBarTools      *menu_bar_tools);

static void add_menu_items                        (CodeSlayerMenuBarTools      *menu_bar_tools);
static void show_plugins_action                   (CodeSlayerMenuBarTools      *menu_bar_tools);
static void reorder_plugins                       (CodeSlayerMenuBarTools      *menu_bar_tools);

#define CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_TOOLS_TYPE, CodeSlayerMenuBarToolsPrivate))

typedef struct _CodeSlayerMenuBarToolsPrivate CodeSlayerMenuBarToolsPrivate;

struct _CodeSlayerMenuBarToolsPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget     *menu_bar;
  GtkWidget     *menu;
  GtkWidget     *separator_item;
  GList         *plugins;
};

G_DEFINE_TYPE (CodeSlayerMenuBarTools, codeslayer_menu_bar_tools, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_bar_tools_class_init (CodeSlayerMenuBarToolsClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_tools_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarToolsPrivate));
}

static void
codeslayer_menu_bar_tools_init (CodeSlayerMenuBarTools *menu_bar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE (menu_bar_tools);
  priv->plugins = NULL;
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu_bar_tools), _("Tools"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_tools), menu);
}

static void
codeslayer_menu_bar_tools_finalize (CodeSlayerMenuBarTools *menu_bar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  priv = CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE (menu_bar_tools);
  g_list_free (priv->plugins);
  G_OBJECT_CLASS (codeslayer_menu_bar_tools_parent_class)->finalize (G_OBJECT (menu_bar_tools));
}

/**
 * codeslayer_menu_bar_tools_new:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 *
 * Creates a new #CodeSlayerMenuBarTools.
 *
 * Returns: a new #CodeSlayerMenuBarTools. 
 */
GtkWidget*
codeslayer_menu_bar_tools_new (GtkWidget     *menu_bar, 
                               GtkAccelGroup *accel_group)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GtkWidget *menu_bar_tools;
  
  menu_bar_tools = g_object_new (codeslayer_menu_bar_tools_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE (menu_bar_tools);

  priv->menu_bar = menu_bar;
  priv->accel_group = accel_group;

  add_menu_items (CODESLAYER_MENU_BAR_TOOLS (menu_bar_tools));

  return menu_bar_tools;
}

static void
add_menu_items (CodeSlayerMenuBarTools *menu_bar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GtkWidget *plugins_item;

  priv = CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE (menu_bar_tools);

  priv->separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->separator_item);

  gtk_menu_set_accel_group (GTK_MENU (priv->menu), priv->accel_group);
  
  plugins_item = gtk_menu_item_new_with_label (_("Plugins"));
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), plugins_item);
  
  gtk_widget_show_all (priv->menu);

  g_signal_connect_swapped (G_OBJECT (plugins_item), "activate",
                            G_CALLBACK (show_plugins_action), menu_bar_tools);
}

static void
show_plugins_action (CodeSlayerMenuBarTools *menu_bar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  priv = CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE (menu_bar_tools);
  codeslayer_menu_bar_show_plugins (CODESLAYER_MENU_BAR (priv->menu_bar));
}

/**
 * codeslayer_menu_bar_tools_add_item:
 * @menu_bar_tools: a #CodeSlayerMenuBarTools.
 * @item: a #GtkWidget.
 */
void
codeslayer_menu_bar_tools_add_item (CodeSlayerMenuBarTools *menu_bar_tools,
                                    GtkWidget              *item)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GList *children;
  
  priv = CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE (menu_bar_tools);
  children = gtk_container_get_children (GTK_CONTAINER (priv->menu));
  gtk_menu_shell_insert (GTK_MENU_SHELL (priv->menu), item, g_list_length (children) - 2);  
  priv->plugins = g_list_append (priv->plugins, item);
  reorder_plugins (menu_bar_tools);
  gtk_widget_show_all (item);
  g_list_free (children);
}                              

/**
 * codeslayer_menu_bar_tools_remove_item:
 * @menu_bar_tools: a #CodeSlayerMenuBarTools.
 * @item: a #GtkWidget.
 */
void
codeslayer_menu_bar_tools_remove_item (CodeSlayerMenuBarTools *menu_bar_tools,
                                       GtkWidget              *item)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  priv = CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE (menu_bar_tools);
  gtk_container_remove (GTK_CONTAINER (priv->menu), item);
  priv->plugins = g_list_remove (priv->plugins, item);  
  reorder_plugins (menu_bar_tools);
}

static gint
compare_plugins (GtkMenuItem *item1, 
                 GtkMenuItem *item2)
{
  return g_strcmp0 (gtk_menu_item_get_label (item1),
                    gtk_menu_item_get_label (item2));
}

static void
reorder_plugins (CodeSlayerMenuBarTools *menu_bar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GList *tmp;
  int pos = 0;
  
  priv = CODESLAYER_MENU_BAR_TOOLS_GET_PRIVATE (menu_bar_tools);
  
  priv->plugins = g_list_sort (priv->plugins, (GCompareFunc) compare_plugins);
  tmp = priv->plugins;

  while (tmp != NULL)
    {
      GtkWidget *item = tmp->data;
      gtk_menu_reorder_child (GTK_MENU (priv->menu), item, pos);
      pos++;
      tmp = g_list_next (tmp);
    }
}
