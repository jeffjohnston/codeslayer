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

#ifndef __CODESLAYER_PROJECTS_H__
#define	__CODESLAYER_PROJECTS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-profile.h>
#include <codeslayer/codeslayer-profile-handler.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>

G_BEGIN_DECLS

#define CODESLAYER_PROJECTS_TYPE            (codeslayer_projects_get_type ())
#define CODESLAYER_PROJECTS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROJECTS_TYPE, CodeSlayerProjects))
#define CODESLAYER_PROJECTS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROJECTS_TYPE, CodeSlayerProjectsClass))
#define IS_CODESLAYER_PROJECTS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROJECTS_TYPE))
#define IS_CODESLAYER_PROJECTS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROJECTS_TYPE))

typedef struct _CodeSlayerProjects CodeSlayerProjects;
typedef struct _CodeSlayerProjectsClass CodeSlayerProjectsClass;

struct _CodeSlayerProjects
{
  GtkVBox parent_instance;
};

struct _CodeSlayerProjectsClass
{
  GtkVBoxClass parent_class;

  void (*select_document) (CodeSlayerProjects *projects);
  void (*remove_project) (CodeSlayerProjects *projects);
  void (*project_renamed) (CodeSlayerProjects *projects);
  void (*projects_changed) (CodeSlayerProjects *projects);
  void (*properties_opened) (CodeSlayerProjects *projects);
  void (*properties_saved) (CodeSlayerProjects *projects);

  /* private signals */
  void (*file_path_renamed) (CodeSlayerProjects *projects);
  void (*rename_file_folder) (CodeSlayerProjects *projects);
  void (*delete_file_folder) (CodeSlayerProjects *projects);
  void (*cut_file_folder) (CodeSlayerProjects *projects);
  void (*copy_file_folder) (CodeSlayerProjects *projects);
  void (*paste_file_folder) (CodeSlayerProjects *projects);
  void (*find_projects) (CodeSlayerProjects *projects);
  void (*search_find) (CodeSlayerProjects *projects);
};

GType codeslayer_projects_get_type (void) G_GNUC_CONST;

GtkWidget*  codeslayer_projects_new                (GtkWidget               *window, 
                                                    CodeSlayerProfileHandler *profile_handler,
                                                    CodeSlayerRegistry      *registry, 
                                                    GtkWidget               *project_properties);

void        codeslayer_projects_clear              (CodeSlayerProjects    *projects);
void        codeslayer_projects_add_project        (CodeSlayerProjects    *projects, 
                                                    CodeSlayerProject     *project);
gboolean    codeslayer_projects_select_document    (CodeSlayerProjects    *projects, 
                                                    CodeSlayerDocument    *document);
void        codeslayer_projects_refresh            (CodeSlayerProjects    *projects);
void        codeslayer_projects_search_find        (CodeSlayerProjects    *projects);
void        codeslayer_projects_add_popup_item     (CodeSlayerProjects    *projects,
                                                    GtkWidget             *item);
void        codeslayer_projects_remove_popup_item  (CodeSlayerProjects    *projects,
                                                    GtkWidget             *item);

G_END_DECLS

#endif /* _CODESLAYER_PROJECTS_H */
