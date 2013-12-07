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

#ifndef __CODESLAYER_PROJECT_H__
#define	__CODESLAYER_PROJECT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_PROJECT_TYPE            (codeslayer_project_get_type ())
#define CODESLAYER_PROJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROJECT_TYPE, CodeSlayerProject))
#define CODESLAYER_PROJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROJECT_TYPE, CodeSlayerProjectClass))
#define IS_CODESLAYER_PROJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROJECT_TYPE))
#define IS_CODESLAYER_PROJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROJECT_TYPE))

typedef struct _CodeSlayerProject CodeSlayerProject;
typedef struct _CodeSlayerProjectClass CodeSlayerProjectClass;

struct _CodeSlayerProject
{
  GObject parent_instance;
};

struct _CodeSlayerProjectClass
{
  GObjectClass parent_class;
};

GType codeslayer_project_get_type (void) G_GNUC_CONST;

CodeSlayerProject*  codeslayer_project_new             (void);
const gchar*       codeslayer_project_get_name         (CodeSlayerProject *project);
void               codeslayer_project_set_name         (CodeSlayerProject *project,
                                                        const gchar       *name);
const gchar*       codeslayer_project_get_folder_path  (CodeSlayerProject *project);
void               codeslayer_project_set_folder_path  (CodeSlayerProject *project,
                                                        const gchar       *folder_path);

G_END_DECLS

#endif /* __CODESLAYER_PROJECT_H__ */
