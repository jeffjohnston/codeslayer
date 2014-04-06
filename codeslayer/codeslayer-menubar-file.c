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
#include <codeslayer/codeslayer-menubar-file.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-menubar-file
 * @short_description: The file menu.
 * @title: CodeSlayerMenuBarFile
 * @include: codeslayer/codeslayer-menubar-file.h
 */

static void codeslayer_menu_bar_file_class_init  (CodeSlayerMenuBarFileClass *klass);
static void codeslayer_menu_bar_file_init        (CodeSlayerMenuBarFile      *menu_bar_file);
static void codeslayer_menu_bar_file_finalize    (CodeSlayerMenuBarFile      *menu_bar_file);

static void add_menu_items                       (CodeSlayerMenuBarFile      *menu_bar_file);
static void new_document_action                  (CodeSlayerMenuBarFile      *menu_bar_file);
static void open_document_action                 (CodeSlayerMenuBarFile      *menu_bar_file);
static void recent_document_action               (GtkMenuItem                *menuitem, 
                                                  CodeSlayerMenuBarFile      *menu_bar_file);
static void save_document_action                 (CodeSlayerMenuBarFile      *menu_bar_file);
static void save_all_documents_action            (CodeSlayerMenuBarFile      *menu_bar_file);
static void close_document_action                (CodeSlayerMenuBarFile      *menu_bar_file);
static void quit_application_action              (CodeSlayerMenuBarFile      *menu_bar_file);
static void show_profiles_action                 (CodeSlayerMenuBarFile      *menu_bar_file);
static void sync_menu_action                     (CodeSlayerMenuBarFile      *menu_bar_file,
                                                  gboolean                    enable_projects,
                                                  gboolean                    has_open_documents);
static void recent_documents_action              (CodeSlayerMenuBarFile      *menu_bar_file);

#define CODESLAYER_MENU_BAR_FILE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_MENU_BAR_FILE_TYPE, CodeSlayerMenuBarFilePrivate))

typedef struct _CodeSlayerMenuBarFilePrivate CodeSlayerMenuBarFilePrivate;

struct _CodeSlayerMenuBarFilePrivate
{
  CodeSlayerProfile *profile;
  GtkAccelGroup     *accel_group;
  GtkWidget         *menu_bar;
  GtkWidget         *menu;  
  GtkWidget         *new_item;
  GtkWidget         *open_item;
  GtkWidget         *save_item;
  GtkWidget         *save_all_item;
  GtkWidget         *save_separator_item;
  GtkWidget         *close_tab_item;
  GtkWidget         *recent_documents_item;
  GtkWidget         *recent_documents_separator_item;
};

G_DEFINE_TYPE (CodeSlayerMenuBarFile, codeslayer_menu_bar_file, GTK_TYPE_MENU_ITEM)

static void
codeslayer_menu_bar_file_class_init (CodeSlayerMenuBarFileClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_menu_bar_file_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerMenuBarFilePrivate));
}

static void
codeslayer_menu_bar_file_init (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  GtkWidget *menu;

  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu_bar_file), _("File"));
  
  menu = gtk_menu_new ();
  priv->menu = menu;
  priv->recent_documents_item = NULL;
  priv->recent_documents_separator_item = NULL;
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_bar_file), menu);
}

static void
codeslayer_menu_bar_file_finalize (CodeSlayerMenuBarFile *menu_bar_file)
{
  G_OBJECT_CLASS (codeslayer_menu_bar_file_parent_class)->finalize (G_OBJECT (menu_bar_file));
}

/**
 * codeslayer_menu_bar_file_new:
 * @menu_bar: a #CodeSlayerMenuBar.
 * @accel_group: a #GtkAccelGroup.
 * @profile: a #CodeSlayerProfile.
 *
 * Creates a new #CodeSlayerMenuBarFile.
 *
 * Returns: a new #CodeSlayerMenuBarFile. 
 */
GtkWidget*
codeslayer_menu_bar_file_new (GtkWidget         *menu_bar, 
                              GtkAccelGroup     *accel_group, 
                              CodeSlayerProfile *profile)
{
  CodeSlayerMenuBarFilePrivate *priv;
  GtkWidget *menu_bar_file;
  
  menu_bar_file = g_object_new (codeslayer_menu_bar_file_get_type (), NULL);
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);

  priv->menu_bar = menu_bar;
  priv->accel_group = accel_group;
  priv->profile = profile;

  add_menu_items (CODESLAYER_MENU_BAR_FILE (menu_bar_file));
  
  recent_documents_action (CODESLAYER_MENU_BAR_FILE (menu_bar_file));

  g_signal_connect_swapped (G_OBJECT (menu_bar), "sync-menu",
                            G_CALLBACK (sync_menu_action), menu_bar_file);

  g_signal_connect_swapped (G_OBJECT (profile), "recent-documents-changed",
                            G_CALLBACK (recent_documents_action), menu_bar_file);

  return menu_bar_file;
}

