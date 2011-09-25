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

#ifndef __CODESLAYER_PLUGIN_H__
#define __CODESLAYER_PLUGIN_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_PLUGIN_TYPE            (codeslayer_plugin_get_type ())
#define CODESLAYER_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PLUGIN_TYPE, CodeSlayerPlugin))
#define CODESLAYER_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PLUGIN_TYPE, CodeSlayerPluginClass))
#define IS_CODESLAYER_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PLUGIN_TYPE))
#define IS_CODESLAYER_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PLUGIN_TYPE))

typedef struct _CodeSlayerPlugin CodeSlayerPlugin;
typedef struct _CodeSlayerPluginClass CodeSlayerPluginClass;

struct _CodeSlayerPlugin
{
  GObject parent_instance;
};

struct _CodeSlayerPluginClass
{
  GObjectClass parent_class;
};

GType codeslayer_plugin_get_type (void) G_GNUC_CONST;

CodeSlayerPlugin*  codeslayer_plugin_new               (void);

void               codeslayer_plugin_set_data          (CodeSlayerPlugin *plugin, 
                                                        GObject          *data);

const gchar*       codeslayer_plugin_get_lib           (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_set_lib           (CodeSlayerPlugin *plugin, 
                                                        const gchar      *lib);

const gchar*       codeslayer_plugin_get_version       (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_set_version       (CodeSlayerPlugin *plugin, 
                                                        const gchar      *version);

const gchar*       codeslayer_plugin_get_name          (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_set_name          (CodeSlayerPlugin *plugin, 
                                                        const gchar      *name);

const gchar*       codeslayer_plugin_get_description   (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_set_description   (CodeSlayerPlugin *plugin, 
                                                        const gchar      *description);

const gchar*       codeslayer_plugin_get_authors       (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_set_authors       (CodeSlayerPlugin *plugin, 
                                                        const gchar      *authors);

const gchar*       codeslayer_plugin_get_copyright     (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_set_copyright     (CodeSlayerPlugin *plugin, 
                                                        const gchar      *copyright);

const gchar*       codeslayer_plugin_get_website       (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_set_website       (CodeSlayerPlugin *plugin, 
                                                        const gchar      *website);

gboolean           codeslayer_plugin_get_enabled       (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_set_enabled       (CodeSlayerPlugin *plugin, 
                                                        gboolean          enabled);
void               codeslayer_plugin_activate          (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_deactivate        (CodeSlayerPlugin *plugin);
void               codeslayer_plugin_configure         (CodeSlayerPlugin *plugin);
gboolean           codeslayer_plugin_is_configurable   (CodeSlayerPlugin *plugin);

G_END_DECLS

#endif /* __CODESLAYER_PLUGIN_H__ */
