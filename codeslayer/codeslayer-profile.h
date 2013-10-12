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

#ifndef __CODESLAYER_PROFILE_H__
#define	__CODESLAYER_PROFILE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-plugin.h>
#include <codeslayer/codeslayer-registry.h>

G_BEGIN_DECLS

#define CODESLAYER_PROFILE_TYPE            (codeslayer_profile_get_type ())
#define CODESLAYER_PROFILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROFILE_TYPE, CodeSlayerProfile))
#define CODESLAYER_PROFILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROFILE_TYPE, CodeSlayerProfileClass))
#define IS_CODESLAYER_PROFILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROFILE_TYPE))
#define IS_CODESLAYER_PROFILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROFILE_TYPE))

typedef struct _CodeSlayerProfile CodeSlayerProfile;
typedef struct _CodeSlayerProfileClass CodeSlayerProfileClass;

struct _CodeSlayerProfile
{
  GObject parent_instance;
};

struct _CodeSlayerProfileClass
{
  GObjectClass parent_class;
};

GType
codeslayer_profile_get_type (void) G_GNUC_CONST;

CodeSlayerProfile*   codeslayer_profile_new (void);

const gchar*         codeslayer_profile_get_file_path             (CodeSlayerProfile  *profile);
void                 codeslayer_profile_set_file_path             (CodeSlayerProfile  *profile, 
                                                                   const gchar        *file_path);
gboolean             codeslayer_profile_get_enable_projects       (CodeSlayerProfile  *profile);
void                 codeslayer_profile_set_enable_projects       (CodeSlayerProfile  *profile, 
                                                                   gboolean            enable_projects);
GList*               codeslayer_profile_get_projects              (CodeSlayerProfile  *profile);
void                 codeslayer_profile_set_projects              (CodeSlayerProfile  *profile,
                                                                   GList              *projects);
CodeSlayerProject*   codeslayer_profile_find_project              (CodeSlayerProfile  *profile, 
                                                                   const gchar        *project_key);
CodeSlayerProject*   codeslayer_profile_get_project_by_file_path  (CodeSlayerProfile  *profile, 
                                                                   const gchar        *file_path);                                                      
void                 codeslayer_profile_add_project               (CodeSlayerProfile  *profile,
                                                                  CodeSlayerProject  *project);
void                 codeslayer_profile_remove_project            (CodeSlayerProfile  *profile,
                                                                   CodeSlayerProject  *project);
gboolean             codeslayer_profile_contains_project          (CodeSlayerProfile  *profile,
                                                                   CodeSlayerProject  *project);                                                        
GList*               codeslayer_profile_get_documents             (CodeSlayerProfile  *profile);
void                 codeslayer_profile_set_documents             (CodeSlayerProfile  *profile,
                                                                   GList              *documents);
void                 codeslayer_profile_add_document              (CodeSlayerProfile  *profile,
                                                                   CodeSlayerDocument *document);
void                 codeslayer_profile_remove_document           (CodeSlayerProfile  *profile,
                                                                   CodeSlayerDocument *document);
CodeSlayerRegistry*  codeslayer_profile_get_registry              (CodeSlayerProfile  *profile);
GList*               codeslayer_profile_get_plugins               (CodeSlayerProfile  *profile);
void                 codeslayer_profile_set_plugins               (CodeSlayerProfile  *profile, 
                                                                   GList              *plugins);
gboolean             codeslayer_profile_contains_plugin           (CodeSlayerProfile  *profile, 
                                                                   const gchar        *plugin);
void                 codeslayer_profile_add_plugin                (CodeSlayerProfile  *profile, 
                                                                   const gchar        *plugin);
void                 codeslayer_profile_remove_plugin             (CodeSlayerProfile  *profile, 
                                                                   const gchar        *plugin);

G_END_DECLS

#endif /* __CODESLAYER_PROFILE_H__ */
