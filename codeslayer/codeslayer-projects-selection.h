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

#ifndef __CODESLAYER_PROJECTS_SELECTION_H__
#define __CODESLAYER_PROJECTS_SELECTION_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-project.h>

G_BEGIN_DECLS

#define CODESLAYER_PROJECTS_SELECTION_TYPE            (codeslayer_projects_selection_get_type ())
#define CODESLAYER_PROJECTS_SELECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROJECTS_SELECTION_TYPE, CodeSlayerProjectsSelection))
#define CODESLAYER_PROJECTS_SELECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROJECTS_SELECTION_TYPE, CodeSlayerProjectsSelectionClass))
#define IS_CODESLAYER_PROJECTS_SELECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROJECTS_SELECTION_TYPE))
#define IS_CODESLAYER_PROJECTS_SELECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROJECTS_SELECTION_TYPE))

typedef struct _CodeSlayerProjectsSelection CodeSlayerProjectsSelection;
typedef struct _CodeSlayerProjectsSelectionClass CodeSlayerProjectsSelectionClass;

struct _CodeSlayerProjectsSelection
{
  GObject parent_instance;
};

struct _CodeSlayerProjectsSelectionClass
{
  GObjectClass parent_class;
};

GType codeslayer_projects_selection_get_type (void) G_GNUC_CONST;

CodeSlayerProjectsSelection*  codeslayer_projects_selection_new   (void);

const gchar*                  codeslayer_projects_selection_get_file_path  (CodeSlayerProjectsSelection *projects_selection);
void                          codeslayer_projects_selection_set_file_path  (CodeSlayerProjectsSelection *projects_selection, 
                                                                            const gchar                 *file_path);

CodeSlayerProject*            codeslayer_projects_selection_get_project    (CodeSlayerProjectsSelection *projects_selection);
void                          codeslayer_projects_selection_set_project    (CodeSlayerProjectsSelection *projects_selection, 
                                                                            CodeSlayerProject           *project);

G_END_DECLS

#endif /* __CODESLAYER_PROJECTS_SELECTION_H__ */
