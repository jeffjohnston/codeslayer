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

#ifndef __CODESLAYER_H__
#define	__CODESLAYER_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-projects-selection.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-project-properties.h>
#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-editor-linker.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-group.h>
#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-menuitem.h>
#include <codeslayer/codeslayer-completion.h>
#include <codeslayer/codeslayer-completion-provider.h>
#include <codeslayer/codeslayer-completion-proposal.h>
#include <codeslayer/codeslayer-processes.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-xml.h>

G_BEGIN_DECLS

#ifndef CODESLAYER_HOME
#define CODESLAYER_HOME ".codeslayer"
#endif

#define CODESLAYER_TYPE            (codeslayer_get_type ())
#define CODESLAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_TYPE, CodeSlayer))
#define CODESLAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_TYPE, CodeSlayerClass))
#define IS_CODESLAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_TYPE))
#define IS_CODESLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_TYPE))

typedef struct _CodeSlayer CodeSlayer;
typedef struct _CodeSlayerClass CodeSlayerClass;

struct _CodeSlayer
{
  GObject parent_instance;
};

struct _CodeSlayerClass
{
  GObjectClass parent_class;
  
  void (*editor_saved) (CodeSlayer *codeslayer);
  void (*editors_all_saved) (CodeSlayer *codeslayer);
  void (*editor_added) (CodeSlayer *codeslayer);
  void (*editor_removed) (CodeSlayer *codeslayer);
  void (*editor_switched) (CodeSlayer *codeslayer);
  void (*project_properties_opened) (CodeSlayer *codeslayer);
  void (*project_properties_saved) (CodeSlayer *codeslayer);
  void (*projects_changed) (CodeSlayer *codeslayer);
};

GType codeslayer_get_type (void) G_GNUC_CONST;
  
CodeSlayer*               codeslayer_new                             (GtkWindow                   *window,
                                                                      CodeSlayerPreferences       *preferences, 
                                                                      CodeSlayerProcesses         *processes, 
                                                                      CodeSlayerMenuBar           *menu_bar,
                                                                      CodeSlayerNotebook          *notebook,
                                                                      CodeSlayerProjects          *projects,
                                                                      CodeSlayerProjectProperties *project_properties,
                                                                      CodeSlayerSidePane          *side_pane,
                                                                      CodeSlayerBottomPane        *bottom_pane,
                                                                      CodeSlayerGroups            *groups);

gboolean                  codeslayer_select_editor                   (CodeSlayer                  *codeslayer, 
                                                                      CodeSlayerDocument          *document);

CodeSlayerEditor*         codeslayer_get_active_editor               (CodeSlayer                  *codeslayer);

const gchar*              codeslayer_get_active_editor_file_path     (CodeSlayer                  *codeslayer);

CodeSlayerProject*        codeslayer_get_active_editor_project       (CodeSlayer                  *codeslayer);

CodeSlayerDocument*       codeslayer_get_active_editor_document      (CodeSlayer                  *codeslayer);

GList*                    codeslayer_get_all_editors                 (CodeSlayer                  *codeslayer);


void                      codeslayer_add_to_side_pane                (CodeSlayer                  *codeslayer, 
                                                                      GtkWidget                   *widget,
                                                                      const gchar                 *title);

void                      codeslayer_remove_from_side_pane           (CodeSlayer                  *codeslayer, 
                                                                      GtkWidget                   *widget);

void                      codeslayer_show_side_pane                  (CodeSlayer                  *codeslayer, 
                                                                      GtkWidget                   *widget);

void                      codeslayer_add_to_bottom_pane              (CodeSlayer                  *codeslayer, 
                                                                      GtkWidget                   *widget,
                                                                      const gchar                 *title);

void                      codeslayer_remove_from_bottom_pane         (CodeSlayer                  *codeslayer, 
                                                                      GtkWidget                   *widget);

void                      codeslayer_show_bottom_pane                (CodeSlayer                  *codeslayer, 
                                                                      GtkWidget                   *widget);

void                      codeslayer_add_to_menubar                  (CodeSlayer                  *codeslayer, 
                                                                      GtkMenuItem                 *menuitem) __attribute__ ((deprecated));

void                      codeslayer_add_to_menu_bar                 (CodeSlayer                  *codeslayer, 
                                                                      GtkMenuItem                 *menuitem);

void                      codeslayer_remove_from_menubar             (CodeSlayer                  *codeslayer, 
                                                                      GtkMenuItem                 *menuitem) __attribute__ ((deprecated));
                                                        
void                      codeslayer_remove_from_menu_bar            (CodeSlayer                  *codeslayer, 
                                                                      GtkMenuItem                 *menuitem);
                                                        
void                      codeslayer_add_to_projects_popup           (CodeSlayer                  *codeslayer, 
                                                                      GtkMenuItem                 *menuitem);

void                      codeslayer_remove_from_projects_popup      (CodeSlayer                  *codeslayer, 
                                                                      GtkMenuItem                 *menuitem);
                                                        
void                      codeslayer_add_to_project_properties       (CodeSlayer                  *codeslayer, 
                                                                      GtkWidget                   *widget,
                                                                      const gchar                 *title);

void                      codeslayer_remove_from_project_properties  (CodeSlayer                  *codeslayer, 
                                                                      GtkWidget                   *widget);
                                                        
GtkAccelGroup*            codeslayer_get_menubar_accel_group         (CodeSlayer                  *codeslayer) __attribute__ ((deprecated));

GtkAccelGroup*            codeslayer_get_menu_bar_accel_group        (CodeSlayer                  *codeslayer);

CodeSlayerGroup*          codeslayer_get_group                       (CodeSlayer                  *codeslayer);

CodeSlayerGroup*          codeslayer_get_active_group                (CodeSlayer                  *codeslayer) __attribute__ ((deprecated));

gchar*                    codeslayer_get_active_group_folder_path    (CodeSlayer                  *codeslayer) __attribute__ ((deprecated));

gchar*                    codeslayer_get_configuration_folder_path   (CodeSlayer                  *codeslayer) __attribute__ ((deprecated));

gchar*                    codeslayer_get_project_config_folder_path  (CodeSlayer                  *codeslayer, 
                                                                      CodeSlayerProject           *project);

gchar*                    codeslayer_get_plugins_config_folder_path  (CodeSlayer                  *codeslayer);

gchar*                    codeslayer_get_group_config_folder_path    (CodeSlayer                  *codeslayer);

CodeSlayerProject*        codeslayer_get_project_by_file_path        (CodeSlayer                  *codeslayer, 
                                                                      const gchar                 *file_path);

CodeSlayerPreferences*    codeslayer_get_preferences                 (CodeSlayer                  *codeslayer);

GtkWindow*                codeslayer_get_toplevel_window             (CodeSlayer                  *codeslayer);
gint                      codeslayer_add_to_process_bar              (CodeSlayer                  *codeslayer,
                                                                      gchar                       *name,
                                                                      StopProcessFunc              func,
                                                                      gpointer                     data);
void                      codeslayer_remove_from_process_bar         (CodeSlayer                  *codeslayer,
                                                                      gint                         id);
CodeSlayerEditorLinker*   codeslayer_get_editor_linker               (CodeSlayer                  *codeslayer,
                                                                      GtkTextView                 *text_view);

G_END_DECLS

#endif /* __CODESLAYER_H__ */
