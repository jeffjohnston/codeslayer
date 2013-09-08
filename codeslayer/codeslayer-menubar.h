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

#ifndef __CODESLAYER_MENU_BAR_H__
#define	__CODESLAYER_MENU_BAR_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-settings.h>
#include <codeslayer/codeslayer-config.h>

G_BEGIN_DECLS

#define CODESLAYER_MENU_BAR_TYPE            (codeslayer_menu_bar_get_type ())
#define CODESLAYER_MENU_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_MENU_BAR_TYPE, CodeSlayerMenuBar))
#define CODESLAYER_MENU_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_MENU_BAR_TYPE, CodeSlayerMenuBarClass))
#define IS_CODESLAYER_MENU_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_MENU_BAR_TYPE))
#define IS_CODESLAYER_MENU_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_MENU_BAR_TYPE))

typedef struct _CodeSlayerMenuBar CodeSlayerMenuBar;
typedef struct _CodeSlayerMenuBarClass CodeSlayerMenuBarClass;

struct _CodeSlayerMenuBar
{
  GtkMenuBar parent_instance;
};

struct _CodeSlayerMenuBarClass
{
  GtkMenuBarClass parent_class;

  void (*new_projects) (CodeSlayerMenuBar *menu_bar);
  void (*open_projects) (CodeSlayerMenuBar *menu_bar);
  void (*add_projects) (CodeSlayerMenuBar *menu_bar);
  void (*find_projects) (CodeSlayerMenuBar *menu_bar);
  void (*fullscreen_window) (CodeSlayerMenuBar *menu_bar);
  void (*show_side_pane) (CodeSlayerMenuBar *menu_bar);
  void (*show_bottom_pane) (CodeSlayerMenuBar *menu_bar);
  void (*draw_spaces) (CodeSlayerMenuBar *menu_bar);
  void (*word_wrap) (CodeSlayerMenuBar *menu_bar);
  void (*new_editor) (CodeSlayerMenuBar *menu_bar);
  void (*open_editor) (CodeSlayerMenuBar *menu_bar);
  void (*save_editor) (CodeSlayerMenuBar *menu_bar);
  void (*save_all_editors) (CodeSlayerMenuBar *menu_bar);
  void (*close_editor) (CodeSlayerMenuBar *menu_bar);
  void (*quit_application) (CodeSlayerMenuBar *menu_bar);
  void (*find) (CodeSlayerMenuBar *menu_bar);
  void (*replace) (CodeSlayerMenuBar *menu_bar);
  void (*find_next) (CodeSlayerMenuBar *menu_bar);
  void (*find_previous) (CodeSlayerMenuBar *menu_bar);
  void (*go_to_line) (CodeSlayerMenuBar *menu_bar);
  void (*show_preferences) (CodeSlayerMenuBar *menu_bar);
  void (*show_plugins) (CodeSlayerMenuBar *menu_bar);  
  void (*undo) (CodeSlayerMenuBar *menu_bar);
  void (*redo) (CodeSlayerMenuBar *menu_bar);
  void (*cut) (CodeSlayerMenuBar *menu_bar);
  void (*copy) (CodeSlayerMenuBar *menu_bar);
  void (*paste) (CodeSlayerMenuBar *menu_bar);
  void (*del) (CodeSlayerMenuBar *menu_bar);
  void (*select_all) (CodeSlayerMenuBar *menu_bar);
  void (*to_uppercase) (CodeSlayerMenuBar *menu_bar);
  void (*to_lowercase) (CodeSlayerMenuBar *menu_bar);
  void (*copy_lines) (CodeSlayerMenuBar *menu_bar);
  void (*sync_with_editor) (CodeSlayerMenuBar *menu_bar);
  void (*scan_external_changes) (CodeSlayerMenuBar *menu_bar);
  void (*sync_engine) (CodeSlayerMenuBar *menu_bar);
};

GType codeslayer_menu_bar_get_type (void) G_GNUC_CONST;
  
GtkWidget*      codeslayer_menu_bar_new                        (GtkWidget             *window, 
                                                                CodeSlayerPreferences *preferences, 
                                                                CodeSlayerSettings    *settings);
                                                             
void            codeslayer_menu_bar_new_editor                 (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_open_editor                (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_save_editor                (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_save_all_editors           (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_close_editor               (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_quit_application           (CodeSlayerMenuBar     *menu_bar);

void            codeslayer_menu_bar_undo                       (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_redo                       (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_cut                        (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_copy                       (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_paste                      (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_delete                     (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_select_all                 (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_to_uppercase               (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_to_lowercase               (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_copy_lines                 (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_show_preferences           (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_sync_with_editor           (CodeSlayerMenuBar     *menu_bar, 
                                                                gboolean               sync_with_editor);
void            codeslayer_menu_bar_scan_external_changes      (CodeSlayerMenuBar     *menu_bar);

void            codeslayer_menu_bar_find                       (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_replace                    (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_find_next                  (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_find_previous              (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_find_projects              (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_go_to_line                 (CodeSlayerMenuBar     *menu_bar);

void            codeslayer_menu_bar_fullscreen_window          (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_show_side_pane             (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_show_bottom_pane           (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_draw_spaces                (CodeSlayerMenuBar     *menu_bar);
void            codeslayer_menu_bar_word_wrap                  (CodeSlayerMenuBar     *menu_bar);

void            codeslayer_menu_bar_add_tools_item             (CodeSlayerMenuBar     *menu_bar, 
                                                                GtkWidget             *item);
void            codeslayer_menu_bar_remove_tools_item          (CodeSlayerMenuBar     *menu_bar, 
                                                                GtkWidget             *item);
GtkAccelGroup*  codeslayer_menu_bar_get_accel_group            (CodeSlayerMenuBar     *menu_bar);

void            codeslayer_menu_bar_open_projects              (CodeSlayerMenuBar     *menu_bar, 
                                                                GFile                 *file);
void            codeslayer_menu_bar_new_projects               (CodeSlayerMenuBar     *menu_bar, 
                                                                gchar                 *file_name);
void            codeslayer_menu_bar_add_projects               (CodeSlayerMenuBar     *menu_bar, 
                                                                GSList                *files);
void            codeslayer_menu_bar_show_plugins               (CodeSlayerMenuBar     *menu_bar);

G_END_DECLS

#endif /* __CODESLAYER_MENU_BAR_H__ */
