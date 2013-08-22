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

#include <codeslayer/codeslayer-config.h>

/**
 * SECTION:codeslayer-group
 * @short_description: Contains the projects.
 * @title: CodeSlayerConfig
 * @include: codeslayer/codeslayer-group.h
 */

static void codeslayer_config_class_init              (CodeSlayerConfigClass *klass);
static void codeslayer_config_init                    (CodeSlayerConfig      *group);
static void codeslayer_config_finalize                (CodeSlayerConfig      *group);
static void codeslayer_config_get_property            (GObject              *object, 
                                                      guint                 prop_id, 
                                                      GValue               *value, 
                                                      GParamSpec           *pspec);
static void codeslayer_config_set_property            (GObject              *object, 
                                                      guint                 prop_id, 
                                                      const GValue         *value, 
                                                      GParamSpec           *pspec);
static void codeslayer_config_remove_all_projects     (CodeSlayerConfig      *group);
static void codeslayer_config_remove_all_documents    (CodeSlayerConfig      *group);
static void codeslayer_config_remove_all_libs         (CodeSlayerConfig      *group);
static void codeslayer_config_remove_all_preferences  (CodeSlayerConfig      *group);

#define CODESLAYER_CONFIG_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_CONFIG_TYPE, CodeSlayerConfigPrivate))

typedef struct _CodeSlayerConfigPrivate CodeSlayerConfigPrivate;

struct _CodeSlayerConfigPrivate
{
  gchar *file_path;
  GList *projects;
  GList *documents;
  GList *libs;
  GList *preferences;
};

G_DEFINE_TYPE (CodeSlayerConfig, codeslayer_config, G_TYPE_OBJECT)
     
enum
{
  PROP_0,
  PROP_FILE_PATH,
  PROP_PROJECTS,
  PROP_DOCUMENTS,
  PROP_LIBS, 
  PROP_PREFERENCES  
};

static void 
codeslayer_config_class_init (CodeSlayerConfigClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_config_finalize;

  gobject_class->get_property = codeslayer_config_get_property;
  gobject_class->set_property = codeslayer_config_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerConfigPrivate));

  /**
   * CodeSlayerConfig:file_path:
   *
   * The text that will be displayed for the group.
   */
  g_object_class_install_property (gobject_class, PROP_FILE_PATH,
                                   g_param_spec_string ("file_path", 
                                                        "File Path", 
                                                        "File Path", "",
                                                        G_PARAM_READWRITE));

  /**
   * CodeSlayerConfig:projects:
   *
   * The list of projects in the group.
   */
  g_object_class_install_property (gobject_class, PROP_PROJECTS,
                                   g_param_spec_pointer ("projects",
                                                         "Projects",
                                                         "Projects",
                                                         G_PARAM_READWRITE));

  /**
   * CodeSlayerConfig:documents:
   *
   * The list of documents in the group.
   */
  g_object_class_install_property (gobject_class, PROP_LIBS,
                                   g_param_spec_pointer ("documents",
                                                         "Documents",
                                                         "Documents",
                                                         G_PARAM_READWRITE));
                                                         
  /**
   * CodeSlayerConfig:libs:
   *
   * The list of libs in the group.
   */
  g_object_class_install_property (gobject_class, PROP_LIBS,
                                   g_param_spec_pointer ("libs",
                                                         "Libs",
                                                         "Libs",
                                                         G_PARAM_READWRITE));
                                                         
  /**
   * CodeSlayerConfig:preferences:
   *
   * The list of preferences in the group.
   */
  g_object_class_install_property (gobject_class, PROP_LIBS,
                                   g_param_spec_pointer ("preferences",
                                                         "Preferences",
                                                         "Preferences",
                                                         G_PARAM_READWRITE));                                                         
}

static void
codeslayer_config_init (CodeSlayerConfig *group)
{
  CodeSlayerConfigPrivate *priv; 
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->file_path = NULL;
  priv->projects = NULL;
  priv->documents = NULL;
  priv->libs = NULL;
  priv->preferences = NULL;
}

static void
codeslayer_config_finalize (CodeSlayerConfig *group)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  
  if (priv->file_path)
    g_free (priv->file_path);
  
  codeslayer_config_remove_all_projects (group);
  codeslayer_config_remove_all_documents (group);
  codeslayer_config_remove_all_libs (group);
  codeslayer_config_remove_all_preferences (group);

  G_OBJECT_CLASS (codeslayer_config_parent_class)->finalize (G_OBJECT (group));
  
  g_print ("codeslayer_config_finalize\n");
}

