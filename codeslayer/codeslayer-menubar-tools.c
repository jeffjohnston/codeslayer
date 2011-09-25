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

static void codeslayer_menubar_tools_class_init  (CodeSlayerMenuBarToolsClass *klass);
static void codeslayer_menubar_tools_init        (CodeSlayerMenuBarTools      *menubar_tools);
static void codeslayer_menubar_tools_finalize    (CodeSlayerMenuBarTools      *menubar_tools);

static void add_menu_items                       (CodeSlayerMenuBarTools      *menubar_tools);
static void show_plugins_action                  (CodeSlayerMenuBarTools      *menubar_tools);
static void reorder_plugins                      (CodeSlayerMenuBarTools      *menubar_tools);

#define CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENUBAR_TOOLS_TYPE, CodeSlayerMenuBarToolsPrivate))

typedef struct _CodeSlayerMenuBarToolsPrivate CodeSlayerMenuBarToolsPrivate;

struct _CodeSlayerMenuBarToolsPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget     *menubar;
  GtkWidget     *menu;
  GtkWidget     *separator_item;
  GList         *plugins;
};

G_DEFINE_TYPE (CodeSlayerMenuBarTools, codeslayer_menubar_tools, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menubar_tools_class_init (CodeSlayerMenuBarToolsClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menubar_tools_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarToolsPrivate));
}

static void
codeslayer_menubar_tools_init (CodeSlayerMenuBarTools *menubar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE (menubar_tools);
  priv->plugins = NULL;
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menubar_tools), _("Tools"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menubar_tools), menu);
}

static void
codeslayer_menubar_tools_finalize (CodeSlayerMenuBarTools *menubar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  priv = CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE (menubar_tools);
  g_list_free (priv->plugins);
  G_OBJECT_CLASS (codeslayer_menubar_tools_parent_class)->finalize (G_OBJECT (menubar_tools));
}

/**
 * codeslayer_menubar_tools_new:
 * @menubar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 *
 * Creates a new #CodeSlayerMenuBarTools.
 *
 * Returns: a new #CodeSlayerMenuBarTools. 
 */
GtkWidget*
codeslayer_menubar_tools_new (GtkWidget     *menubar, 
                              GtkAccelGroup *accel_group)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GtkWidget *menubar_tools;
  
  menubar_tools = g_object_new (codeslayer_menubar_tools_get_type (), NULL);
  priv = CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE (menubar_tools);

  priv->menubar = menubar;
  priv->accel_group = accel_group;

  add_menu_items (CODESLAYER_MENUBAR_TOOLS (menubar_tools));

  return menubar_tools;
}

static void
add_menu_items (CodeSlayerMenuBarTools *menubar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GtkWidget *plugins_item;

  priv = CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE (menubar_tools);

  priv->separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->separator_item);

  gtk_menu_set_accel_group (GTK_MENU (priv->menu), priv->accel_group);
  
  plugins_item = gtk_menu_item_new_with_label (_("Plugins"));
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), plugins_item);
  
  gtk_widget_show_all (priv->menu);

  g_signal_connect_swapped (G_OBJECT (plugins_item), "activate",
                            G_CALLBACK (show_plugins_action), menubar_tools);
}

static void
show_plugins_action (CodeSlayerMenuBarTools *menubar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  priv = CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE (menubar_tools);
  codeslayer_menubar_show_plugins (CODESLAYER_MENUBAR (priv->menubar));
}

/**
 * codeslayer_menubar_tools_add_item:
 * @menubar_tools: a #CodeSlayerMenuBarTools.
 * @item: a #GtkWidget.
 */
void
codeslayer_menubar_tools_add_item (CodeSlayerMenuBarTools *menubar_tools,
                                   GtkWidget              *item)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GList *children;
  
  priv = CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE (menubar_tools);
  children = gtk_container_get_children (GTK_CONTAINER (priv->menu));
  gtk_menu_shell_insert (GTK_MENU_SHELL (priv->menu), item, g_list_length (children) - 2);  
  priv->plugins = g_list_append (priv->plugins, item);
  reorder_plugins (menubar_tools);
  gtk_widget_show_all (item);
  g_list_free (children);
}                              

/**
 * codeslayer_menubar_tools_remove_item:
 * @menubar_tools: a #CodeSlayerMenuBarTools.
 * @item: a #GtkWidget.
 */
void
codeslayer_menubar_tools_remove_item (CodeSlayerMenuBarTools *menubar_tools,
                                      GtkWidget              *item)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  priv = CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE (menubar_tools);
  gtk_container_remove (GTK_CONTAINER (priv->menu), item);
  priv->plugins = g_list_remove (priv->plugins, item);  
  reorder_plugins (menubar_tools);
}

static gint
compare_plugins (GtkMenuItem *item1, 
                 GtkMenuItem *item2)
{
  return g_strcmp0 (gtk_menu_item_get_label (item1),
                    gtk_menu_item_get_label (item2));
}

static void
reorder_plugins (CodeSlayerMenuBarTools *menubar_tools)
{
  CodeSlayerMenuBarToolsPrivate *priv;
  GList *tmp;
  int pos = 0;
  
  priv = CODESLAYER_MENUBAR_TOOLS_GET_PRIVATE (menubar_tools);
  
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
