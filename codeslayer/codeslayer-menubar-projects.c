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
#include <codeslayer/codeslayer-menubar-projects.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-menubar-projects
 * @short_description: The projects menu.
 * @title: CodeSlayerMenuBarProjects
 * @include: codeslayer/codeslayer-menu-projects.h
 */

static void codeslayer_menu_bar_projects_class_init  (CodeSlayerMenuBarProjectsClass *klass);
static void codeslayer_menu_bar_projects_init        (CodeSlayerMenuBarProjects      *menu_bar_projects);
static void codeslayer_menu_bar_projects_finalize    (CodeSlayerMenuBarProjects      *menu_bar_projects);

static void add_menu_items                           (CodeSlayerMenuBarProjects      *menu_bar_projects);
static void add_projects_action                      (CodeSlayerMenuBarProjects      *menu_bar_projects);
static void sync_with_editor_action                  (CodeSlayerMenuBarProjects      *menu_bar_projects);
static void scan_external_changes_action             (CodeSlayerMenuBarProjects      *menu_bar_projects);

#define CODESLAYER_MENU_BAR_PROJECTS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_PROJECTS_TYPE, CodeSlayerMenuBarProjectsPrivate))

typedef struct _CodeSlayerMenuBarProjectsPrivate CodeSlayerMenuBarProjectsPrivate;

struct _CodeSlayerMenuBarProjectsPrivate
{
  GtkWidget          *window;
  CodeSlayerSettings *settings;
  GtkAccelGroup      *accel_group;
  GtkWidget          *menu_bar;
  GtkWidget          *menu;
  GtkWidget          *sync_with_editor_item;
};

G_DEFINE_TYPE (CodeSlayerMenuBarProjects, codeslayer_menu_bar_projects, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_bar_projects_class_init (CodeSlayerMenuBarProjectsClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_projects_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarProjectsPrivate));
}

static void
codeslayer_menu_bar_projects_init (CodeSlayerMenuBarProjects *menu_bar_projects)
{
  CodeSlayerMenuBarProjectsPrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENU_BAR_PROJECTS_GET_PRIVATE (menu_bar_projects);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu_bar_projects), _("Projects"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_projects), menu);
}

static void
codeslayer_menu_bar_projects_finalize (CodeSlayerMenuBarProjects *menu_bar_projects)
{
  G_OBJECT_CLASS (codeslayer_menu_bar_projects_parent_class)->finalize (G_OBJECT (menu_bar_projects));
}

/**
 * codeslayer_menu_bar_projects_new:
 * @window: a #GtkWindow.
 * @menu_bar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 * @settings: a #CodeSlayerSettings.
 *
 * Creates a new #CodeSlayerMenuBarProjects.
 *
 * Returns: a new #CodeSlayerMenuBarProjects. 
 */
GtkWidget*
codeslayer_menu_bar_projects_new (GtkWidget          *window, 
                                  GtkWidget          *menu_bar, 
                                  GtkAccelGroup      *accel_group, 
                                  CodeSlayerSettings *settings)
{
  CodeSlayerMenuBarProjectsPrivate *priv;
  GtkWidget *menu_bar_projects;
  
  menu_bar_projects = g_object_new (codeslayer_menu_bar_projects_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_PROJECTS_GET_PRIVATE (menu_bar_projects);

  priv->window = window;
  priv->menu_bar = menu_bar;
  priv->accel_group = accel_group;
  priv->settings = settings;

  add_menu_items (CODESLAYER_MENU_BAR_PROJECTS (menu_bar_projects));

  return menu_bar_projects;
}

static void
add_menu_items (CodeSlayerMenuBarProjects *menu_bar_projects)
{
  CodeSlayerMenuBarProjectsPrivate *priv;
  GtkWidget *add_projects_item;
  GtkWidget *scan_external_changes_item;
  GtkWidget *sync_with_editor_item;
  gboolean sync_with_editor;
  
  priv = CODESLAYER_MENU_BAR_PROJECTS_GET_PRIVATE (menu_bar_projects);
  
  add_projects_item = gtk_menu_item_new_with_label (_("Add Projects"));
  gtk_menu_set_accel_group (GTK_MENU (priv->menu), priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), add_projects_item);

  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());

  sync_with_editor_item = gtk_check_menu_item_new_with_label (_("Sync With Editor"));
  priv->sync_with_editor_item = sync_with_editor_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), sync_with_editor_item);
  
  sync_with_editor = codeslayer_settings_get_boolean (priv->settings, 
                                                      CODESLAYER_SETTINGS_SYNC_WITH_EDITOR);
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (sync_with_editor_item), sync_with_editor);

  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());
  scan_external_changes_item = gtk_menu_item_new_with_label (_("Scan External Changes"));
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), scan_external_changes_item);

  g_signal_connect_swapped (G_OBJECT (add_projects_item), "activate",
                            G_CALLBACK (add_projects_action), menu_bar_projects);

  g_signal_connect_swapped (G_OBJECT (sync_with_editor_item), "activate",
                            G_CALLBACK (sync_with_editor_action), menu_bar_projects);

  g_signal_connect_swapped (G_OBJECT (scan_external_changes_item), "activate",
                            G_CALLBACK (scan_external_changes_action), menu_bar_projects);  
}

static void
add_projects_action (CodeSlayerMenuBarProjects *menu_bar_projects)
{
  CodeSlayerMenuBarProjectsPrivate *priv;
  GtkWidget *dialog;
  gint response;
  
  priv = CODESLAYER_MENU_BAR_PROJECTS_GET_PRIVATE (menu_bar_projects);
  
  dialog = gtk_file_chooser_dialog_new (_("Select Project"), 
                                        GTK_WINDOW (priv->window),
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_OK, 
                                        NULL);
                                        
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);;
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GSList *files = gtk_file_chooser_get_files (GTK_FILE_CHOOSER (dialog));
      codeslayer_menu_bar_add_projects (CODESLAYER_MENU_BAR (priv->menu_bar), files);
      g_slist_foreach (files, (GFunc) g_object_unref, NULL);
      g_slist_free (files);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
sync_with_editor_action (CodeSlayerMenuBarProjects *menu_bar_projects)
{
  CodeSlayerMenuBarProjectsPrivate *priv;
  gboolean sync_with_editor;
  
  priv = CODESLAYER_MENU_BAR_PROJECTS_GET_PRIVATE (menu_bar_projects);
  
  sync_with_editor = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (priv->sync_with_editor_item));
  codeslayer_menu_bar_sync_projects_with_editor (CODESLAYER_MENU_BAR (priv->menu_bar),
                                                sync_with_editor);
                                                
  codeslayer_settings_set_boolean (priv->settings, CODESLAYER_SETTINGS_SYNC_WITH_EDITOR,
                                   sync_with_editor);
  codeslayer_settings_save (priv->settings);
}

static void
scan_external_changes_action (CodeSlayerMenuBarProjects *menu_bar_projects)
{
  CodeSlayerMenuBarProjectsPrivate *priv;
  priv = CODESLAYER_MENU_BAR_PROJECTS_GET_PRIVATE (menu_bar_projects);
  codeslayer_menu_bar_scan_external_changes (CODESLAYER_MENU_BAR (priv->menu_bar));
}