static void
codeslayer_config_get_property (GObject    *object, 
                               guint       prop_id,
                               GValue     *value, 
                               GParamSpec *pspec)
{
  CodeSlayerConfig *group;
  CodeSlayerConfigPrivate *priv;
  
  group = CODESLAYER_CONFIG (object);
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);

  switch (prop_id)
    {
    case PROP_FILE_PATH:
      g_value_set_string (value, priv->file_path);
      break;
    case PROP_PROJECTS:
      g_value_set_pointer (value, priv->projects);
      break;
    case PROP_DOCUMENTS:
      g_value_set_pointer (value, priv->documents);
      break;
    case PROP_LIBS:
      g_value_set_pointer (value, priv->libs);
      break;
    case PROP_PREFERENCES:
      g_value_set_pointer (value, priv->preferences);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_config_set_property (GObject      *object, 
                               guint         prop_id,
                               const GValue *value, 
                               GParamSpec   *pspec)
{
  CodeSlayerConfig *group;
  
  group = CODESLAYER_CONFIG (object);

  switch (prop_id)
    {
    case PROP_FILE_PATH:
      codeslayer_config_set_file_path (group, g_value_get_string (value));
      break;
    case PROP_PROJECTS:
      codeslayer_config_set_projects (group, g_value_get_pointer (value));
      break;
    case PROP_DOCUMENTS:
      codeslayer_config_set_documents (group, g_value_get_pointer (value));
      break;
    case PROP_LIBS:
      codeslayer_config_set_libs (group, g_value_get_pointer (value));
      break;
    case PROP_PREFERENCES:
      codeslayer_config_set_preferences (group, g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_config_new:
 *
 * Creates a new #CodeSlayerConfig.
 *
 * Returns: a new #CodeSlayerConfig. 
 */
CodeSlayerConfig*
codeslayer_config_new (void)
{
  return CODESLAYER_CONFIG (g_object_new (codeslayer_config_get_type (), NULL));
}

/**
 * codeslayer_config_get_file_path:
 * @group: a #CodeSlayerConfig.
 *
 * Returns: the text to display for the group. For internal use only.
 */
const gchar*
codeslayer_config_get_file_path (CodeSlayerConfig *group)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (group)->file_path;
}

/**
 * codeslayer_config_set_file_path:
 * @group: a #CodeSlayerConfig.
 * @name: the text to display for the group.
 */
void
codeslayer_config_set_file_path (CodeSlayerConfig *group, 
                                const gchar     *file_path)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}

/**
 * codeslayer_config_get_projects:
 * @group: a #CodeSlayerConfig.
 *
 * Returns: The list of #CodeSlayerProject objects within the group.
 */
GList*
codeslayer_config_get_projects (CodeSlayerConfig *group)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (group)->projects;
}

/**
 * codeslayer_config_set_projects:
 * @group: a #CodeSlayerConfig.
 * @projects: the list of #CodeSlayerProject objects to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_config_set_projects (CodeSlayerConfig *group, 
                               GList           *projects)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->projects = projects;
  g_list_foreach (priv->projects, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_config_get_project_by_file_path:
 * @group: a #CodeSlayerConfig.
 * @file_path: the file_path of the #CodeSlayerProject to find.
 *
 * Returns: the project found by given file path. Will return NULL if the 
 *          project specified is not found.
 */
CodeSlayerProject*
codeslayer_config_get_project_by_file_path (CodeSlayerConfig *group, 
                                           const gchar *file_path)
{
  CodeSlayerConfigPrivate *priv;
  GList *list;
  
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);

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
 * codeslayer_config_add_project:
 * @group: a #CodeSlayerConfig.
 * @project: the #CodeSlayerProject to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_config_add_project (CodeSlayerConfig   *group,
                              CodeSlayerProject *project)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->projects = g_list_prepend (priv->projects, project);
  g_object_ref_sink (G_OBJECT (project));
}

/**
 * codeslayer_config_remove_project:
 * @group: a #CodeSlayerConfig.
 * @project: the #CodeSlayerProject to remove from the group.
 *
 * For internal use only.
 */
void
codeslayer_config_remove_project (CodeSlayerConfig   *group,
                                 CodeSlayerProject *project)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->projects = g_list_remove (priv->projects, project);
  g_object_unref (project);
}

/**
 * codeslayer_config_contains_project:
 * @group: a #CodeSlayerConfig.
 * @project: the #CodeSlayerProject to check.
 */
gboolean            
codeslayer_config_contains_project (CodeSlayerConfig   *group,
                                   CodeSlayerProject *project)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  return g_list_index (priv->projects, project) != -1;
}                                   

static void
codeslayer_config_remove_all_projects (CodeSlayerConfig *group)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);  
  if (priv->projects)
    {
      g_list_foreach (priv->projects, (GFunc) g_object_unref, NULL);
      priv->projects = g_list_remove_all (priv->projects, NULL);
      g_list_free (priv->projects);
      priv->projects = NULL;
    }
}

/**
 * codeslayer_config_get_documents:
 * @group: a #CodeSlayerConfig.
 *
 * Returns: The list of #CodeSlayerDocuments objects within the group.
 */
GList *
codeslayer_config_get_documents (CodeSlayerConfig *group)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (group)->documents;
}

