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

#ifndef __CODESLAYER_PLUGINS_H__
#define __CODESLAYER_PLUGINS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-plugin.h>
#include <codeslayer/codeslayer-config.h>

G_BEGIN_DECLS

#define CODESLAYER_PLUGINS_TYPE            (codeslayer_plugins_get_type ())
#define CODESLAYER_PLUGINS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PLUGINS_TYPE, CodeSlayerPlugins))
#define CODESLAYER_PLUGINS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PLUGINS_TYPE, CodeSlayerPluginsClass))
#define IS_CODESLAYER_PLUGINS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PLUGINS_TYPE))
#define IS_CODESLAYER_PLUGINS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PLUGINS_TYPE))

#define CODESLAYER_PLUGINS_DIR "plugins"
#define CODESLAYER_PLUGINS_CONFIG_DIR "config"

typedef struct _CodeSlayerPlugins CodeSlayerPlugins;
typedef struct _CodeSlayerPluginsClass CodeSlayerPluginsClass;

struct _CodeSlayerPlugins
{
  GObject parent_instance;
};

struct _CodeSlayerPluginsClass
{
  GObjectClass parent_class;
};

GType codeslayer_plugins_get_type (void) G_GNUC_CONST;

CodeSlayerPlugins*  codeslayer_plugins_new         (GtkWidget         *window);

void                codeslayer_plugins_load        (CodeSlayerPlugins *plugins, 
                                                    GObject           *data);
void                codeslayer_plugins_activate    (CodeSlayerPlugins *plugins, 
                                                    CodeSlayerConfig  *config);
void                codeslayer_plugins_deactivate  (CodeSlayerPlugins *plugins);
GList*              codeslayer_plugins_get_list    (CodeSlayerPlugins *plugins);
void                codeslayer_plugins_run_dialog  (CodeSlayerPlugins *plugins, 
                                                    CodeSlayerConfig  *config);

G_END_DECLS

#endif /* __CODESLAYER_PLUGINS_H__ */
