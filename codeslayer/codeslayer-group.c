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

#include <codeslayer/codeslayer-group.h>

/**
 * SECTION:codeslayer-group
 * @short_description: Contains the projects.
 * @title: CodeSlayerGroup
 * @include: codeslayer/codeslayer-group.h
 */

static void codeslayer_group_class_init           (CodeSlayerGroupClass *klass);
static void codeslayer_group_init                 (CodeSlayerGroup      *group);
static void codeslayer_group_finalize             (CodeSlayerGroup      *group);
static void codeslayer_group_get_property         (GObject              *object, 
                                                   guint                 prop_id, 
                                                   GValue               *value, 
                                                   GParamSpec           *pspec);
static void codeslayer_group_set_property         (GObject              *object, 
                                                   guint                 prop_id, 
                                                   const GValue         *value, 
                                                   GParamSpec           *pspec);
static void codeslayer_group_remove_all_projects  (CodeSlayerGroup      *group);

#define CODESLAYER_GROUP_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_GROUP_TYPE, CodeSlayerGroupPrivate))

typedef struct _CodeSlayerGroupPrivate CodeSlayerGroupPrivate;

struct _CodeSlayerGroupPrivate
{
  gchar *name;
  GList *projects;
  GList *libs;
};

G_DEFINE_TYPE (CodeSlayerGroup, codeslayer_group, G_TYPE_OBJECT)
     
enum
{
  PROP_0,
  PROP_NAME,
  PROP_PROJECTS,
  PROP_LIBS
};

static void 
codeslayer_group_class_init (CodeSlayerGroupClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_group_finalize;

  gobject_class->get_property = codeslayer_group_get_property;
  gobject_class->set_property = codeslayer_group_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerGroupPrivate));

  /**
   * CodeSlayerGroup:name:
   *
   * The text that will be displayed for the group.
   */
  g_object_class_install_property (gobject_class, PROP_NAME,
                                   g_param_spec_string ("name", 
                                                        "Name", 
                                                        "Name", "",
                                                        G_PARAM_READWRITE));

  /**
   * CodeSlayerGroup:projects:
   *
   * The list of projects in the group.
   */
  g_object_class_install_property (gobject_class, PROP_PROJECTS,
                                   g_param_spec_pointer ("projects",
                                                         "Projects",
                                                         "Projects",
                                                         G_PARAM_READWRITE));
                                                         
  /**
   * CodeSlayerGroup:libs:
   *
   * The list of libs in the group.
   */
  g_object_class_install_property (gobject_class, PROP_LIBS,
                                   g_param_spec_pointer ("libs",
                                                         "Libs",
                                                         "Libs",
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_group_init (CodeSlayerGroup *group)
{
  CodeSlayerGroupPrivate *priv; 
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->name = NULL;
  priv->projects = NULL;
  priv->libs = NULL;
}

static void
codeslayer_group_finalize (CodeSlayerGroup *group)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  if (priv->name)
    {
      g_free (priv->name);
    }
  if (priv->libs)
    {
      g_list_foreach (priv->libs, (GFunc) g_free, NULL);    
      g_list_free (priv->libs);
    }
  if (priv->projects)
    {
      codeslayer_group_remove_all_projects (group);
      g_list_free (priv->projects);          
    }
  G_OBJECT_CLASS (codeslayer_group_parent_class)->finalize (G_OBJECT (group));
}

static void
codeslayer_group_get_property (GObject    *object, 
                               guint       prop_id,
                               GValue     *value, 
                               GParamSpec *pspec)
{
  CodeSlayerGroup *group;
  CodeSlayerGroupPrivate *priv;
  
  group = CODESLAYER_GROUP (object);
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_PROJECTS:
      g_value_set_pointer (value, priv->projects);
      break;
    case PROP_LIBS:
      g_value_set_pointer (value, priv->libs);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_group_set_property (GObject      *object, 
                               guint         prop_id,
                               const GValue *value, 
                               GParamSpec   *pspec)
{
  CodeSlayerGroup *group;
  
  group = CODESLAYER_GROUP (object);

  switch (prop_id)
    {
    case PROP_NAME:
      codeslayer_group_set_name (group, g_value_get_string (value));
      break;
    case PROP_PROJECTS:
      codeslayer_group_set_projects (group, g_value_get_pointer (value));
      break;
    case PROP_LIBS:
      codeslayer_group_set_libs (group, g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_group_new:
 *
 * Creates a new #CodeSlayerGroup.
 *
 * Returns: a new #CodeSlayerGroup. 
 */
CodeSlayerGroup *
codeslayer_group_new (void)
{
  return CODESLAYER_GROUP (g_object_new (codeslayer_group_get_type (), NULL));
}

/**
 * codeslayer_group_get_name:
 * @group: a #CodeSlayerGroup.
 *
 * Returns: the text to display for the group.
 */
const gchar *
codeslayer_group_get_name (CodeSlayerGroup *group)
{
  return CODESLAYER_GROUP_GET_PRIVATE (group)->name;
}

/**
 * codeslayer_group_set_name:
 * @group: a #CodeSlayerGroup.
 * @name: the text to display for the group.
 */
void
codeslayer_group_set_name (CodeSlayerGroup *group, 
                           const gchar     *name)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

/**
 * codeslayer_group_get_projects:
 * @group: a #CodeSlayerGroup.
 *
 * Returns: The list of #CodeSlayerProject objects within the group.
 */
GList *
codeslayer_group_get_projects (CodeSlayerGroup *group)
{
  return CODESLAYER_GROUP_GET_PRIVATE (group)->projects;
}

/**
 * codeslayer_group_set_projects:
 * @group: a #CodeSlayerGroup.
 * @projects: the list of #CodeSlayerProject objects to add to the group.
 */
void
codeslayer_group_set_projects (CodeSlayerGroup *group, 
                               GList           *projects)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->projects = projects;
  g_list_foreach (priv->projects, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_group_find_project:
 * @group: a #CodeSlayerGroup.
 * @project_key: the key of the #CodeSlayerProject to find.
 *
 * Returns: the project found by project_key. Will return NULL if the 
 *          project specified by project_key is not found.
 */
CodeSlayerProject*
codeslayer_group_find_project (CodeSlayerGroup  *group,
                               const gchar      *project_key)
{
  CodeSlayerGroupPrivate *priv;
  GList *projects;

  priv = CODESLAYER_GROUP_GET_PRIVATE (group);

  projects = priv->projects;
  while (projects != NULL)
    {
      CodeSlayerProject *project = projects->data;
      if (g_strcmp0 (codeslayer_project_get_key (project), project_key) == 0)
        return project;
      projects = g_list_next (projects);
    }
  return NULL;
}

/**
 * codeslayer_group_add_project:
 * @group: a #CodeSlayerGroup.
 * @project: the #CodeSlayerProject to add to the group.
 */
void
codeslayer_group_add_project (CodeSlayerGroup   *group,
                              CodeSlayerProject *project)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->projects = g_list_prepend (priv->projects, project);
  g_object_ref_sink (G_OBJECT (project));
}

/**
 * codeslayer_group_remove_project:
 * @group: a #CodeSlayerGroup.
 * @project: the #CodeSlayerProject to remove from the group.
 */
void
codeslayer_group_remove_project (CodeSlayerGroup   *group,
                                 CodeSlayerProject *project)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->projects = g_list_remove (priv->projects, project);
  g_object_unref (project);
}

/**
 * codeslayer_group_contains_project:
 * @group: a #CodeSlayerGroup.
 * @project: the #CodeSlayerProject to check.
 */
gboolean            
codeslayer_group_contains_project (CodeSlayerGroup   *group,
                                   CodeSlayerProject *project)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  return g_list_index (priv->projects, project) != -1;
}                                   

static void
codeslayer_group_remove_all_projects (CodeSlayerGroup *group)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  g_list_foreach (priv->projects, (GFunc) g_object_unref, NULL);
  priv->projects = g_list_remove_all (priv->projects, NULL);
}

/**
 * codeslayer_group_get_libs:
 * @group: a #CodeSlayerGroup.
 *
 * Returns: The list of #CodeSlayerPlugin lib objects within the group.
 */
GList*
codeslayer_group_get_libs (CodeSlayerGroup *group)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  return priv->libs;
}

/**
 * codeslayer_group_set_libs:
 * @group: a #CodeSlayerGroup.
 * @libs: the list of #CodeSlayerPlugin lib objects to add to the group.
 */
void
codeslayer_group_set_libs (CodeSlayerGroup *group, 
                           GList           *libs)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->libs = libs;
}

