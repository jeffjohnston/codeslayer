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
 * SECTION:codeslayer-config
 * @short_description: Contains the projects.
 * @title: CodeSlayerConfig
 * @include: codeslayer/codeslayer-config.h
 */

static void codeslayer_config_class_init              (CodeSlayerConfigClass *klass);
static void codeslayer_config_init                    (CodeSlayerConfig      *config);
static void codeslayer_config_finalize                (CodeSlayerConfig      *config);

static void remove_all_projects     (CodeSlayerConfig      *config);
static void remove_all_documents    (CodeSlayerConfig      *config);
static void remove_all_plugins      (CodeSlayerConfig      *config);
static void remove_all_preferences  (CodeSlayerConfig      *config);
static void remove_all_settings     (CodeSlayerConfig      *config);

#define CODESLAYER_CONFIG_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_CONFIG_TYPE, CodeSlayerConfigPrivate))

typedef struct _CodeSlayerConfigPrivate CodeSlayerConfigPrivate;

struct _CodeSlayerConfigPrivate
{
  gchar      *file_path;
  gboolean    projects_mode;
  GList      *projects;
  GList      *documents;
  GList      *plugins;
  GHashTable *preferences;
  GHashTable *settings;
};

G_DEFINE_TYPE (CodeSlayerConfig, codeslayer_config, G_TYPE_OBJECT)
     
static void 
codeslayer_config_class_init (CodeSlayerConfigClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_config_finalize;

  g_type_class_add_private (klass, sizeof (CodeSlayerConfigPrivate));                                                      
}

static void
codeslayer_config_init (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv; 
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  priv->file_path = NULL;
  priv->projects_mode = FALSE;
  priv->projects = NULL;
  priv->documents = NULL;
  priv->plugins = NULL;
  priv->preferences = NULL;
  priv->settings = NULL;
}

static void
codeslayer_config_finalize (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  
  if (priv->file_path)
    g_free (priv->file_path);
  
  remove_all_projects (config);
  remove_all_documents (config);
  remove_all_plugins (config);
  remove_all_preferences (config);
  remove_all_settings (config);

  G_OBJECT_CLASS (codeslayer_config_parent_class)->finalize (G_OBJECT (config));
  
  g_print ("codeslayer_config_finalize\n");
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
  CodeSlayerConfigPrivate *priv;
  CodeSlayerConfig *config;

  config = CODESLAYER_CONFIG (g_object_new (codeslayer_config_get_type (), NULL));
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  
  priv->preferences = g_hash_table_new_full ((GHashFunc)g_str_hash, (GEqualFunc)g_str_equal, 
                                             (GDestroyNotify)g_free, (GDestroyNotify)g_free);

  priv->settings = g_hash_table_new_full ((GHashFunc)g_str_hash, (GEqualFunc)g_str_equal, 
                                          (GDestroyNotify)g_free, (GDestroyNotify)g_free);

  return config;
}

/**
 * codeslayer_config_get_file_path:
 * @config: a #CodeSlayerConfig.
 *
 * Returns: the text to display for the config.
 */
const gchar*
codeslayer_config_get_file_path (CodeSlayerConfig *config)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (config)->file_path;
}

/**
 * codeslayer_config_set_file_path:
 * @config: a #CodeSlayerConfig.
 * @name: the text to display for the config.
 */
void
codeslayer_config_set_file_path (CodeSlayerConfig *config, 
                                 const gchar      *file_path)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}

gboolean
codeslayer_config_get_projects_mode (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  return priv->projects_mode;
}

void 
codeslayer_config_set_projects_mode (CodeSlayerConfig *config, 
                                     gboolean          projects_mode)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  priv->projects_mode = projects_mode;
}

/**
 * codeslayer_config_get_projects:
 * @config: a #CodeSlayerConfig.
 *
 * Returns: The list of #CodeSlayerProject objects within the config.
 */
GList*
codeslayer_config_get_projects (CodeSlayerConfig *config)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (config)->projects;
}

/**
 * codeslayer_config_set_projects:
 * @config: a #CodeSlayerConfig.
 * @projects: the list of #CodeSlayerProject objects to add to the config.
 */
