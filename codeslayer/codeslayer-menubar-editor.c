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

static void codeslayer_menu_bar_editor_class_init  (CodeSlayerMenuBarEditorClass *klass);
static void codeslayer_menu_bar_editor_init        (CodeSlayerMenuBarEditor      *menu_bar_editor);
static void codeslayer_menu_bar_editor_finalize    (CodeSlayerMenuBarEditor      *menu_bar_editor);

static void add_menu_items                         (CodeSlayerMenuBarEditor      *menu_bar_editor);

static void new_editor_action                      (CodeSlayerMenuBarEditor      *menu_bar_editor);
static void open_editor_action                     (CodeSlayerMenuBarEditor      *menu_bar_editor);
static void save_editor_action                     (CodeSlayerMenuBarEditor      *menu_bar_editor);
static void save_all_editors_action                (CodeSlayerMenuBarEditor      *menu_bar_editor);
static void close_editor_action                    (CodeSlayerMenuBarEditor      *menu_bar_editor);
static void quit_application_action                (CodeSlayerMenuBarEditor      *menu_bar_editor);
static void show_preferences_action                (CodeSlayerMenuBarEditor      *menu_bar_editor);


#define CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_EDITOR_TYPE, CodeSlayerMenuBarEditorPrivate))

typedef struct _CodeSlayerMenuBarEditorPrivate CodeSlayerMenuBarEditorPrivate;

struct _CodeSlayerMenuBarEditorPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget     *menu_bar;
  GtkWidget     *menu;  
  GtkWidget     *new_item;
  GtkWidget     *open_item;
  GtkWidget     *save_item;
  GtkWidget     *save_all_item;
  GtkWidget     *save_separator_item;
  GtkWidget     *close_tab_item;
};

G_DEFINE_TYPE (CodeSlayerMenuBarEditor, codeslayer_menu_bar_editor, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_bar_editor_class_init (CodeSlayerMenuBarEditorClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_editor_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarEditorPrivate));
}

static void
codeslayer_menu_bar_editor_init (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu_bar_editor), _("Editor"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_editor), menu);
}

static void
codeslayer_menu_bar_editor_finalize (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  G_OBJECT_CLASS (codeslayer_menu_bar_editor_parent_class)->finalize (G_OBJECT (menu_bar_editor));
}

/**
 * codeslayer_menu_bar_editor_new:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 *
 * Creates a new #CodeSlayerMenuBarEditor.
 *
 * Returns: a new #CodeSlayerMenuBarEditor. 
 */
GtkWidget*
codeslayer_menu_bar_editor_new (GtkWidget     *menu_bar, 
                               GtkAccelGroup *accel_group)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  GtkWidget *menu_bar_editor;
  
  menu_bar_editor = g_object_new (codeslayer_menu_bar_editor_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);

  priv->menu_bar = menu_bar;
  priv->accel_group = accel_group;

  add_menu_items (CODESLAYER_MENU_BAR_EDITOR (menu_bar_editor));

  return menu_bar_editor;
}

static void
add_menu_items (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  GtkWidget *new_item;
  GtkWidget *open_item;
  GtkWidget *save_item;
  GtkWidget *save_all_item;
  GtkWidget *save_separator_item;
  GtkWidget *close_tab_item;
  GtkWidget *preferences_item;
  GtkWidget *quit_application_item;
  
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);

  new_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW, priv->accel_group);
  priv->new_item = new_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), new_item);

  open_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, priv->accel_group);
  priv->open_item = open_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), open_item);
  
  save_separator_item = gtk_separator_menu_item_new ();
  priv->save_separator_item = save_separator_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu),save_separator_item );

  save_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, priv->accel_group);
  priv->save_item = save_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), save_item);

  save_all_item = gtk_menu_item_new_with_label (_("Save All"));
  priv->save_all_item = save_all_item;
  gtk_widget_add_accelerator (save_all_item, "activate", priv->accel_group,
                              GDK_KEY_S, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), save_all_item);

  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());
  
  preferences_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES, 
                                                              priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), preferences_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());
    
  close_tab_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, 
                                                            priv->accel_group);
  priv->close_tab_item = close_tab_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), close_tab_item);

  quit_application_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, 
                                                                   priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), quit_application_item);
  
  g_signal_connect_swapped (G_OBJECT (preferences_item), "activate",
                            G_CALLBACK (show_preferences_action), menu_bar_editor);  

  g_signal_connect_swapped (G_OBJECT (new_item), "activate",
                            G_CALLBACK (new_editor_action), menu_bar_editor);
  
  g_signal_connect_swapped (G_OBJECT (open_item), "activate",
                            G_CALLBACK (open_editor_action), menu_bar_editor);
  
  g_signal_connect_swapped (G_OBJECT (save_item), "activate",
                            G_CALLBACK (save_editor_action), menu_bar_editor);
  
  g_signal_connect_swapped (G_OBJECT (save_all_item), "activate",
                            G_CALLBACK (save_all_editors_action), menu_bar_editor);
  
  g_signal_connect_swapped (G_OBJECT (close_tab_item), "activate",
                            G_CALLBACK (close_editor_action), menu_bar_editor);
  
  g_signal_connect_swapped (G_OBJECT (quit_application_item), "activate",
                            G_CALLBACK (quit_application_action), menu_bar_editor);
}

/**
 * codeslayer_menu_bar_editor_sync:
 * @menu_bar_editor: a #CodeSlayerMenuBarEditor.
 * @notebook: a #GtkNotebook.
 */
void
codeslayer_menu_bar_editor_sync (CodeSlayerMenuBarEditor *menu_bar_editor, 
                                 gboolean                 projects_mode,
                                 gboolean                 has_open_editors)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);

  gtk_widget_set_sensitive (priv->save_item, has_open_editors);
  gtk_widget_set_sensitive (priv->save_all_item, has_open_editors);
  gtk_widget_set_sensitive (priv->close_tab_item, has_open_editors);

  if (projects_mode)
    {
      gtk_widget_hide (priv->new_item);
      gtk_widget_hide (priv->open_item);
      gtk_widget_hide (priv->save_separator_item);
    }
  else
    {
      gtk_widget_show (priv->new_item);
      gtk_widget_show (priv->open_item);
      gtk_widget_show (priv->save_separator_item);
    }
}

static void
new_editor_action (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);
  codeslayer_menu_bar_new_editor (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
open_editor_action (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);
  codeslayer_menu_bar_open_editor (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
save_editor_action (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);
  codeslayer_menu_bar_save_editor (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
save_all_editors_action (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);
  codeslayer_menu_bar_save_all_editors (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
close_editor_action (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);
  codeslayer_menu_bar_close_editor (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
show_preferences_action (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);
  codeslayer_menu_bar_show_preferences (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
quit_application_action (CodeSlayerMenuBarEditor *menu_bar_editor)
{
  CodeSlayerMenuBarEditorPrivate *priv;
  priv = CODESLAYER_MENU_BAR_EDITOR_GET_PRIVATE (menu_bar_editor);
  codeslayer_menu_bar_quit_application (CODESLAYER_MENU_BAR (priv->menu_bar));
}
