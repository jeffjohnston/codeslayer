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

#ifndef __CODESLAYER_PLUGINS_SELECTOR_H__
#define	__CODESLAYER_PLUGINS_SELECTOR_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-plugins.h>

G_BEGIN_DECLS

#define CODESLAYER_PLUGINS_SELECTOR_TYPE            (codeslayer_plugins_selector_get_type ())
#define CODESLAYER_PLUGINS_SELECTOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PLUGINS_SELECTOR_TYPE, CodeSlayerPluginsSelector))
#define CODESLAYER_PLUGINS_SELECTOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PLUGINS_SELECTOR_TYPE, CodeSlayerPluginsSelectorClass))
#define IS_CODESLAYER_PLUGINS_SELECTOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PLUGINS_SELECTOR_TYPE))
#define IS_CODESLAYER_PLUGINS_SELECTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PLUGINS_SELECTOR_TYPE))

typedef struct _CodeSlayerPluginsSelector CodeSlayerPluginsSelector;
typedef struct _CodeSlayerPluginsSelectorClass CodeSlayerPluginsSelectorClass;

struct _CodeSlayerPluginsSelector
{
  GtkVBox parent_instance;
};

struct _CodeSlayerPluginsSelectorClass
{
  GtkVBoxClass parent_class;
};

GType codeslayer_plugins_selector_get_type (void) G_GNUC_CONST;

GtkWidget*  codeslayer_plugins_selector_new  (CodeSlayerPlugins *plugins, 
                                              CodeSlayerProfile *profile);

G_END_DECLS

#endif /* __CODESLAYER_PLUGINS_SELECTOR_H__ */