/**
 * codeslayer_group_contains_lib:
 * @group: a #CodeSlayerGroup.
 * @lib: the lib to find.
 *
 * Returns: is TRUE if the lib is found in the group.
 */
gboolean
codeslayer_group_contains_lib (CodeSlayerGroup *group, 
                               const gchar     *lib)
{
  CodeSlayerGroupPrivate *priv;
  GList *libs;

  priv = CODESLAYER_GROUP_GET_PRIVATE (group);

  libs = priv->libs;

  while (libs != NULL)
    {
      gchar *name = libs->data;
      if (g_strcmp0 (name, lib) == 0)
        return TRUE;
      libs = g_list_next (libs);
    }
    
  return FALSE;

}                                                        

/**
 * codeslayer_group_add_lib:
 * @group: a #CodeSlayerGroup.
 * @lib: the lib to add.
 */
void
codeslayer_group_add_lib (CodeSlayerGroup *group, 
                          const gchar     *lib)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->libs = g_list_prepend (priv->libs, g_strdup (lib));
}                                                        

/**
 * codeslayer_group_remove_lib:
 * @group: a #CodeSlayerGroup.
 * @lib: the lib to remove.
 */
void
codeslayer_group_remove_lib (CodeSlayerGroup *group, 
                             const gchar     *lib)
{
  CodeSlayerGroupPrivate *priv;
  GList *libs;

  priv = CODESLAYER_GROUP_GET_PRIVATE (group);

  libs = priv->libs;

  while (libs != NULL)
    {
      gchar *name = libs->data;
      if (g_strcmp0 (name, lib) == 0)
        {
          priv->libs = g_list_remove (priv->libs, name);
          return;
        }
      libs = g_list_next (libs);
    }
}                             
