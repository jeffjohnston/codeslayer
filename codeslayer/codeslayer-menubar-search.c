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

#include <gdk/gdkkeysyms.h>
#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-menubar-search.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-menubar-search
 * @short_description: The search menu.
 * @title: CodeSlayerMenuBarSearch
 * @include: codeslayer/codeslayer-menubar-search.h
 */

static void codeslayer_menubar_search_class_init  (CodeSlayerMenuBarSearchClass *klass);
static void codeslayer_menubar_search_init        (CodeSlayerMenuBarSearch      *menubar_search);
static void codeslayer_menubar_search_finalize    (CodeSlayerMenuBarSearch      *menubar_search);

static void add_menu_items                        (CodeSlayerMenuBarSearch      *menubar_search);

static void find_action                           (CodeSlayerMenuBarSearch      *menubar_search);
static void find_next_action                      (CodeSlayerMenuBarSearch      *menubar_search);
static void find_previous_action                  (CodeSlayerMenuBarSearch      *menubar_search);
static void find_incremental_action               (CodeSlayerMenuBarSearch      *menubar_search);
static void replace_action                        (CodeSlayerMenuBarSearch      *menubar_search);
static void find_projects_action                  (CodeSlayerMenuBarSearch      *menubar_search);

#define CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENUBAR_SEARCH_TYPE, CodeSlayerMenuBarSearchPrivate))

typedef struct _CodeSlayerMenuBarSearchPrivate CodeSlayerMenuBarSearchPrivate;

struct _CodeSlayerMenuBarSearchPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget     *menubar;
  GtkWidget     *menu;  
  GtkWidget     *find_menu_item;
  GtkWidget     *replace_menu_item;
  GtkWidget     *find_next_menu_item;
  GtkWidget     *find_previous_menu_item;
  GtkWidget     *find_incremental_menu_item;
  GtkWidget     *find_projects_menu_item;
};

G_DEFINE_TYPE (CodeSlayerMenuBarSearch, codeslayer_menubar_search, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menubar_search_class_init (CodeSlayerMenuBarSearchClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menubar_search_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarSearchPrivate));
}

static void
codeslayer_menubar_search_init (CodeSlayerMenuBarSearch *menubar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menubar_search), _("Search"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menubar_search), menu);
}

static void
codeslayer_menubar_search_finalize (CodeSlayerMenuBarSearch *menubar_search)
{
  G_OBJECT_CLASS (codeslayer_menubar_search_parent_class)->finalize (G_OBJECT (menubar_search));
}

/**
 * codeslayer_menubar_search_new:
 * @menubar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 *
 * Creates a new #CodeSlayerMenuBarSearch.
 *
 * Returns: a new #CodeSlayerMenuBarSearch. 
 */
GtkWidget*
codeslayer_menubar_search_new (GtkWidget     *menubar, 
                               GtkAccelGroup *accel_group)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  GtkWidget *menubar_search;
  
  menubar_search = g_object_new (codeslayer_menubar_search_get_type (), NULL);
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);

  priv->menubar = menubar;
  priv->accel_group = accel_group;

  add_menu_items (CODESLAYER_MENUBAR_SEARCH (menubar_search));

  return menubar_search;
}

