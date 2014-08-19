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

static void codeslayer_menu_bar_search_class_init  (CodeSlayerMenuBarSearchClass *klass);
static void codeslayer_menu_bar_search_init        (CodeSlayerMenuBarSearch      *menu_bar_search);
static void codeslayer_menu_bar_search_finalize    (CodeSlayerMenuBarSearch      *menu_bar_search);

static void add_menu_items                         (CodeSlayerMenuBarSearch      *menu_bar_search);

static void find_action                            (CodeSlayerMenuBarSearch      *menu_bar_search);
static void find_next_action                       (CodeSlayerMenuBarSearch      *menu_bar_search);
static void find_previous_action                   (CodeSlayerMenuBarSearch      *menu_bar_search);
static void replace_action                         (CodeSlayerMenuBarSearch      *menu_bar_search);
static void find_projects_action                   (CodeSlayerMenuBarSearch      *menu_bar_search);
static void search_for_document_action             (CodeSlayerMenuBarSearch      *menu_bar_search);
static void go_to_line_action                      (CodeSlayerMenuBarSearch      *menu_bar_search);
static void sync_menu_action                       (CodeSlayerMenuBarSearch      *menu_bar_search,
                                                    gboolean                      enable_projects,
                                                    gboolean                      has_open_documents);

#define CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_SEARCH_TYPE, CodeSlayerMenuBarSearchPrivate))

typedef struct _CodeSlayerMenuBarSearchPrivate CodeSlayerMenuBarSearchPrivate;

struct _CodeSlayerMenuBarSearchPrivate
{
  GtkAccelGroup     *accel_group;
  CodeSlayerProfile *profile;
  GtkWidget         *menu_bar;
  GtkWidget         *menu;  
  GtkWidget         *find_item;
  GtkWidget         *replace_item;
  GtkWidget         *find_next_item;
  GtkWidget         *find_previous_item;
  GtkWidget         *find_projects_item;
  GtkWidget         *search_for_document_item;
  GtkWidget         *find_projects_separator_item;
  GtkWidget         *go_to_line_item;
};

G_DEFINE_TYPE (CodeSlayerMenuBarSearch, codeslayer_menu_bar_search, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_bar_search_class_init (CodeSlayerMenuBarSearchClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_search_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarSearchPrivate));
}

static void
codeslayer_menu_bar_search_init (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu_bar_search), _("Search"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_search), menu);
}

static void
codeslayer_menu_bar_search_finalize (CodeSlayerMenuBarSearch *menu_bar_search)
{
  G_OBJECT_CLASS (codeslayer_menu_bar_search_parent_class)->finalize (G_OBJECT (menu_bar_search));
}

/**
 * codeslayer_menu_bar_search_new:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 * @profile: a #CodeSlayerProfile.
 *
 * Creates a new #CodeSlayerMenuBarSearch.
 *
 * Returns: a new #CodeSlayerMenuBarSearch. 
 */
GtkWidget*
codeslayer_menu_bar_search_new (GtkWidget         *menu_bar, 
                                GtkAccelGroup     *accel_group, 
                                CodeSlayerProfile *profile)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  GtkWidget *menu_bar_search;
  
  menu_bar_search = g_object_new (codeslayer_menu_bar_search_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);

  priv->menu_bar = menu_bar;
  priv->accel_group = accel_group;
  priv->profile = profile;

  add_menu_items (CODESLAYER_MENU_BAR_SEARCH (menu_bar_search));

  g_signal_connect_swapped (G_OBJECT (menu_bar), "sync-menu",
                            G_CALLBACK (sync_menu_action), menu_bar_search);

  return menu_bar_search;
}

