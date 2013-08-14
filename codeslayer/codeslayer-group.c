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

static void codeslayer_group_class_init            (CodeSlayerGroupClass *klass);
static void codeslayer_group_init                  (CodeSlayerGroup      *group);
static void codeslayer_group_finalize              (CodeSlayerGroup      *group);
static void codeslayer_group_get_property          (GObject              *object, 
                                                    guint                 prop_id, 
                                                    GValue               *value, 
                                                    GParamSpec           *pspec);
static void codeslayer_group_set_property          (GObject              *object, 
                                                    guint                 prop_id, 
                                                    const GValue         *value, 
                                                    GParamSpec           *pspec);
static void codeslayer_group_remove_all_projects   (CodeSlayerGroup      *group);
static void codeslayer_group_remove_all_documents  (CodeSlayerGroup      *group);
static void codeslayer_group_remove_all_preferences  (CodeSlayerGroup      *group);

#define CODESLAYER_GROUP_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_GROUP_TYPE, CodeSlayerGroupPrivate))

typedef struct _CodeSlayerGroupPrivate CodeSlayerGroupPrivate;

struct _CodeSlayerGroupPrivate
{
  gchar *name;
  GList *projects;
  GList *documents;
  GList *preferences;
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
  priv->documents = NULL;
  priv->preferences = NULL;
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
  if (priv->documents)
    {
      codeslayer_group_remove_all_documents (group);
      g_list_free (priv->documents);          
    }
  if (priv->preferences)
    {
      codeslayer_group_remove_all_preferences (group);
      g_list_free (priv->preferences);          
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
CodeSlayerGroup*
codeslayer_group_new (void)
{
  return CODESLAYER_GROUP (g_object_new (codeslayer_group_get_type (), NULL));
}

/**
 * codeslayer_group_get_name:
 * @group: a #CodeSlayerGroup.
 *
 * Returns: the text to display for the group. For internal use only.
 */
const gchar*
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
 *
 * For internal use only.
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
 * codeslayer_group_get_project_by_file_path:
 * @group: a #CodeSlayerGroup.
 * @file_path: the file_path of the #CodeSlayerProject to find.
 *
 * Returns: the project found by given file path. Will return NULL if the 
 *          project specified is not found.
 */
CodeSlayerProject*
codeslayer_group_get_project_by_file_path (CodeSlayerGroup *group, 
                                           const gchar *file_path)
{
  CodeSlayerGroupPrivate *priv;
  GList *list;
  
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);

  list = priv->projects;
  while (list != NULL)
    {
      CodeSlayerProject *project = list->data;
      gchar *folder_path_expanded;
      const gchar *folder_path = codeslayer_project_get_folder_path (project);
      folder_path_expanded = g_strconcat (folder_path, G_DIR_SEPARATOR_S, NULL);
      
      if (g_str_has_prefix (file_path, folder_path_expanded))
        {
          g_free (folder_path_expanded);
          return project;
        }
      
      g_free (folder_path_expanded);
      list = g_list_next (list);
    }
  
  return NULL;
}

/**
 * codeslayer_group_add_project:
 * @group: a #CodeSlayerGroup.
 * @project: the #CodeSlayerProject to add to the group.
 *
 * For internal use only.
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
 *
 * For internal use only.
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
 * codeslayer_group_get_documents:
 * @group: a #CodeSlayerGroup.
 *
 * Returns: The list of #CodeSlayerDocuments objects within the group.
 */
GList *
codeslayer_group_get_documents (CodeSlayerGroup *group)
{
  return CODESLAYER_GROUP_GET_PRIVATE (group)->documents;
}

/**
 * codeslayer_group_set_documents:
 * @group: a #CodeSlayerGroup.
 * @projects: the list of #CodeSlayerDocument objects to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_group_set_documents (CodeSlayerGroup *group, 
                                GList           *documents)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->documents = documents;
  g_list_foreach (priv->documents, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_group_add_document:
 * @group: a #CodeSlayerGroup.
 * @document: the #CodeSlayerDocument to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_group_add_document (CodeSlayerGroup    *group,
                               CodeSlayerDocument *document)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->documents = g_list_prepend (priv->documents, document);
  g_object_ref_sink (G_OBJECT (document));
}

static void
codeslayer_group_remove_all_documents (CodeSlayerGroup *group)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  g_list_foreach (priv->documents, (GFunc) g_object_unref, NULL);
  priv->documents = g_list_remove_all (priv->documents, NULL);
}

/**
 * codeslayer_group_get_preferences:
 * @group: a #CodeSlayerGroup.
 *
 * Returns: The list of #CodeSlayerPreference objects within the group.
 */
GList *
codeslayer_group_get_preferences (CodeSlayerGroup *group)
{
  return CODESLAYER_GROUP_GET_PRIVATE (group)->preferences;
}

/**
 * codeslayer_group_set_preferences:
 * @group: a #CodeSlayerGroup.
 * @projects: the list of #CodeSlayerPreference objects to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_group_set_preferences (CodeSlayerGroup *group, 
                                  GList           *preferences)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->preferences = preferences;
  g_list_foreach (priv->preferences, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_group_add_preference:
 * @group: a #CodeSlayerGroup.
 * @preference: the #CodeSlayerPreference to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_group_add_preference (CodeSlayerGroup      *group,
                                 CodeSlayerPreference *preference)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  priv->preferences = g_list_prepend (priv->preferences, preference);
  g_object_ref_sink (G_OBJECT (preference));
}

static void
codeslayer_group_remove_all_preferences (CodeSlayerGroup *group)
{
  CodeSlayerGroupPrivate *priv;
  priv = CODESLAYER_GROUP_GET_PRIVATE (group);
  g_list_foreach (priv->preferences, (GFunc) g_object_unref, NULL);
  priv->preferences = g_list_remove_all (priv->preferences, NULL);
}

/**
 * codeslayer_group_get_libs:
 * @group: a #CodeSlayerGroup.
 *
 * Returns: The list of #CodeSlayerPlugin lib objects within the group. For
 *          internal use only.
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
 *
 * For internal use only.
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
 * Returns: is TRUE if the lib is found in the group. For internal use only.
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
 *
 * For internal use only.
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
 *
 * For internal use only.
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
