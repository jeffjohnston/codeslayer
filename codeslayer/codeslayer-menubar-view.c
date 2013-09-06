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
#include <codeslayer/codeslayer-menubar-view.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-menubar-view
 * @short_description: The view menu.
 * @title: CodeSlayerMenuBarView
 * @include: codeslayer/codeslayer-menubar-view.h
 */

static void codeslayer_menu_bar_view_class_init  (CodeSlayerMenuBarViewClass *klass);
static void codeslayer_menu_bar_view_init        (CodeSlayerMenuBarView      *menu_bar_view);
static void codeslayer_menu_bar_view_finalize    (CodeSlayerMenuBarView      *menu_bar_view);

static void add_menu_items                       (CodeSlayerMenuBarView      *menu_bar_view, 
                                                  CodeSlayerSettings         *settings);

static void fullscreen_window_action             (CodeSlayerMenuBarView      *menu_bar_view);
static void show_side_pane_action                (CodeSlayerMenuBarView      *menu_bar_view);
static void show_bottom_pane_action              (CodeSlayerMenuBarView      *menu_bar_view);
static void draw_spaces_action                   (CodeSlayerMenuBarView      *menu_bar_view);
static void word_wrap_action                     (CodeSlayerMenuBarView      *menu_bar_view);

#define CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_VIEW_TYPE, CodeSlayerMenuBarViewPrivate))

typedef struct _CodeSlayerMenuBarViewPrivate CodeSlayerMenuBarViewPrivate;

struct _CodeSlayerMenuBarViewPrivate
{
  CodeSlayerSettings *settings;
  GtkAccelGroup      *accel_group;
  GtkWidget          *menu_bar;
  GtkWidget          *menu;
  GtkWidget          *fullscreen_window_item;
  GtkWidget          *show_side_pane_item;
  GtkWidget          *show_bottom_pane_item;
  GtkWidget          *draw_spaces_item;
  GtkWidget          *word_wrap_item;
  gulong              show_side_pane_id;
  gulong              show_bottom_pane_id;
};

G_DEFINE_TYPE (CodeSlayerMenuBarView, codeslayer_menu_bar_view, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_bar_view_class_init (CodeSlayerMenuBarViewClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_view_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarViewPrivate));
}

static void
codeslayer_menu_bar_view_init (CodeSlayerMenuBarView *menu_bar_view)
{
  CodeSlayerMenuBarViewPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu_bar_view), _("View"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_view), menu);
}

static void
codeslayer_menu_bar_view_finalize (CodeSlayerMenuBarView *menu_bar_view)
{
  G_OBJECT_CLASS (codeslayer_menu_bar_view_parent_class)->finalize (G_OBJECT (menu_bar_view));
}

/**
 * codeslayer_menu_bar_view_new:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 * @settings: a #CodeSlayerSettings.
 *
 * Creates a new #CodeSlayerMenuBarView.
 *
 * Returns: a new #CodeSlayerMenuBarView. 
 */
GtkWidget*
codeslayer_menu_bar_view_new (GtkWidget          *menu_bar, 
                              GtkAccelGroup      *accel_group, 
                              CodeSlayerSettings *settings)
{
  CodeSlayerMenuBarViewPrivate *priv;
  GtkWidget *menu_bar_view;
  
  menu_bar_view = g_object_new (codeslayer_menu_bar_view_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);

  priv->menu_bar = menu_bar;
  priv->accel_group = accel_group;
  priv->settings = settings;

  add_menu_items (CODESLAYER_MENU_BAR_VIEW (menu_bar_view), settings);

  return menu_bar_view;
}