static void
add_menu_items (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  GtkWidget *new_item;
  GtkWidget *open_item;
  GtkWidget *save_item;
  GtkWidget *save_all_item;
  GtkWidget *save_separator_item;
  GtkWidget *close_tab_item;
  GtkWidget *profiles_item;
  GtkWidget *recent_documents_item;
  GtkWidget *quit_application_item;
  
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);

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
  
  profiles_item = gtk_menu_item_new_with_label (_("Profiles"));
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), profiles_item);

  priv->recent_documents_separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->recent_documents_separator_item);
  recent_documents_item = gtk_menu_item_new_with_label (_("Recent Documents"));
  priv->recent_documents_item = recent_documents_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), recent_documents_item);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (recent_documents_item), gtk_menu_new ());

  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), gtk_separator_menu_item_new ());
    
  close_tab_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, 
                                                            priv->accel_group);
  priv->close_tab_item = close_tab_item;
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), close_tab_item);

  quit_application_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, 
                                                                   priv->accel_group);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), quit_application_item);
  
  g_signal_connect_swapped (G_OBJECT (profiles_item), "activate",
                            G_CALLBACK (show_profiles_action), menu_bar_file);  

  g_signal_connect_swapped (G_OBJECT (new_item), "activate",
                            G_CALLBACK (new_document_action), menu_bar_file);
  
  g_signal_connect_swapped (G_OBJECT (open_item), "activate",
                            G_CALLBACK (open_document_action), menu_bar_file);
  
  g_signal_connect_swapped (G_OBJECT (save_item), "activate",
                            G_CALLBACK (save_document_action), menu_bar_file);
  
  g_signal_connect_swapped (G_OBJECT (save_all_item), "activate",
                            G_CALLBACK (save_all_documents_action), menu_bar_file);
  
  g_signal_connect_swapped (G_OBJECT (close_tab_item), "activate",
                            G_CALLBACK (close_document_action), menu_bar_file);
  
  g_signal_connect_swapped (G_OBJECT (quit_application_item), "activate",
                            G_CALLBACK (quit_application_action), menu_bar_file);
}

static void
sync_menu_action (CodeSlayerMenuBarFile *menu_bar_file,
                  gboolean               enable_projects,
                  gboolean               has_open_documents)
{
  CodeSlayerMenuBarFilePrivate *priv;
  
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);

  gtk_widget_set_sensitive (priv->save_item, has_open_documents);
  gtk_widget_set_sensitive (priv->save_all_item, has_open_documents);
  gtk_widget_set_sensitive (priv->close_tab_item, has_open_documents);

  if (enable_projects)
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
    
  if (codeslayer_profile_get_recent_documents (priv->profile) == NULL)
    {
      gtk_widget_hide (priv->recent_documents_separator_item);
      gtk_widget_hide (priv->recent_documents_item);
    }    
}

static void
recent_documents_action (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  GList *recent_documents;
  
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  
  recent_documents = codeslayer_profile_get_recent_documents (priv->profile);
  
  if (recent_documents != NULL)
    {
      GtkWidget *recent_documents_submenu;
      GList *children;
      GList *menu_items;
      
      recent_documents_submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (priv->recent_documents_item));
      
      children = gtk_container_get_children (GTK_CONTAINER (recent_documents_submenu));
      menu_items = children;
      while (menu_items != NULL)
        {
          GtkMenuItem *menu_item = menu_items->data;
          gtk_container_remove (GTK_CONTAINER (recent_documents_submenu), GTK_WIDGET (menu_item));
          menu_items = g_list_next (menu_items);
        }
      if (children != NULL)
        g_list_free (children);
      
      while (recent_documents != NULL)
        {
          gchar *recent_document = recent_documents->data;
          gchar *recent_document_basename;
          GtkWidget *recent_document_item;
          
          recent_document_basename = g_path_get_basename (recent_document);
          recent_document_item = gtk_menu_item_new_with_label (recent_document_basename);
          g_object_set_data (G_OBJECT (recent_document_item), "recent_document", recent_document);
          gtk_menu_shell_append (GTK_MENU_SHELL (recent_documents_submenu), recent_document_item);
          
          g_signal_connect (G_OBJECT (recent_document_item), "activate",
                            G_CALLBACK (recent_document_action), menu_bar_file);
          
          g_free (recent_document_basename);
          recent_documents = g_list_next (recent_documents);
        }  

      gtk_widget_show (priv->recent_documents_separator_item);
      gtk_widget_show_all (priv->recent_documents_item);
    }
}

static void
new_document_action (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  codeslayer_menu_bar_new_document (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
open_document_action (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  codeslayer_menu_bar_open_document (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
save_document_action (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  codeslayer_menu_bar_save_document (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
save_all_documents_action (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  codeslayer_menu_bar_save_all_documents (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
close_document_action (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  codeslayer_menu_bar_close_document (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
recent_document_action (GtkMenuItem           *menuitem, 
                        CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  gchar *recent_document;
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  recent_document = g_object_get_data (G_OBJECT (menuitem), "recent_document");
  codeslayer_menu_bar_recent_document (CODESLAYER_MENU_BAR (priv->menu_bar), recent_document);
}

static void
show_profiles_action (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  codeslayer_menu_bar_show_profiles (CODESLAYER_MENU_BAR (priv->menu_bar));
}

static void
quit_application_action (CodeSlayerMenuBarFile *menu_bar_file)
{
  CodeSlayerMenuBarFilePrivate *priv;
  priv = CODESLAYER_MENU_BAR_FILE_GET_PRIVATE (menu_bar_file);
  codeslayer_menu_bar_quit_application (CODESLAYER_MENU_BAR (priv->menu_bar));
}