static void
add_menu_items (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  GtkWidget *find_item;
  GtkWidget *find_next_item;
  GtkWidget *find_previous_item;
  GtkWidget *replace_item;
  GtkWidget *search_for_document_item;
  GtkWidget *find_projects_item;
  GtkWidget *find_projects_separator_item;
  GtkWidget *go_to_line_item;
  
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  
  find_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_FIND, priv->accel_group);
  priv->find_item = find_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_item);

  find_next_item = gtk_menu_item_new_with_label (_("Find Next"));
  priv->find_next_item = find_next_item;
  gtk_widget_add_accelerator (find_next_item, "activate", priv->accel_group,
                              GDK_KEY_G, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_next_item);

  find_previous_item = gtk_menu_item_new_with_label (_("Find Previous"));
  priv->find_previous_item = find_previous_item;
  gtk_widget_add_accelerator (find_previous_item,  "activate",
                              priv->accel_group, GDK_KEY_G,
                              GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_previous_item);

  replace_item = gtk_menu_item_new_with_label (_("Replace"));
  priv->replace_item = replace_item;
  gtk_widget_add_accelerator (replace_item, "activate", priv->accel_group,
                              GDK_KEY_H, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), replace_item);
  
  find_projects_separator_item = gtk_separator_menu_item_new ();
  priv->find_projects_separator_item = find_projects_separator_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_projects_separator_item);

  find_projects_item = gtk_menu_item_new_with_label (_("Find In Projects"));
  priv->find_projects_item = find_projects_item;
  gtk_widget_add_accelerator (find_projects_item, "activate",
                              priv->accel_group, GDK_KEY_F,
                              GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), find_projects_item);
  
  search_for_document_item = gtk_menu_item_new_with_label (_("Search For Document"));
  priv->search_for_document_item = search_for_document_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), search_for_document_item);

  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());

  go_to_line_item = gtk_menu_item_new_with_label (_("Go To Line"));
  priv->go_to_line_item = go_to_line_item;
  gtk_widget_add_accelerator (go_to_line_item, "activate", priv->accel_group,
                              GDK_KEY_J, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), go_to_line_item);

  g_signal_connect_swapped (G_OBJECT (find_item), "activate",
                            G_CALLBACK (find_action), menu_bar_search);
  
  g_signal_connect_swapped (G_OBJECT (replace_item), "activate",
                            G_CALLBACK (replace_action), menu_bar_search);
  
  g_signal_connect_swapped (G_OBJECT (find_next_item), "activate",
                            G_CALLBACK (find_next_action), menu_bar_search);
  
  g_signal_connect_swapped (G_OBJECT (find_previous_item), "activate",
                            G_CALLBACK (find_previous_action), menu_bar_search);
  
  g_signal_connect_swapped (G_OBJECT (find_projects_item), "activate",
                            G_CALLBACK (find_projects_action), menu_bar_search);
  
  g_signal_connect_swapped (G_OBJECT (search_for_document_item), "activate",
                            G_CALLBACK (search_for_document_action), menu_bar_search);
  
  g_signal_connect_swapped (G_OBJECT (go_to_line_item), "activate",
                            G_CALLBACK (go_to_line_action), menu_bar_search);
}

static void
sync_menu_action (CodeSlayerMenuBarSearch *menu_bar_search,
                    gboolean                 enable_projects,
                    gboolean                 has_open_documents)
{
  CodeSlayerMenuBarSearchPrivate *priv;

  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  
  gtk_widget_set_sensitive (priv->find_item, enable_projects || has_open_documents);  
  gtk_widget_set_sensitive (priv->find_next_item, has_open_documents);
  gtk_widget_set_sensitive (priv->find_previous_item, has_open_documents);
  gtk_widget_set_sensitive (priv->replace_item, has_open_documents);
  gtk_widget_set_sensitive (priv->go_to_line_item, has_open_documents);
  
  if (enable_projects)
    {
      gtk_widget_show (priv->find_projects_item);
      gtk_widget_show (priv->search_for_document_item);
      gtk_widget_show (priv->find_projects_separator_item);
    }
  else
    {
      gtk_widget_hide (priv->find_projects_item);
      gtk_widget_hide (priv->search_for_document_item);
      gtk_widget_hide (priv->find_projects_separator_item);
    }
}

static void
find_action (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  codeslayer_menu_bar_find (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
replace_action (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  codeslayer_menu_bar_replace (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
find_next_action (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  codeslayer_menu_bar_find_next (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
find_previous_action (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  codeslayer_menu_bar_find_previous (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
find_projects_action (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  codeslayer_menu_bar_find_projects (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
search_for_document_action (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  codeslayer_menu_bar_search_for_document (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
go_to_line_action (CodeSlayerMenuBarSearch *menu_bar_search)
{
  CodeSlayerMenuBarSearchPrivate *priv;
  priv = CODESLAYER_MENU_BAR_SEARCH_GET_PRIVATE (menu_bar_search);
  codeslayer_menu_bar_go_to_line (CODESLAYER_MENU_BAR (priv->menu_bar));
}
