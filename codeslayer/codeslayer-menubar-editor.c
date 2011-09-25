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
#include <codeslayer/codeslayer-menubar-editor.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-menubar-editor
 * @short_description: The editor menu.
 * @title: CodeSlayerMenuBarEditor
 * @include: codeslayer/codeslayer-menubar-editor.h
 */

static void codeslayer_menubar_editor_class_init  (CodeSlayerMenuBarEditorClass *klass);
static void codeslayer_menubar_editor_init        (CodeSlayerMenuBarEditor      *menubar_editor);
static void codeslayer_menubar_editor_finalize    (CodeSlayerMenuBarEditor      *menubar_editor);

static void add_menu_items                        (CodeSlayerMenuBarEditor      *menubar_editor);

static void save_editor_action                    (CodeSlayerMenuBarEditor      *menubar_editor);
static void save_all_editors_action               (CodeSlayerMenuBarEditor      *menubar_editor);
static void close_editor_action                   (CodeSlayerMenuBarEditor      *menubar_editor);
static void quit_application_action               (CodeSlayerMenuBarEditor      *menubar_editor);
static void show_preferences_action               (CodeSlayerMenuBarEditor      *menubar_editor);


#define CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENUBAR_EDITOR_TYPE, CodeSlayerMenuBarEditorPrivate))

typedef struct _CodeSlayerMenuBarEditorPrivate CodeSlayerMenuBarEditorPrivate;

struct _CodeSlayerMenuBarEditorPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget     *menubar;
  GtkWidget     *menu;  
  GtkWidget     *save_menu_item;
  GtkWidget     *save_all_menu_item;
  GtkWidget     *close_tab_menu_item;
};

G_DEFINE_TYPE (CodeSlayerMenuBarEditor, codeslayer_menubar_editor, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menubar_editor_class_init (CodeSlayerMenuBarEditorClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menubar_editor_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarEditorPrivate));
}

static void
codeslayer_menubar_editor_init (CodeSlayerMenuBarEditor *menubar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menubar_editor), _("Editor"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menubar_editor), menu);
}

static void
codeslayer_menubar_editor_finalize (CodeSlayerMenuBarEditor *menubar_editor)
{
  G_OBJECT_CLASS (codeslayer_menubar_editor_parent_class)->finalize (G_OBJECT (menubar_editor));
}

/**
 * codeslayer_menubar_editor_new:
 * @menubar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 *
 * Creates a new #CodeSlayerMenuBarEditor.
 *
 * Returns: a new #CodeSlayerMenuBarEditor. 
 */
GtkWidget*
codeslayer_menubar_editor_new (GtkWidget     *menubar, 
                               GtkAccelGroup *accel_group)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  GtkWidget *menubar_editor;
  
  menubar_editor = g_object_new (codeslayer_menubar_editor_get_type (), NULL);
  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);

  priv->menubar = menubar;
  priv->accel_group = accel_group;

  add_menu_items (CODESLAYER_MENUBAR_EDITOR (menubar_editor));

  return menubar_editor;
}

static void
add_menu_items (CodeSlayerMenuBarEditor *menubar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  GtkWidget *save_menu_item;
  GtkWidget *save_all_menu_item;
  GtkWidget *close_tab_menu_item;
  GtkWidget *preferences_menu_item;
  GtkWidget *quit_application_menu_item;
  
  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);

  save_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, 
                                                       priv->accel_group);
  priv->save_menu_item = save_menu_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), save_menu_item);

  save_all_menu_item = gtk_menu_item_new_with_label (_("Save All"));
  priv->save_all_menu_item = save_all_menu_item;
  gtk_widget_add_accelerator (save_all_menu_item, "activate", priv->accel_group,
                              GDK_S, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), save_all_menu_item);

  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());
  
  preferences_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, 
                                                              priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), preferences_menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());

  close_tab_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, 
                                                            priv->accel_group);
  priv->close_tab_menu_item = close_tab_menu_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), close_tab_menu_item);

  quit_application_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, 
                                                                   priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), quit_application_menu_item);
  
  g_signal_connect_swapped (G_OBJECT (preferences_menu_item), "activate",
                            G_CALLBACK (show_preferences_action), menubar_editor);  

  g_signal_connect_swapped (G_OBJECT (save_menu_item), "activate",
                            G_CALLBACK (save_editor_action), menubar_editor);
  
  g_signal_connect_swapped (G_OBJECT (save_all_menu_item), "activate",
                            G_CALLBACK (save_all_editors_action), menubar_editor);
  
  g_signal_connect_swapped (G_OBJECT (close_tab_menu_item), "activate",
                            G_CALLBACK (close_editor_action), menubar_editor);
  
  g_signal_connect_swapped (G_OBJECT (quit_application_menu_item), "activate",
                            G_CALLBACK (quit_application_action), menubar_editor);
}

/**
 * codeslayer_menubar_editor_sync_with_notebook:
 * @menubar_editor: a #CodeSlayerMenuBarEditor.
 * @notebook: a #GtkNotebook.
 */
void
codeslayer_menubar_editor_sync_with_notebook (CodeSlayerMenuBarEditor *menubar_editor,
                                              GtkWidget               *notebook)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  gboolean sensitive;
  gint pages;
  
  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);

  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
  sensitive = pages > 0;

  gtk_widget_set_sensitive (priv->save_menu_item, sensitive);
  gtk_widget_set_sensitive (priv->save_all_menu_item, sensitive);
  gtk_widget_set_sensitive (priv->close_tab_menu_item, sensitive);
}                                                         

static void
save_editor_action (CodeSlayerMenuBarEditor *menubar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);
  codeslayer_menubar_save_editor (CODESLAYER_MENUBAR (priv->menubar));
}

static void
save_all_editors_action (CodeSlayerMenuBarEditor *menubar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);
  codeslayer_menubar_save_all_editors (CODESLAYER_MENUBAR (priv->menubar));
}

static void
close_editor_action (CodeSlayerMenuBarEditor *menubar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);
  codeslayer_menubar_close_editor (CODESLAYER_MENUBAR (priv->menubar));
}

static void
show_preferences_action (CodeSlayerMenuBarEditor *menubar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);
  codeslayer_menubar_show_preferences (CODESLAYER_MENUBAR (priv->menubar));
}

static void
quit_application_action (CodeSlayerMenuBarEditor *menubar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENUBAR_EDITOR_GET_PRIVATE (menubar_editor);
  codeslayer_menubar_quit_application (CODESLAYER_MENUBAR (priv->menubar));
}
