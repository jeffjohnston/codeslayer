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
 * SECTION:codeslayer-menubar-edit
 * @short_description: The edit menu.
 * @title: CodeSlayerMenuBarEdit
 * @include: codeslayer/codeslayer-menubar-edit.h
 */

static void codeslayer_menu_bar_edit_class_init  (CodeSlayerMenuBarEditClass *klass);
static void codeslayer_menu_bar_edit_init        (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void codeslayer_menu_bar_edit_finalize    (CodeSlayerMenuBarEdit      *menu_bar_edit);

static void add_menu_items                       (CodeSlayerMenuBarEdit      *menu_bar_edit);

static void cut_action                           (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void copy_action                          (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void paste_action                         (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void copy_lines_action                    (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void to_uppercase_action                  (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void to_lowercase_action                  (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void show_preferences_action              (CodeSlayerMenuBarEdit      *menu_bar_edit);
static void sync_menu_action                     (CodeSlayerMenuBarEdit      *menu_bar_edit,
                                                  gboolean                    enable_projects,
                                                  gboolean                    has_open_documents);

#define CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_EDIT_TYPE, CodeSlayerMenuBarEditPrivate))

typedef struct _CodeSlayerMenuBarEditPrivate CodeSlayerMenuBarEditPrivate;

struct _CodeSlayerMenuBarEditPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget     *menu_bar;
  GtkWidget     *menu;  
  GtkWidget     *cut_item;
  GtkWidget     *copy_item;
  GtkWidget     *paste_item;
  GtkWidget     *copy_lines_item;
  GtkWidget     *to_uppercase_item;
  GtkWidget     *to_lowercase_item;
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

  g_signal_connect_swapped (G_OBJECT (menu_bar), "sync-menu",
                            G_CALLBACK (sync_menu_action), menu_bar_edit);

  return menu_bar_edit;
}

static void
add_menu_items (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  GtkWidget *cut_item;
  GtkWidget *copy_item;
  GtkWidget *paste_item;
  GtkWidget *copy_lines_item;
  GtkWidget *to_uppercase_item;
  GtkWidget *to_lowercase_item;
  GtkWidget *preferences_item;
  
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  
  cut_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CUT, priv->accel_group);
  priv->cut_item = cut_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), cut_item);
  
  copy_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_COPY, priv->accel_group);
  priv->copy_item = copy_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), copy_item);
  
  paste_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PASTE, priv->accel_group);
  priv->paste_item = paste_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), paste_item);
  
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());

  copy_lines_item = gtk_menu_item_new_with_label (_("Copy Lines"));
  priv->copy_lines_item = copy_lines_item;
  gtk_widget_add_accelerator (copy_lines_item, "activate", priv->accel_group,
                              GDK_KEY_Down, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), copy_lines_item);
  
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());

  to_lowercase_item = gtk_menu_item_new_with_label (_("Lowercase"));
  priv->to_lowercase_item = to_lowercase_item;
  gtk_widget_add_accelerator (to_lowercase_item, "activate", priv->accel_group,
                              GDK_KEY_L, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), to_lowercase_item);

  to_uppercase_item = gtk_menu_item_new_with_label (_("Uppercase"));
  priv->to_uppercase_item = to_uppercase_item;
  gtk_widget_add_accelerator (to_uppercase_item, "activate", priv->accel_group,
                              GDK_KEY_U, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), to_uppercase_item);

  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());
  
  preferences_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, 
                                                              priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), preferences_item);
  
  g_signal_connect_swapped (G_OBJECT (cut_item), "activate",
                            G_CALLBACK (cut_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (copy_item), "activate",
                            G_CALLBACK (copy_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (paste_item), "activate",
                            G_CALLBACK (paste_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (copy_lines_item), "activate",
                            G_CALLBACK (copy_lines_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (to_uppercase_item), "activate",
                            G_CALLBACK (to_uppercase_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (to_lowercase_item), "activate",
                            G_CALLBACK (to_lowercase_action), menu_bar_edit);
  
  g_signal_connect_swapped (G_OBJECT (preferences_item), "activate",
                            G_CALLBACK (show_preferences_action), menu_bar_edit);  
}

static void
sync_menu_action (CodeSlayerMenuBarEdit *menu_bar_edit,
                  gboolean               enable_projects,
                  gboolean               has_open_documents)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);

  gtk_widget_set_sensitive (priv->cut_item, enable_projects || has_open_documents);
  gtk_widget_set_sensitive (priv->copy_item, enable_projects || has_open_documents);
  gtk_widget_set_sensitive (priv->paste_item, enable_projects || has_open_documents);
  gtk_widget_set_sensitive (priv->copy_lines_item, has_open_documents);
  gtk_widget_set_sensitive (priv->to_lowercase_item, has_open_documents);
  gtk_widget_set_sensitive (priv->to_uppercase_item, has_open_documents);
}

static void
cut_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_cut (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
copy_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_copy (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
paste_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_paste (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
copy_lines_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_copy_lines (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
to_uppercase_action (CodeSlayerMenuBarEdit *menu_bar_edit)
{
  CodeSlayerMenuBarEditPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDIT_GET_PRIVATE (menu_bar_edit);
  codeslayer_menu_bar_to_uppercase (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
to_lowercase_action (CodeSlayerMenuBarEdit *menu_bar_edit)
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
