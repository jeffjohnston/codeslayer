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

#include <codeslayer/codeslayer-preferences-projects.h>
#include <codeslayer/codeslayer-preferences-listview.h>
#include <codeslayer/codeslayer-preferences-utils.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-preferences-projects
 * @short_description: The preferences projects tab.
 * @title: CodeSlayerPreferencesProjects
 * @include: codeslayer/codeslayer-preferences-projects.h
 */

static void codeslayer_preferences_projects_class_init  (CodeSlayerPreferencesProjectsClass *klass);
static void codeslayer_preferences_projects_init        (CodeSlayerPreferencesProjects      *preferences_search);
static void codeslayer_preferences_projects_finalize    (CodeSlayerPreferencesProjects      *preferences_search);

#define CODESLAYER_PREFERENCES_PROJECTS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PREFERENCES_PROJECTS_TYPE, CodeSlayerPreferencesProjectsPrivate))

typedef struct _CodeSlayerPreferencesProjectsPrivate CodeSlayerPreferencesProjectsPrivate;

struct _CodeSlayerPreferencesProjectsPrivate
{
  GObject *exclude_file_types;
  GObject *exclude_directories;
};

G_DEFINE_TYPE (CodeSlayerPreferencesProjects, codeslayer_preferences_projects, GTK_TYPE_VBOX)

static void
codeslayer_preferences_projects_class_init (CodeSlayerPreferencesProjectsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_preferences_projects_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPreferencesProjectsPrivate));
}

static void
codeslayer_preferences_projects_init (CodeSlayerPreferencesProjects *preferences_search)
{
  gtk_box_set_homogeneous (GTK_BOX (preferences_search), FALSE);
  gtk_box_set_spacing (GTK_BOX (preferences_search), 15);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_search), 5);
}

static void
codeslayer_preferences_projects_finalize (CodeSlayerPreferencesProjects *preferences_projects)
{
  CodeSlayerPreferencesProjectsPrivate *priv;
  priv = CODESLAYER_PREFERENCES_PROJECTS_GET_PRIVATE (preferences_projects);
  g_object_unref (priv->exclude_file_types);
  g_object_unref (priv->exclude_directories);
  G_OBJECT_CLASS (codeslayer_preferences_projects_parent_class)->finalize (G_OBJECT (preferences_projects));
}

/**
 * codeslayer_preferences_projects_new:
 * @preferences: a #CodeSlayerPreferences.
 *
 * Creates a new #CodeSlayerPreferencesProjects.
 *
 * Returns: a new #CodeSlayerPreferencesProjects. 
 */
GtkWidget*
codeslayer_preferences_projects_new (CodeSlayerPreferences *preferences, 
                                     CodeSlayerRegistry    *registry)
{
  CodeSlayerPreferencesProjectsPrivate *priv;
  GtkWidget *preferences_projects;

  preferences_projects = g_object_new (codeslayer_preferences_projects_get_type (), NULL);
  priv = CODESLAYER_PREFERENCES_PROJECTS_GET_PRIVATE (preferences_projects);
  
  priv->exclude_file_types = codeslayer_preferences_list_view_new (preferences, registry, 
                                       CODESLAYER_REGISTRY_PROJECTS_EXCLUDE_TYPES,
                                       preferences_projects, _("Exclude File Types"));
  
  priv->exclude_directories = codeslayer_preferences_list_view_new (preferences, registry, 
                                       CODESLAYER_REGISTRY_PROJECTS_EXCLUDE_DIRS,
                                       preferences_projects, _("Exclude Directories"));
                                       
  g_signal_connect_swapped (G_OBJECT (priv->exclude_file_types), "list-changed",
                            G_CALLBACK (codeslayer_preferences_utils_notify), registry);
  
  g_signal_connect_swapped (G_OBJECT (priv->exclude_directories), "list-changed",
                            G_CALLBACK (codeslayer_preferences_utils_notify), registry);
  
  return preferences_projects;
}