void
codeslayer_config_set_projects (CodeSlayerConfig *config, 
                                GList            *projects)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  priv->projects = projects;
  g_list_foreach (priv->projects, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_config_get_project_by_file_path:
 * @config: a #CodeSlayerConfig.
 * @file_path: the file_path of the #CodeSlayerProject to find.
 *
 * Returns: the project found by given file path. Will return NULL if the 
 *          project specified is not found.
 */
CodeSlayerProject*
codeslayer_config_get_project_by_file_path (CodeSlayerConfig *config, 
                                            const gchar      *file_path)
{
  CodeSlayerConfigPrivate *priv;
  GList *list;
  
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);

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
 * @config: a #CodeSlayerConfig.
 * @project: the #CodeSlayerProject to add to the config.
 */
void
codeslayer_config_add_project (CodeSlayerConfig  *config,
                               CodeSlayerProject *project)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  priv->projects = g_list_append (priv->projects, project);
  g_object_ref_sink (G_OBJECT (project));
}

/**
 * codeslayer_config_remove_project:
 * @config: a #CodeSlayerConfig.
 * @project: the #CodeSlayerProject to remove from the config.
 */
void
codeslayer_config_remove_project (CodeSlayerConfig  *config,
                                  CodeSlayerProject *project)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  priv->projects = g_list_remove (priv->projects, project);
  g_object_unref (project);
}

/**
 * codeslayer_config_contains_project:
 * @config: a #CodeSlayerConfig.
 * @project: the #CodeSlayerProject to check.
 */
gboolean            
codeslayer_config_contains_project (CodeSlayerConfig  *config,
                                    CodeSlayerProject *project)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  return g_list_index (priv->projects, project) != -1;
}                                   

static void
remove_all_projects (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);  
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
 * @config: a #CodeSlayerConfig.
 *
 * Returns: The list of #CodeSlayerDocuments objects within the config.
 */
GList *
codeslayer_config_get_documents (CodeSlayerConfig *config)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (config)->documents;
}

/**
 * codeslayer_config_set_documents:
 * @config: a #CodeSlayerConfig.
 * @projects: the list of #CodeSlayerDocument objects to add to the config.
 */
void
codeslayer_config_set_documents (CodeSlayerConfig *config, 
                                 GList            *documents)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  remove_all_documents (config);
  priv->documents = documents;
}

/**
 * codeslayer_config_add_document:
 * @config: a #CodeSlayerConfig.
 * @document: the #CodeSlayerDocument to add to the config.
 */
void
codeslayer_config_add_document (CodeSlayerConfig   *config,
                                CodeSlayerDocument *document)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  if (priv->documents == NULL || g_list_index (priv->documents, document) == -1)
    priv->documents = g_list_append (priv->documents, document);
}

/**
 * codeslayer_config_remove_document:
 * @config: a #CodeSlayerConfig.
 * @document: the #CodeSlayerDocument to remove from the config.
 */
void
codeslayer_config_remove_document (CodeSlayerConfig   *config,
                                   CodeSlayerDocument *document)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  priv->documents = g_list_remove (priv->documents, document);
}

static void
remove_all_documents (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  if (priv->documents)
    {
      /*g_list_foreach (priv->documents, (GFunc) g_object_unref, NULL);*/
      priv->documents = g_list_remove_all (priv->documents, NULL);
      g_list_free (priv->documents);
      priv->documents = NULL;
    }  
}

/**
 * codeslayer_config_get_plugins:
 * @config: a #CodeSlayerConfig.
 *
 * Returns: The list of #CodeSlayerPlugin plugin objects within the config. For
 *          internal use only.
 */
GList*
codeslayer_config_get_plugins (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  return priv->plugins;
}

/**
 * codeslayer_config_set_plugins:
 * @config: a #CodeSlayerConfig.
 * @plugins: the list of #CodeSlayerPlugin plugin objects to add to the config.
 */
void
codeslayer_config_set_plugins (CodeSlayerConfig *config, 
                               GList            *plugins)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  priv->plugins = plugins;
}

/**
 * codeslayer_config_contains_plugin:
 * @config: a #CodeSlayerConfig.
 * @plugin: the plugin to find.
 *
 * Returns: is TRUE if the plugin is found in the config.
 */
