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

static void codeslayer_menubar_help_class_init  (CodeSlayerMenuBarHelpClass *klass);
static void codeslayer_menubar_help_init        (CodeSlayerMenuBarHelp      *menubar_help);
static void codeslayer_menubar_help_finalize    (CodeSlayerMenuBarHelp      *menubar_help);

static void add_menu_items                      (CodeSlayerMenuBarHelp      *menubar_help);
static void about_action                        (void);

#define CODESLAYER_MENUBAR_HELP_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENUBAR_HELP_TYPE, CodeSlayerMenuBarHelpPrivate))

typedef struct _CodeSlayerMenuBarHelpPrivate CodeSlayerMenuBarHelpPrivate;

struct _CodeSlayerMenuBarHelpPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget     *menubar;
  GtkWidget     *menu;
};

G_DEFINE_TYPE (CodeSlayerMenuBarHelp, codeslayer_menubar_help, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menubar_help_class_init (CodeSlayerMenuBarHelpClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menubar_help_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarHelpPrivate));
}

static void
codeslayer_menubar_help_init (CodeSlayerMenuBarHelp *menubar_help)
{
  CodeSlayerMenuBarHelpPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENUBAR_HELP_GET_PRIVATE (menubar_help);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menubar_help), _("Help"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menubar_help), menu);
}

static void
codeslayer_menubar_help_finalize (CodeSlayerMenuBarHelp *menubar_help)
{
  G_OBJECT_CLASS (codeslayer_menubar_help_parent_class)->finalize (G_OBJECT (menubar_help));
}

/**
 * codeslayer_menubar_help_new:
 * @menubar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 *
 * Creates a new #CodeSlayerMenuBarHelp.
 *
 * Returns: a new #CodeSlayerMenuBarHelp. 
 */
GtkWidget*
codeslayer_menubar_help_new (GtkWidget     *menubar, 
                             GtkAccelGroup *accel_group)
{
  CodeSlayerMenuBarHelpPrivate *priv;
  GtkWidget *menubar_help;
  
  menubar_help = g_object_new (codeslayer_menubar_help_get_type (), NULL);
  priv = CODESLAYER_MENUBAR_HELP_GET_PRIVATE (menubar_help);

  priv->menubar = menubar;
  priv->accel_group = accel_group;

  add_menu_items (CODESLAYER_MENUBAR_HELP (menubar_help));

  return menubar_help;
}

static void
add_menu_items (CodeSlayerMenuBarHelp *menubar_help)
{
  CodeSlayerMenuBarHelpPrivate *priv;
  GtkWidget *about_menu_item;

  priv = CODESLAYER_MENUBAR_HELP_GET_PRIVATE (menubar_help);

  about_menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, 
                                                        priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), about_menu_item);

  g_signal_connect_swapped (G_OBJECT (about_menu_item), "activate",
                            G_CALLBACK (about_action), NULL);
}

static void
about_action ()
{
  GtkWidget *dialog;
  dialog = gtk_about_dialog_new ();
  gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (dialog), PACKAGE_NAME);
  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), PACKAGE_VERSION);
  gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog), "(C) 2010 Jeff Johnston");
  gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (dialog), "A lightweight code editor.");
  gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog), PACKAGE_URL);
  gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (dialog), PACKAGE_URL);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (GTK_WIDGET (dialog));
}
