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

#ifndef __CODESLAYER_PROJECTS_ENGINE_H__
#define	__CODESLAYER_PROJECTS_ENGINE_H__

#include <gtk/gtk.h>

#include <codeslayer/codeslayer-abstract-engine.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-profiles.h>
#include <codeslayer/codeslayer-plugins.h>

G_BEGIN_DECLS

#define CODESLAYER_PROJECTS_ENGINE_TYPE            (codeslayer_projects_engine_get_type ())
#define CODESLAYER_PROJECTS_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROJECTS_ENGINE_TYPE, CodeSlayerProjectsEngine))
#define CODESLAYER_PROJECTS_ENGINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROJECTS_ENGINE_TYPE, CodeSlayerProjectsEngineClass))
#define IS_CODESLAYER_PROJECTS_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROJECTS_ENGINE_TYPE))
#define IS_CODESLAYER_PROJECTS_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROJECTS_ENGINE_TYPE))

typedef struct _CodeSlayerProjectsEngine CodeSlayerProjectsEngine;
typedef struct _CodeSlayerProjectsEngineClass CodeSlayerProjectsEngineClass;

struct _CodeSlayerProjectsEngine
{
  CodeSlayerAbstractEngine parent_instance;
};

struct _CodeSlayerProjectsEngineClass
{
  CodeSlayerAbstractEngineClass parent_class;
};

GType codeslayer_projects_engine_get_type (void) G_GNUC_CONST;

CodeSlayerProjectsEngine*  codeslayer_projects_engine_new          (GtkWindow                *window,
                                                                    CodeSlayerProfiles       *profiles,
                                                                    CodeSlayerPlugins        *plugins,
                                                                    GtkWidget                *projects, 
                                                                    GtkWidget                *menubar,
                                                                    GtkWidget                *notebook,
                                                                    GtkWidget                *notebook_pane,
                                                                    GtkWidget                *side_pane,
                                                                    GtkWidget                *bottom_pane, 
                                                                    GtkWidget                *hpaned,
                                                                    GtkWidget                *vpaned);

void                       codeslayerprojects_engine_load_profile  (CodeSlayerProjectsEngine *engine);
                                                            

G_END_DECLS

#endif /* _CODESLAYER_PROJECTS_ENGINE_H */