static void
add_menu_items (CodeSlayerMenuBarView *menu_bar_view, 
                CodeSlayerSettings    *settings)
{
  CodeSlayerMenuBarViewPrivate *priv;
  GtkWidget *fullscreen_window_item;
  GtkWidget *show_side_pane_item;
  GtkWidget *show_bottom_pane_item;
  GtkWidget *draw_spaces_item;
  GtkWidget *word_wrap_item;
  
  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);

  fullscreen_window_item = gtk_check_menu_item_new_with_label (_("Full Screen"));
  priv->fullscreen_window_item = fullscreen_window_item;
  gtk_widget_add_accelerator (fullscreen_window_item, "activate", 
                              priv->accel_group, GDK_KEY_F11, 0, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), fullscreen_window_item);

  show_bottom_pane_item = gtk_check_menu_item_new_with_label (_("Bottom Pane"));
  priv->show_bottom_pane_item = show_bottom_pane_item;
  gtk_widget_add_accelerator (show_bottom_pane_item, "activate", 
                              priv->accel_group, GDK_KEY_F12, 0, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), show_bottom_pane_item);

  show_side_pane_item = gtk_check_menu_item_new_with_label (_("Side Pane"));
  priv->show_side_pane_item = show_side_pane_item;
  gtk_widget_add_accelerator (show_side_pane_item, "activate", 
                              priv->accel_group, GDK_KEY_F12, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), show_side_pane_item);
  
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());

  word_wrap_item = gtk_check_menu_item_new_with_label (_("Word Wrap"));
  priv->word_wrap_item = word_wrap_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), word_wrap_item);
  
  draw_spaces_item = gtk_check_menu_item_new_with_label (_("Draw Spaces"));
  priv->draw_spaces_item = draw_spaces_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), draw_spaces_item);
  
  g_signal_connect_swapped (G_OBJECT (fullscreen_window_item), "activate",
                            G_CALLBACK (fullscreen_window_action), menu_bar_view);

  priv->show_side_pane_id = g_signal_connect_swapped (G_OBJECT (show_side_pane_item), "activate",
                                                      G_CALLBACK (show_side_pane_action), menu_bar_view);

  priv->show_bottom_pane_id = g_signal_connect_swapped (G_OBJECT (show_bottom_pane_item), "activate",
                                                        G_CALLBACK (show_bottom_pane_action), menu_bar_view);

  g_signal_connect_swapped (G_OBJECT (word_wrap_item), "activate",
                            G_CALLBACK (word_wrap_action), menu_bar_view);

  g_signal_connect_swapped (G_OBJECT (draw_spaces_item), "activate",
                            G_CALLBACK (draw_spaces_action), menu_bar_view);
}

void
codeslayer_menu_bar_view_sync (CodeSlayerMenuBarView *menu_bar_view,
                               gboolean               has_open_editors)
{
  CodeSlayerMenuBarViewPrivate *priv;
  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);
  
  gtk_widget_set_sensitive (priv->draw_spaces_item, has_open_editors);
  gtk_widget_set_sensitive (priv->word_wrap_item, has_open_editors);
  
  g_signal_handler_block (priv->show_side_pane_item, priv->show_side_pane_id);
  g_signal_handler_block (priv->show_bottom_pane_item, priv->show_bottom_pane_id);
  
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (priv->show_side_pane_item),
                                  codeslayer_settings_get_boolean (priv->settings, 
                                                                   CODESLAYER_SETTINGS_SIDE_PANE_VISIBLE));

  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (priv->show_bottom_pane_item),
                                  codeslayer_settings_get_boolean (priv->settings, 
                                                                   CODESLAYER_SETTINGS_BOTTOM_PANE_VISIBLE));

  g_signal_handler_unblock (priv->show_side_pane_item, priv->show_side_pane_id);
  g_signal_handler_unblock (priv->show_bottom_pane_item, priv->show_bottom_pane_id);
  
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (priv->draw_spaces_item),
                                  codeslayer_settings_get_boolean (priv->settings, 
                                                                   CODESLAYER_SETTINGS_DRAW_SPACES));

  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (priv->word_wrap_item),
                                  codeslayer_settings_get_boolean (priv->settings, 
                                                                   CODESLAYER_SETTINGS_WORD_WRAP));
}                                             

static void
fullscreen_window_action (CodeSlayerMenuBarView *menu_bar_view)
{
  CodeSlayerMenuBarViewPrivate *priv;
  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);
  codeslayer_menu_bar_fullscreen_window (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
show_side_pane_action (CodeSlayerMenuBarView *menu_bar_view)
{
  CodeSlayerMenuBarViewPrivate *priv;
  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);
  codeslayer_menu_bar_show_side_pane (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
show_bottom_pane_action (CodeSlayerMenuBarView *menu_bar_view)
{
  CodeSlayerMenuBarViewPrivate *priv;
  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);
  codeslayer_menu_bar_show_bottom_pane (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
draw_spaces_action (CodeSlayerMenuBarView *menu_bar_view)
{
  CodeSlayerMenuBarViewPrivate *priv;
  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);
  codeslayer_menu_bar_draw_spaces (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
word_wrap_action (CodeSlayerMenuBarView *menu_bar_view)
{
  CodeSlayerMenuBarViewPrivate *priv;
  priv = CODESLAYER_MENU_BAR_VIEW_GET_PRIVATE (menu_bar_view);
  codeslayer_menu_bar_word_wrap (CODESLAYER_MENU_BAR (priv->menu_bar));
}
