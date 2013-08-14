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

#ifndef __CODESLAYER_GROUP_H__
#define	__CODESLAYER_GROUP_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-preference.h>
#include <codeslayer/codeslayer-plugin.h>

G_BEGIN_DECLS

#define CODESLAYER_GROUP_TYPE            (codeslayer_group_get_type ())
#define CODESLAYER_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_GROUP_TYPE, CodeSlayerGroup))
#define CODESLAYER_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_GROUP_TYPE, CodeSlayerGroupClass))
#define IS_CODESLAYER_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_GROUP_TYPE))
#define IS_CODESLAYER_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_GROUP_TYPE))

typedef struct _CodeSlayerGroup CodeSlayerGroup;
typedef struct _CodeSlayerGroupClass CodeSlayerGroupClass;

struct _CodeSlayerGroup
{
  GObject parent_instance;
};

struct _CodeSlayerGroupClass
{
  GObjectClass parent_class;
};

GType
codeslayer_group_get_type (void) G_GNUC_CONST;

CodeSlayerGroup*    codeslayer_group_new (void);

const gchar*        codeslayer_group_get_name                  (CodeSlayerGroup   *group);
void                codeslayer_group_set_name                  (CodeSlayerGroup   *group, 
                                                                const gchar       *name);
GList*              codeslayer_group_get_projects              (CodeSlayerGroup   *group);
void                codeslayer_group_set_projects              (CodeSlayerGroup   *group,
                                                                GList             *projects);
CodeSlayerProject*  codeslayer_group_find_project              (CodeSlayerGroup   *group, 
                                                                const gchar       *project_key);
CodeSlayerProject*  codeslayer_group_get_project_by_file_path  (CodeSlayerGroup   *group, 
                                                                const gchar       *file_path);                                                      
void                codeslayer_group_add_project               (CodeSlayerGroup   *group,
                                                                CodeSlayerProject *project);
void                codeslayer_group_remove_project            (CodeSlayerGroup   *group,
                                                                CodeSlayerProject *project);
gboolean            codeslayer_group_contains_project          (CodeSlayerGroup   *group,
                                                                CodeSlayerProject *project);                                                        

GList*              codeslayer_group_get_documents             (CodeSlayerGroup   *group);
void                codeslayer_group_set_documents             (CodeSlayerGroup   *group,
                                                                GList             *documents);
void                codeslayer_group_add_document              (CodeSlayerGroup    *group,
                                                                CodeSlayerDocument *document);

GList*              codeslayer_group_get_preferences           (CodeSlayerGroup   *group);
void                codeslayer_group_set_preferences           (CodeSlayerGroup   *group,
                                                                GList             *preferences);
void                codeslayer_group_add_preference            (CodeSlayerGroup    *group,
                                                                CodeSlayerPreference *preference);

GList*              codeslayer_group_get_libs                  (CodeSlayerGroup   *group);
void                codeslayer_group_set_libs                  (CodeSlayerGroup   *group, 
                                                                GList             *libs);
gboolean            codeslayer_group_contains_lib              (CodeSlayerGroup   *group, 
                                                                const gchar       *lib);
void                codeslayer_group_add_lib                   (CodeSlayerGroup   *group, 
                                                                const gchar       *lib);
void                codeslayer_group_remove_lib                (CodeSlayerGroup   *group, 
                                                                const gchar       *lib);

G_END_DECLS

#endif /* __CODESLAYER_GROUP_H__ */
