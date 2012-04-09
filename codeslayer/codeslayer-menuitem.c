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

#include <codeslayer/codeslayer-menuitem.h>

/**
 * SECTION:codeslayer-menuitem
 * @short_description: The menu item.
 * @title: CodeSlayerMenuItem
 * @include: codeslayer/codeslayer-menuitem.h
 *
 * This is used for the projects popup menu. Classes that implement this should
 * bind to the projects-menu-selected signal.
 */

static void codeslayer_menu_item_class_init  (CodeSlayerMenuItemClass *klass);
static void codeslayer_menu_item_init        (CodeSlayerMenuItem      *menubar_help);
static void codeslayer_menu_item_finalize    (CodeSlayerMenuItem      *menubar_help);

enum
{
  PROJECTS_MENU_SELECTED,
  LAST_SIGNAL
};

static guint codeslayer_menu_item_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerMenuItem, codeslayer_menu_item, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_item_class_init (CodeSlayerMenuItemClass *klass)
{
  /**
   * CodeSlayerMenuItem::projects-menu-selected
   * @menuitem: the menuitem that received the signal
   * @selections: the #GList of #CodeSlayerProjectsSelection objects
   *
   * The ::projects-menu-selected signal is a request once the menu item is selected.
   */
  codeslayer_menu_item_signals[PROJECTS_MENU_SELECTED] =
    g_signal_new ("projects-menu-selected", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerMenuItemClass, projects_menu_selected), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_item_finalize;
}

static void
codeslayer_menu_item_init (CodeSlayerMenuItem *menuitem)
{
}

static void
codeslayer_menu_item_finalize (CodeSlayerMenuItem *menuitem)
{
  G_OBJECT_CLASS (codeslayer_menu_item_parent_class)->finalize (G_OBJECT (menuitem));
}

/**
 * codeslayer_menu_item_new:
 *
 * Creates a new #CodeSlayerMenuItem.
 *
 * Returns: a new #CodeSlayerMenuItem. 
 */
GtkWidget*
codeslayer_menu_item_new (void)
{
  GtkWidget *menuitem;
  menuitem = g_object_new (codeslayer_menu_item_get_type (), NULL);
  return menuitem;
}

/**
 * codeslayer_menu_item_new_with_label:
 * @label: the text for the menu item
 *
 * Creates a new #CodeSlayerMenuItem.
 *
 * Returns: a new #CodeSlayerMenuItem. 
 */
GtkWidget*  
codeslayer_menu_item_new_with_label  (const gchar *label)
{
  GtkWidget *menuitem;
  menuitem = g_object_new (codeslayer_menu_item_get_type (), NULL);
  gtk_menu_item_set_label (GTK_MENU_ITEM (menuitem), label);
  return menuitem;
}
