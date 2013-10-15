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
#include <codeslayer/codeslayer-menubar-edit.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-menubar-file
 * @short_description: The file menu.
 * @title: CodeSlayerMenuBarEdit
 * @include: codeslayer/codeslayer-menubar-file.h
 */

static void codeslayer_menu_bar_edit_class_init  (CodeSlayerMenuBarEditClass *klass);
static void codeslayer_menu_bar_edit_init        (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void codeslayer_menu_bar_edit_finalize    (CodeSlayerMenuBarEdit      *menu_bar_edit);

static void add_menu_items                       (CodeSlayerMenuBarEdit      *menu_bar_edit);

static void copy_lines_action                    (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void upper_case_action                     (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void lower_case_action                    (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void show_preferences_action              (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void sync_engine_action                   (CodeSlayerMenuBarEdit      *menu_bar_edit,
                                                  gboolean                    enable_projects,
                                                  gboolean                    has_open_editors);

#define CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_EDIT_TYPE, CodeSlayerMenuBarEditPrivate))

typedef struct _CodeSlayerMenuBarEditPrivate CodeSlayerMenuBarEditPrivate;

struct _CodeSlayerMenuBarEditPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget     *menu_bar;
  GtkWidget     *menu;  
  GtkWidget     *copy_lines_item;
  GtkWidget     *upper_case_item;
  GtkWidget     *lower_case_item;
};

G_DEFINE_TYPE (CodeSlayerMenuBarEdit, codeslayer_menu_bar_edit, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_bar_edit_class_init (CodeSlayerMenuBarEditClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_edit_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarEditPrivate));
}

static void
codeslayer_menu_bar_edit_init (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu_bar_edit), _("Edit"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_edit), menu);
}

static void
codeslayer_menu_bar_edit_finalize (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  G_OBJECT_CLASS (codeslayer_menu_bar_edit_parent_class)->finalize (G_OBJECT (menu_bar_edit));
}

/**
 * codeslayer_menu_bar_edit_new:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 *
 * Creates a new #CodeSlayerMenuBarEdit.
 *
 * Returns: a new #CodeSlayerMenuBarEdit. 
 */
GtkWidget*
codeslayer_menu_bar_edit_new (GtkWidget     *menu_bar, 
                              GtkAccelGroup *accel_group)
{
  CodeSlayerMenuBarEditPrivate *priv;
  GtkWidget *menu_bar_edit;
  
  menu_bar_edit = g_object_new (codeslayer_menu_bar_edit_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);

  priv->menu_bar = menu_bar;
  priv->accel_group = accel_group;

  add_menu_items (CODESLAYER_MENU_BAR_EDIT (menu_bar_edit));

  g_signal_connect_swapped (G_OBJECT (menu_bar), "sync-engine",
                            G_CALLBACK (sync_engine_action), menu_bar_edit);

  return menu_bar_edit;
}

static void
add_menu_items (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  GtkWidget *copy_lines_item;
  GtkWidget *upper_case_item;
  GtkWidget *lower_case_item;
  GtkWidget *preferences_item;
  
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);

  copy_lines_item = gtk_menu_item_new_with_label (_("Copy Lines"));
  priv->copy_lines_item = copy_lines_item;
  gtk_widget_add_accelerator (copy_lines_item, "activate", priv->accel_group,
                              GDK_KEY_Down, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), copy_lines_item);
  
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());

  upper_case_item = gtk_menu_item_new_with_label (_("Upper Case"));
  priv->upper_case_item = upper_case_item;
  gtk_widget_add_accelerator (upper_case_item, "activate", priv->accel_group,
                              GDK_KEY_U, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), upper_case_item);

  lower_case_item = gtk_menu_item_new_with_label (_("Lower Case"));
  priv->lower_case_item = lower_case_item;
  gtk_widget_add_accelerator (lower_case_item, "activate", priv->accel_group,
                              GDK_KEY_L, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), lower_case_item);

  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());
  
  preferences_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, 
                                                              priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), preferences_item);
  
  g_signal_connect_swapped (G_OBJECT (copy_lines_item), "activate",
                            G_CALLBACK (copy_lines_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (upper_case_item), "activate",
                            G_CALLBACK (upper_case_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (lower_case_item), "activate",
                            G_CALLBACK (lower_case_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (preferences_item), "activate",
                            G_CALLBACK (show_preferences_action), menu_bar_edit);  
}

static void
sync_engine_action (CodeSlayerMenuBarEdit *menu_bar_edit,
                    gboolean               enable_projects,
                    gboolean               has_open_editors)
{
}

static void
copy_lines_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_copy_lines (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
upper_case_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_to_uppercase (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
lower_case_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_to_lowercase (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
show_preferences_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_show_preferences (CODESLAYER_MENU_BAR (priv->menu_bar));
}