static void
add_menu_items (CodeSlayerMenuBarSearch *menubar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  GtkWidget *find_menu_item;
  GtkWidget *find_next_menu_item;
  GtkWidget *find_previous_menu_item;
  GtkWidget *find_incremental_menu_item;
  GtkWidget *replace_menu_item;
  GtkWidget *find_projects_menu_item;
  
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  
  find_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_FIND, priv->accel_group);
  priv->find_menu_item = find_menu_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_menu_item);

  find_next_menu_item = gtk_menu_item_new_with_label (_("Find Next"));
  priv->find_next_menu_item = find_next_menu_item;
  gtk_widget_add_accelerator (find_next_menu_item, "activate", priv->accel_group,
                              GDK_G, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_next_menu_item);

  find_previous_menu_item = gtk_menu_item_new_with_label (_("Find Previous"));
  priv->find_previous_menu_item = find_previous_menu_item;
  gtk_widget_add_accelerator (find_previous_menu_item,  "activate",
                              priv->accel_group, GDK_G,
                              GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_previous_menu_item);

  find_incremental_menu_item = gtk_menu_item_new_with_label (_("Find Incremental"));
  priv->find_incremental_menu_item = find_incremental_menu_item;
  gtk_widget_add_accelerator (find_incremental_menu_item,  "activate",
                              priv->accel_group, GDK_K,
                              GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_incremental_menu_item);

  replace_menu_item = gtk_menu_item_new_with_label (_("Replace"));
  priv->replace_menu_item = replace_menu_item;
  gtk_widget_add_accelerator (replace_menu_item, "activate", priv->accel_group,
                              GDK_H, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), replace_menu_item);

  find_projects_menu_item = gtk_menu_item_new_with_label (_("Find In Projects"));
  priv->find_projects_menu_item = find_projects_menu_item;
  gtk_widget_add_accelerator (find_projects_menu_item, "activate",
                              priv->accel_group, GDK_F,
                              GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_projects_menu_item);

  g_signal_connect_swapped (G_OBJECT (find_menu_item), "activate",
                            G_CALLBACK (find_action), menubar_search);
  
  g_signal_connect_swapped (G_OBJECT (replace_menu_item), "activate",
                            G_CALLBACK (replace_action), menubar_search);
  
  g_signal_connect_swapped (G_OBJECT (find_next_menu_item), "activate",
                            G_CALLBACK (find_next_action), menubar_search);
  
  g_signal_connect_swapped (G_OBJECT (find_previous_menu_item), "activate",
                            G_CALLBACK (find_previous_action), menubar_search);
  
  g_signal_connect_swapped (G_OBJECT (find_incremental_menu_item), "activate",
                            G_CALLBACK (find_incremental_action), menubar_search);
  
  g_signal_connect_swapped (G_OBJECT (find_projects_menu_item), "activate",
                            G_CALLBACK (find_projects_action), menubar_search);
}

/**
 * codeslayer_menubar_search_sync_with_preferences:
 * @menubar_search: a #CodeSlayerMenuBarSearch.
 * @notebook: a #GtkNotebook.
 */
void  
codeslayer_menubar_search_sync_with_notebook (CodeSlayerMenuBarSearch *menubar_search,
                                              GtkWidget               *notebook)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  gboolean sensitive;
  gint pages;
  
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
  sensitive = pages > 0;
  
  gtk_widget_set_sensitive (priv->replace_menu_item, sensitive);
  gtk_widget_set_sensitive (priv->find_next_menu_item, sensitive);
  gtk_widget_set_sensitive (priv->find_previous_menu_item, sensitive);
  gtk_widget_set_sensitive (priv->find_incremental_menu_item, sensitive);
}                                              

static void
find_action (CodeSlayerMenuBarSearch *menubar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  codeslayer_menubar_find (CODESLAYER_MENUBAR (priv->menubar));
}

static void
replace_action (CodeSlayerMenuBarSearch *menubar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  codeslayer_menubar_replace (CODESLAYER_MENUBAR (priv->menubar));
}

static void
find_next_action (CodeSlayerMenuBarSearch *menubar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  codeslayer_menubar_find_next (CODESLAYER_MENUBAR (priv->menubar));
}

static void
find_previous_action (CodeSlayerMenuBarSearch *menubar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  codeslayer_menubar_find_previous (CODESLAYER_MENUBAR (priv->menubar));
}

static void
find_incremental_action (CodeSlayerMenuBarSearch *menubar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  codeslayer_menubar_find_incremental (CODESLAYER_MENUBAR (priv->menubar));
}

static void
find_projects_action (CodeSlayerMenuBarSearch *menubar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENUBAR_SEARCH_GET_PRIVATE (menubar_search);
  codeslayer_menubar_find_projects (CODESLAYER_MENUBAR (priv->menubar));
}
