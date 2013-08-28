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

#ifndef __CODESLAYER_CONFIG_H__
#define	__CODESLAYER_CONFIG_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-plugin.h>

G_BEGIN_DECLS

#define CODESLAYER_CONFIG_TYPE            (codeslayer_config_get_type ())
#define CODESLAYER_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_CONFIG_TYPE, CodeSlayerConfig))
#define CODESLAYER_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_CONFIG_TYPE, CodeSlayerConfigClass))
#define IS_CODESLAYER_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_CONFIG_TYPE))
#define IS_CODESLAYER_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_CONFIG_TYPE))

typedef struct _CodeSlayerConfig CodeSlayerConfig;
typedef struct _CodeSlayerConfigClass CodeSlayerConfigClass;

struct _CodeSlayerConfig
{
  GObject parent_instance;
};

struct _CodeSlayerConfigClass
{
  GObjectClass parent_class;
};

GType
codeslayer_config_get_type (void) G_GNUC_CONST;

CodeSlayerConfig*    codeslayer_config_new (void);

const gchar*        codeslayer_config_get_file_path             (CodeSlayerConfig     *config);
void                codeslayer_config_set_file_path             (CodeSlayerConfig     *config, 
                                                                 const gchar          *file_path);
gboolean            codeslayer_config_get_projects_mode         (CodeSlayerConfig     *config);
void                codeslayer_config_set_projects_mode         (CodeSlayerConfig     *config, 
                                                                 gboolean              projects_mode);
GList*              codeslayer_config_get_projects              (CodeSlayerConfig     *config);
void                codeslayer_config_set_projects              (CodeSlayerConfig     *config,
                                                                 GList                *projects);
CodeSlayerProject*  codeslayer_config_find_project              (CodeSlayerConfig     *config, 
                                                                 const gchar          *project_key);
CodeSlayerProject*  codeslayer_config_get_project_by_file_path  (CodeSlayerConfig     *config, 
                                                                 const gchar          *file_path);                                                      
void                codeslayer_config_add_project               (CodeSlayerConfig     *config,
                                                                 CodeSlayerProject    *project);
void                codeslayer_config_remove_project            (CodeSlayerConfig     *config,
                                                                 CodeSlayerProject    *project);
gboolean            codeslayer_config_contains_project          (CodeSlayerConfig     *config,
                                                                 CodeSlayerProject    *project);                                                        
GList*              codeslayer_config_get_documents             (CodeSlayerConfig     *config);
void                codeslayer_config_set_documents             (CodeSlayerConfig     *config,
                                                                 GList                *documents);
void                codeslayer_config_add_document              (CodeSlayerConfig     *config,
                                                                 CodeSlayerDocument   *document);
void                codeslayer_config_remove_document           (CodeSlayerConfig     *config,
                                                                 CodeSlayerDocument   *document);
GHashTable*         codeslayer_config_get_preferences           (CodeSlayerConfig     *config);
const gchar*        codeslayer_config_get_preference            (CodeSlayerConfig     *config,
                                                                 gchar                *name);
void                codeslayer_config_set_preference            (CodeSlayerConfig     *config,
                                                                 gchar                *name, 
                                                                 gchar                *value);
GHashTable*         codeslayer_config_get_settings              (CodeSlayerConfig     *config);
const gchar*        codeslayer_config_get_setting               (CodeSlayerConfig     *config,
                                                                 gchar                *name);
void                codeslayer_config_set_setting               (CodeSlayerConfig     *config,
                                                                 gchar                *name, 
                                                                 gchar                *value);
GList*              codeslayer_config_get_plugins               (CodeSlayerConfig     *config);
void                codeslayer_config_set_plugins               (CodeSlayerConfig     *config, 
                                                                 GList                *plugins);
gboolean            codeslayer_config_contains_plugin           (CodeSlayerConfig     *config, 
                                                                 const gchar          *plugin);
void                codeslayer_config_add_plugin                (CodeSlayerConfig     *config, 
                                                                 const gchar          *plugin);
void                codeslayer_config_remove_plugin             (CodeSlayerConfig     *config, 
                                                                 const gchar          *plugin);

G_END_DECLS

#endif /* __CODESLAYER_CONFIG_H__ */