gboolean
codeslayer_config_contains_plugin (CodeSlayerConfig *config, 
                                   const gchar      *plugin)
{
  CodeSlayerConfigPrivate *priv;
  GList *plugins;

  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);

  plugins = priv->plugins;

  while (plugins != NULL)
    {
      gchar *lib = plugins->data;
      if (g_strcmp0 (lib, plugin) == 0)
        return TRUE;
      plugins = g_list_next (plugins);
    }
    
  return FALSE;

}                                                        

/**
 * codeslayer_config_add_plugin:
 * @config: a #CodeSlayerConfig.
 * @plugin: the plugin to add.
 */
void
codeslayer_config_add_plugin (CodeSlayerConfig *config, 
                              const gchar      *plugin)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  priv->plugins = g_list_prepend (priv->plugins, g_strdup (plugin));
}                                                        

static void
remove_all_plugins (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  if (priv->plugins)
    {
      g_list_foreach (priv->plugins, (GFunc) g_free, NULL);
      priv->plugins = g_list_remove_all (priv->plugins, NULL);
      g_list_free (priv->plugins);
      priv->plugins = NULL;
    }
}

/**
 * codeslayer_config_remove_plugin:
 * @config: a #CodeSlayerConfig.
 * @plugin: the plugin to remove.
 */
void
codeslayer_config_remove_plugin (CodeSlayerConfig *config, 
                                 const gchar      *plugin)
{
  CodeSlayerConfigPrivate *priv;
  GList *plugins;

  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);

  plugins = priv->plugins;

  while (plugins != NULL)
    {
      gchar *lib = plugins->data;
      if (g_strcmp0 (lib, plugin) == 0)
        {
          priv->plugins = g_list_remove (priv->plugins, lib);
          return;
        }
      plugins = g_list_next (plugins);
    }
}

/**
 * codeslayer_config_get_preferences:
 * @config: a #CodeSlayerConfig.
 *
 * Returns: The hashtable of preferences.
 */
GHashTable*
codeslayer_config_get_preferences (CodeSlayerConfig *config)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (config)->preferences;
}

/**
 * codeslayer_config_get_preference:
 * @config: a #CodeSlayerConfig.
 * @key: use the key to find the value from the preferences.
 *
 * Returns: The value for the preference.
 */
const gchar*
codeslayer_config_get_preference (CodeSlayerConfig *config,
                                  gchar            *key)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  return g_hash_table_lookup (priv->preferences, key);
}


/**
 * codeslayer_config_set_preference:
 * @config: a #CodeSlayerConfig.
 * @key: the key for the preference.
 * @value: the value for the preference.
 */
void
codeslayer_config_set_preference (CodeSlayerConfig *config,
                                  gchar            *key, 
                                  gchar            *value)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  g_hash_table_replace (priv->preferences, g_strdup (key), g_strdup (value));
}

static void
remove_all_preferences (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  g_hash_table_destroy (priv->preferences);
  priv->preferences = NULL;
}

/**
 * codeslayer_config_get_settings:
 * @config: a #CodeSlayerConfig.
 *
 * Returns: The hashtable of settings.
 */
GHashTable*
codeslayer_config_get_settings (CodeSlayerConfig *config)
{
  return CODESLAYER_CONFIG_GET_PRIVATE (config)->settings;
}

/**
 * codeslayer_config_get_settings:
 * @config: a #CodeSlayerConfig.
 * @key: use the key to find the value from the settings.
 *
 * Returns: The value for the setting.
 */
const gchar*
codeslayer_config_get_setting (CodeSlayerConfig *config,
                               gchar            *key)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  return g_hash_table_lookup (priv->settings, key);
}


/**
 * codeslayer_config_set_setting:
 * @config: a #CodeSlayerConfig.
 * @key: the key for the setting.
 * @value: the value for the setting.
 */
void
codeslayer_config_set_setting (CodeSlayerConfig *config,
                               gchar            *key, 
                               gchar            *value)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  g_hash_table_replace (priv->settings, g_strdup (key), g_strdup (value));
}

static void
remove_all_settings (CodeSlayerConfig *config)
{
  CodeSlayerConfigPrivate *priv;
  priv = CODESLAYER_CONFIG_GET_PRIVATE (config);
  g_hash_table_destroy (priv->settings);
  priv->settings = NULL;
}
