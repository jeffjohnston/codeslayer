/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __CODESLAYER_MENUBAR_H__
#define	__CODESLAYER_MENUBAR_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-group.h>
#include <codeslayer/codeslayer-preferences.h>

G_BEGIN_DECLS

#define CODESLAYER_MENUBAR_TYPE            (codeslayer_menubar_get_type ())
#define CODESLAYER_MENUBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_MENUBAR_TYPE, CodeSlayerMenuBar))
#define CODESLAYER_MENUBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_MENUBAR_TYPE, CodeSlayerMenuBarClass))
#define IS_CODESLAYER_MENUBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_MENUBAR_TYPE))
#define IS_CODESLAYER_MENUBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_MENUBAR_TYPE))

typedef struct _CodeSlayerMenuBar CodeSlayerMenuBar;
typedef struct _CodeSlayerMenuBarClass CodeSlayerMenuBarClass;

struct _CodeSlayerMenuBar
{
  GtkMenuBar parent_instance;
};

struct _CodeSlayerMenuBarClass
{
  GtkMenuBarClass parent_class;

  void (*group_changed) (CodeSlayerMenuBar *menubar);
  void (*new_group) (CodeSlayerMenuBar *menubar);
  void (*rename_group) (CodeSlayerMenuBar *menubar);
  void (*remove_group) (CodeSlayerMenuBar *menubar);
  void (*add_project) (CodeSlayerMenuBar *menubar);
  void (*find_projects) (CodeSlayerMenuBar *menubar);
  void (*fullscreen_window) (CodeSlayerMenuBar *menubar);
  void (*show_side_pane) (CodeSlayerMenuBar *menubar);
  void (*show_bottom_pane) (CodeSlayerMenuBar *menubar);
  void (*draw_spaces) (CodeSlayerMenuBar *menubar);
  void (*save_editor) (CodeSlayerMenuBar *menubar);
  void (*save_all_editors) (CodeSlayerMenuBar *menubar);
  void (*close_editor) (CodeSlayerMenuBar *menubar);
  void (*quit_application) (CodeSlayerMenuBar *menubar);
  void (*find) (CodeSlayerMenuBar *menubar);
  void (*replace) (CodeSlayerMenuBar *menubar);
  void (*find_next) (CodeSlayerMenuBar *menubar);
  void (*find_previous) (CodeSlayerMenuBar *menubar);
  void (*find_incremental) (CodeSlayerMenuBar *menubar);
  void (*change_preferences) (CodeSlayerMenuBar *menubar);
  void (*show_plugins) (CodeSlayerMenuBar *menubar);  
  void (*undo) (CodeSlayerMenuBar *menubar);
  void (*redo) (CodeSlayerMenuBar *menubar);
  void (*cut) (CodeSlayerMenuBar *menubar);
  void (*copy) (CodeSlayerMenuBar *menubar);
  void (*paste) (CodeSlayerMenuBar *menubar);
  void (*del) (CodeSlayerMenuBar *menubar);
  void (*select_all) (CodeSlayerMenuBar *menubar);
  void (*to_uppercase) (CodeSlayerMenuBar *menubar);
  void (*to_lowercase) (CodeSlayerMenuBar *menubar);
  void (*copy_lines) (CodeSlayerMenuBar *menubar);
  void (*sync_projects_with_editor) (CodeSlayerMenuBar *menubar);
};

GType codeslayer_menubar_get_type (void) G_GNUC_CONST;
  
GtkWidget*      codeslayer_menubar_new                        (GtkWidget              *window,
                                                               CodeSlayerGroups       *groups, 
                                                               CodeSlayerPreferences  *preferences);
                                                             
void            codeslayer_menubar_save_editor                (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_save_all_editors           (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_close_editor               (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_quit_application           (CodeSlayerMenuBar      *menubar);

void            codeslayer_menubar_undo                       (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_redo                       (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_cut                        (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_copy                       (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_paste                      (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_delete                     (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_select_all                 (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_to_uppercase               (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_to_lowercase               (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_copy_lines                 (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_show_preferences           (CodeSlayerMenuBar      *menubar);

void            codeslayer_menubar_find                       (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_replace                    (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_find_next                  (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_find_previous              (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_find_incremental           (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_find_projects              (CodeSlayerMenuBar      *menubar);

void            codeslayer_menubar_fullscreen_window          (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_show_side_pane             (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_show_bottom_pane           (CodeSlayerMenuBar      *menubar);
void            codeslayer_menubar_draw_spaces                (CodeSlayerMenuBar      *menubar);

void            codeslayer_menubar_refresh_groups             (CodeSlayerMenuBar      *menubar,
                                                               CodeSlayerGroups       *groups);
void            codeslayer_menubar_group_changed              (CodeSlayerMenuBar      *menubar, 
                                                               const gchar            *group_name);
void            codeslayer_menubar_new_group                  (CodeSlayerMenuBar      *menubar, 
                                                               const gchar            *group_name);
void            codeslayer_menubar_rename_group               (CodeSlayerMenuBar      *menubar, 
                                                               const gchar            *group_name);
void            codeslayer_menubar_remove_group               (CodeSlayerMenuBar      *menubar);
                                                          
void            codeslayer_menubar_sync_with_notebook         (CodeSlayerMenuBar      *menubar,
                                                               GtkWidget              *notebook);

void            codeslayer_menubar_sync_with_panes            (CodeSlayerMenuBar      *menubar, 
                                                               gboolean                show_side_pane, 
                                                               gboolean                show_bottom_pane);

void            codeslayer_menubar_add_tools_item             (CodeSlayerMenuBar      *menubar, 
                                                               GtkWidget              *item);
void            codeslayer_menubar_remove_tools_item          (CodeSlayerMenuBar      *menubar, 
                                                               GtkWidget              *item);
GtkAccelGroup*  codeslayer_menubar_get_accel_group            (CodeSlayerMenuBar      *menubar);

void            codeslayer_menubar_add_project                (CodeSlayerMenuBar      *menubar, 
                                                               GFile                  *file);
void            codeslayer_menubar_show_plugins               (CodeSlayerMenuBar      *menubar);

void            codeslayer_menubar_sync_projects_with_editor  (CodeSlayerMenuBar      *menubar, 
                                                               gboolean                sync_projects_with_editor);

G_END_DECLS

#endif /* __CODESLAYER_MENUBAR_H__ */
