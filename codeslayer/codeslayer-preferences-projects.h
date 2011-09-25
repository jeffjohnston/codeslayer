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

#ifndef __CODESLAYER_PREFERENCES_PROJECTS_H__
#define	__CODESLAYER_PREFERENCES_PROJECTS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-preferences.h>

G_BEGIN_DECLS

#define CODESLAYER_PREFERENCES_PROJECTS_TYPE            (codeslayer_preferences_projects_get_type ())
#define CODESLAYER_PREFERENCES_PROJECTS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PREFERENCES_PROJECTS_TYPE, CodeSlayerPreferencesProjects))
#define CODESLAYER_PREFERENCES_PROJECTS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PREFERENCES_PROJECTS_TYPE, CodeSlayerPreferencesProjectsClass))
#define IS_CODESLAYER_PREFERENCES_PROJECTS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PREFERENCES_PROJECTS_TYPE))
#define IS_CODESLAYER_PREFERENCES_PROJECTS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PREFERENCES_PROJECTS_TYPE))

typedef struct _CodeSlayerPreferencesProjects CodeSlayerPreferencesProjects;
typedef struct _CodeSlayerPreferencesProjectsClass CodeSlayerPreferencesProjectsClass;

struct _CodeSlayerPreferencesProjects
{
  GtkVBox parent_instance;
};

struct _CodeSlayerPreferencesProjectsClass
{
  GtkVBoxClass parent_class;
};

GType codeslayer_preferences_projects_get_type (void) G_GNUC_CONST;

GtkWidget*  codeslayer_preferences_projects_new  (CodeSlayerPreferences *preferences);

G_END_DECLS

#endif /* __CODESLAYER_PREFERENCES_PROJECTS_H__ */