/**
 * codeslayer_config_set_documents:
 * @group: a #CodeSlayerConfig.
 * @projects: the list of #CodeSlayerDocument objects to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_config_set_documents (CodeSlayerConfig *group, 
                                GList           *documents)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->documents = documents;
  g_list_foreach (priv->documents, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_config_add_document:
 * @group: a #CodeSlayerConfig.
 * @document: the #CodeSlayerDocument to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_config_add_document (CodeSlayerConfig    *group,
                               CodeSlayerDocument *document)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  if (priv->documents == NULL || g_list_index (priv->documents, document) == -1)
    {
      priv->documents = g_list_prepend (priv->documents, document);
      g_object_ref_sink (G_OBJECT (document));
    }
}

/**
 * codeslayer_config_remove_document:
 * @group: a #CodeSlayerConfig.
 * @document: the #CodeSlayerDocument to remove from the group.
 *
 * For internal use only.
 */
void
codeslayer_config_remove_document (CodeSlayerConfig    *group,
                                  CodeSlayerDocument *document)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->documents = g_list_remove (priv->documents, document);
}

static void
codeslayer_config_remove_all_documents (CodeSlayerConfig *group)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  if (priv->documents)
    {
      /*g_list_foreach (priv->documents, (GFunc) g_object_unref, NULL);*/
      priv->documents = g_list_remove_all (priv->documents, NULL);
      g_list_free (priv->documents);
      priv->documents = NULL;
    }  
}

/**
 * codeslayer_config_get_libs:
 * @group: a #CodeSlayerConfig.
 *
 * Returns: The list of #CodeSlayerPlugin lib objects within the group. For
 *          internal use only.
 */
GList*
codeslayer_config_get_libs (CodeSlayerConfig *group)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  return priv->libs;
}

/**
 * codeslayer_config_set_libs:
 * @group: a #CodeSlayerConfig.
 * @libs: the list of #CodeSlayerPlugin lib objects to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_config_set_libs (CodeSlayerConfig *group, 
                           GList           *libs)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->libs = libs;
}

/**
 * codeslayer_config_contains_lib:
 * @group: a #CodeSlayerConfig.
 * @lib: the lib to find.
 *
 * Returns: is TRUE if the lib is found in the group. For internal use only.
 */
gboolean
codeslayer_config_contains_lib (CodeSlayerConfig *group, 
                               const gchar     *lib)
{
  CodeSlayerConfigPrivate *priv;
  GList *libs;

  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);

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
 * codeslayer_config_add_lib:
 * @group: a #CodeSlayerConfig.
 * @lib: the lib to add.
 *
 * For internal use only.
 */
void
codeslayer_config_add_lib (CodeSlayerConfig *group, 
                          const gchar     *lib)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->libs = g_list_prepend (priv->libs, g_strdup (lib));
}                                                        

static void
codeslayer_config_remove_all_libs (CodeSlayerConfig *group)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  if (priv->libs)
    {
      g_list_foreach (priv->libs, (GFunc) g_free, NULL);
      priv->libs = g_list_remove_all (priv->libs, NULL);
      g_list_free (priv->libs);
      priv->libs = NULL;
    }
}

/**
 * codeslayer_config_remove_lib:
 * @group: a #CodeSlayerConfig.
 * @lib: the lib to remove.
 *
 * For internal use only.
 */
void
codeslayer_config_remove_lib (CodeSlayerConfig *group, 
                             const gchar     *lib)
{
  CodeSlayerConfigPrivate *priv;
  GList *libs;

  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);

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

/**
 * codeslayer_config_get_preferences:
 * @group: a #CodeSlayerConfig.
 *
 * Returns: The list of #CodeSlayerPreference objects within the group.
 */
GList *
codeslayer_config_get_preferences (CodeSlayerConfig *group)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (group)->preferences;
}

/**
 * codeslayer_config_set_preferences:
 * @group: a #CodeSlayerConfig.
 * @projects: the list of #CodeSlayerPreference objects to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_config_set_preferences (CodeSlayerConfig *group, 
                                  GList           *preferences)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->preferences = preferences;
  g_list_foreach (priv->preferences, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_config_add_preference:
 * @group: a #CodeSlayerConfig.
 * @preference: the #CodeSlayerPreference to add to the group.
 *
 * For internal use only.
 */
void
codeslayer_config_add_preference (CodeSlayerConfig      *group,
                                 CodeSlayerPreference *preference)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  priv->preferences = g_list_prepend (priv->preferences, preference);
  g_object_ref_sink (G_OBJECT (preference));
}

static void
codeslayer_config_remove_all_preferences (CodeSlayerConfig *group)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (group);
  if (priv->preferences)
    {
      g_list_foreach (priv->preferences, (GFunc) g_object_unref, NULL);
      priv->preferences = g_list_remove_all (priv->preferences, NULL);
      g_list_free (priv->preferences);
      priv->preferences = NULL;
    }
}
