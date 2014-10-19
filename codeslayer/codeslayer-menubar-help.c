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
#include <codeslayer/codeslayer-menubar-help.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-menubar-help
 * @short_description: The help menu.
 * @title: CodeSlayerMenuBarHelp
 * @include: codeslayer/codeslayer-menubar-help.h
 */

static void codeslayer_menu_bar_help_class_init  (CodeSlayerMenuBarHelpClass *klass);
static void codeslayer_menu_bar_help_init        (CodeSlayerMenuBarHelp      *menu_bar_help);
static void codeslayer_menu_bar_help_finalize    (CodeSlayerMenuBarHelp      *menu_bar_help);

static void add_menu_items                       (CodeSlayerMenuBarHelp      *menu_bar_help);
static void about_action                         (CodeSlayerMenuBarHelp      *menu_bar_help);

#define CODESLAYER_MENU_BAR_HELP_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_HELP_TYPE, CodeSlayerMenuBarHelpPrivate))

typedef struct _CodeSlayerMenuBarHelpPrivate CodeSlayerMenuBarHelpPrivate;

struct _CodeSlayerMenuBarHelpPrivate
{
  GtkWidget     *window;
  GtkAccelGroup *accel_group;
  GtkWidget     *menu_bar;
  GtkWidget     *menu;
};

G_DEFINE_TYPE (CodeSlayerMenuBarHelp, codeslayer_menu_bar_help, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_bar_help_class_init (CodeSlayerMenuBarHelpClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_help_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarHelpPrivate));
}

static void
codeslayer_menu_bar_help_init (CodeSlayerMenuBarHelp *menu_bar_help)
{
  CodeSlayerMenuBarHelpPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENU_BAR_HELP_GET_PRIVATE (menu_bar_help);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu_bar_help), _("Help"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_help), menu);
}

static void
codeslayer_menu_bar_help_finalize (CodeSlayerMenuBarHelp *menu_bar_help)
{
  G_OBJECT_CLASS (codeslayer_menu_bar_help_parent_class)->finalize (G_OBJECT (menu_bar_help));
}

/**
 * codeslayer_menu_bar_help_new:
 * @window: a #GtkWindow.
 * @menu_bar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 *
 * Creates a new #CodeSlayerMenuBarHelp.
 *
 * Returns: a new #CodeSlayerMenuBarHelp. 
 */
GtkWidget*
codeslayer_menu_bar_help_new (GtkWidget     *window, 
                              GtkWidget     *menu_bar, 
                              GtkAccelGroup *accel_group)
{
  CodeSlayerMenuBarHelpPrivate *priv;
  GtkWidget *menu_bar_help;
  
  menu_bar_help = g_object_new (codeslayer_menu_bar_help_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_HELP_GET_PRIVATE (menu_bar_help);

  priv->window = window;
  priv->menu_bar = menu_bar;
  priv->accel_group = accel_group;

  add_menu_items (CODESLAYER_MENU_BAR_HELP (menu_bar_help));

  return menu_bar_help;
}

static void
add_menu_items (CodeSlayerMenuBarHelp *menu_bar_help)
{
  CodeSlayerMenuBarHelpPrivate *priv;
  GtkWidget *about_menu_item;

  priv = CODESLAYER_MENU_BAR_HELP_GET_PRIVATE (menu_bar_help);

  about_menu_item = gtk_menu_item_new_with_label (_("About"));
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), about_menu_item);

  g_signal_connect_swapped (G_OBJECT (about_menu_item), "activate",
                            G_CALLBACK (about_action), menu_bar_help);
}

static void
about_action (CodeSlayerMenuBarHelp *menu_bar_help)
{
  CodeSlayerMenuBarHelpPrivate *priv;
  priv = CODESLAYER_MENU_BAR_HELP_GET_PRIVATE (menu_bar_help);

  gtk_show_about_dialog (GTK_WINDOW (priv->window), 
                         "program-name", PACKAGE_NAME,
                         "version", PACKAGE_VERSION,
                         "copyright", "(C) 2010 Jeff Johnston",
                         "comments", "A lightweight code editor.",
                         "website", PACKAGE_URL,
                         NULL);
}
