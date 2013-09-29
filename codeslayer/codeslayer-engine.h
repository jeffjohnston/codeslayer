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

#ifndef __CODESLAYER_ENGINE_H__
#define	__CODESLAYER_ENGINE_H__

#include <gtk/gtk.h>

#include <codeslayer/codeslayer-abstract-engine.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-profile-handler.h>
#include <codeslayer/codeslayer-plugins.h>

G_BEGIN_DECLS

#define CODESLAYER_ENGINE_TYPE            (codeslayer_engine_get_type ())
#define CODESLAYER_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_ENGINE_TYPE, CodeSlayerEngine))
#define CODESLAYER_ENGINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_ENGINE_TYPE, CodeSlayerEngineClass))
#define IS_CODESLAYER_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_ENGINE_TYPE))
#define IS_CODESLAYER_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_ENGINE_TYPE))

typedef struct _CodeSlayerEngine CodeSlayerEngine;
typedef struct _CodeSlayerEngineClass CodeSlayerEngineClass;

struct _CodeSlayerEngine
{
  CodeSlayerAbstractEngine parent_instance;
};

struct _CodeSlayerEngineClass
{
  CodeSlayerAbstractEngineClass parent_class;
};

GType codeslayer_engine_get_type (void) G_GNUC_CONST;

CodeSlayerEngine*  codeslayer_engine_new                  (GtkWindow             *window,
                                                           CodeSlayerRegistry    *registry,
                                                           CodeSlayerPreferences *preferences,
                                                           CodeSlayerProfileHandler *profile_handler,
                                                           CodeSlayerPlugins     *plugins,
                                                           GtkWidget             *menubar,
                                                           GtkWidget             *notebook,
                                                           GtkWidget             *notebook_pane,
                                                           GtkWidget             *side_pane,
                                                           GtkWidget             *bottom_pane, 
                                                           GtkWidget             *hpaned,
                                                           GtkWidget             *vpaned);

void               codeslayer_engine_load_default_profile  (CodeSlayerEngine      *engine);
void               codeslayer_engine_open_editor          (CodeSlayerEngine      *engine, 
                                                           gchar                 *file_path);

G_END_DECLS

#endif /* _CODESLAYER_ENGINE_H */
