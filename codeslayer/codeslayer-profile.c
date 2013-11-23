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

#include <codeslayer/codeslayer-profile.h>

/**
 * SECTION:codeslayer-profile
 * @short_description: Contains the projects.
 * @title: CodeSlayerProfile
 * @include: codeslayer/codeslayer-profile.h
 */

static void codeslayer_profile_class_init  (CodeSlayerProfileClass *klass);
static void codeslayer_profile_init        (CodeSlayerProfile      *profile);
static void codeslayer_profile_finalize    (CodeSlayerProfile      *profile);

static void remove_all_projects            (CodeSlayerProfile      *profile);
static void remove_all_documents           (CodeSlayerProfile      *profile);
static void remove_all_plugins             (CodeSlayerProfile      *profile);

#define CODESLAYER_PROFILE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROFILE_TYPE, CodeSlayerProfilePrivate))

typedef struct _CodeSlayerProfilePrivate CodeSlayerProfilePrivate;

struct _CodeSlayerProfilePrivate
{
  gchar              *file_path;
  gchar              *name;
  GList              *projects;
  GList              *documents;
  GList              *plugins;
  CodeSlayerRegistry *registry;
};

G_DEFINE_TYPE (CodeSlayerProfile, codeslayer_profile, G_TYPE_OBJECT)
     
static void 
codeslayer_profile_class_init (CodeSlayerProfileClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_profile_finalize;

  g_type_class_add_private (klass, sizeof (CodeSlayerProfilePrivate));                                                      
}

static void
codeslayer_profile_init (CodeSlayerProfile *profile)
{
  CodeSlayerProfilePrivate *priv; 
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  priv->file_path = NULL;
  priv->name = NULL;
  priv->projects = NULL;
  priv->documents = NULL;
  priv->plugins = NULL;
  priv->registry = NULL;
}

static void
codeslayer_profile_finalize (CodeSlayerProfile *profile)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  
  if (priv->name)
    g_free (priv->name);    
  
  if (priv->file_path)
    g_free (priv->file_path);

  remove_all_projects (profile);
  remove_all_documents (profile);
  remove_all_plugins (profile);
  g_object_unref (priv->registry);

  G_OBJECT_CLASS (codeslayer_profile_parent_class)->finalize (G_OBJECT (profile));
}

/**
 * codeslayer_profile_new:
 *
 * Creates a new #CodeSlayerProfile.
 *
 * Returns: a new #CodeSlayerProfile. 
 */
CodeSlayerProfile*
codeslayer_profile_new (void)
{
  CodeSlayerProfilePrivate *priv;
  CodeSlayerProfile *profile;

  profile = CODESLAYER_PROFILE (g_object_new (codeslayer_profile_get_type (), NULL));
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  
  priv->registry = codeslayer_registry_new ();

  return profile;
}

/**
 * codeslayer_profile_get_name:
 * @profile: a #CodeSlayerProfile.
 *
 * Returns: the text to display for the profile.
 */
const gchar*
codeslayer_profile_get_name (CodeSlayerProfile  *profile)
{
  return CODESLAYER_PROFILE_GET_PRIVATE (profile)->name;
}

/**
 * codeslayer_profile_set_name:
 * @profile: a #CodeSlayerProfile.
 * @name: the name of the profile.
 *
 * Returns: the text to display for the profile.
 */
void
codeslayer_profile_set_name (CodeSlayerProfile *profile,
                             gchar             *name)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

/**
 * codeslayer_profile_get_file_path:
 * @profile: a #CodeSlayerProfile.
 *
 * Returns: the path to the profiles file.
 */
const gchar*
codeslayer_profile_get_file_path (CodeSlayerProfile *profile)
{
  return CODESLAYER_PROFILE_GET_PRIVATE (profile)->file_path;
}

/**
 * codeslayer_profile_set_file_path:
 * @profile: a #CodeSlayerProfile.
 * @file_path: the path to the profiles file.
 */
void
codeslayer_profile_set_file_path (CodeSlayerProfile *profile, 
                                  const gchar      *file_path)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}

gboolean
codeslayer_profile_get_enable_projects (CodeSlayerProfile *profile)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  return codeslayer_registry_get_boolean (priv->registry, CODESLAYER_REGISTRY_ENABLE_PROJECTS);
}

void 
codeslayer_profile_set_enable_projects (CodeSlayerProfile *profile, 
                                        gboolean           enable_projects)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  codeslayer_registry_set_boolean (priv->registry, 
                                   CODESLAYER_REGISTRY_ENABLE_PROJECTS, enable_projects);
}

/**
 * codeslayer_profile_get_projects:
 * @profile: a #CodeSlayerProfile.
 *
 * Returns: The list of #CodeSlayerProject objects within the profile.
 */
GList*
codeslayer_profile_get_projects (CodeSlayerProfile *profile)
{
  return CODESLAYER_PROFILE_GET_PRIVATE (profile)->projects;
}

/**
 * codeslayer_profile_set_projects:
 * @profile: a #CodeSlayerProfile.
 * @projects: the list of #CodeSlayerProject objects to add to the profile.
 */
void
codeslayer_profile_set_projects (CodeSlayerProfile *profile, 
                                 GList             *projects)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  remove_all_projects (profile);
  priv->projects = projects;
  g_list_foreach (priv->projects, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_profile_get_project_by_file_path:
 * @profile: a #CodeSlayerProfile.
 * @file_path: the file_path of the #CodeSlayerProject to find.
 *
 * Returns: the project found by given file path. Will return NULL if the 
 *          project specified is not found.
 */
CodeSlayerProject*
codeslayer_profile_get_project_by_file_path (CodeSlayerProfile *profile, 
                                             const gchar       *file_path)
{
  CodeSlayerProfilePrivate *priv;
  GList *list;
  
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);

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
 * codeslayer_profile_add_project:
 * @profile: a #CodeSlayerProfile.
 * @project: the #CodeSlayerProject to add to the profile.
 */
void
codeslayer_profile_add_project (CodeSlayerProfile *profile,
                                CodeSlayerProject *project)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  priv->projects = g_list_append (priv->projects, project);
  g_object_ref_sink (G_OBJECT (project));
}

/**
 * codeslayer_profile_remove_project:
 * @profile: a #CodeSlayerProfile.
 * @project: the #CodeSlayerProject to remove from the profile.
 */
void
codeslayer_profile_remove_project (CodeSlayerProfile *profile,
                                   CodeSlayerProject *project)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  priv->projects = g_list_remove (priv->projects, project);
  g_object_unref (project);
}

/**
 * codeslayer_profile_contains_project:
 * @profile: a #CodeSlayerProfile.
 * @project: the #CodeSlayerProject to check.
 */
gboolean            
codeslayer_profile_contains_project (CodeSlayerProfile  *profile,
                                    CodeSlayerProject *project)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  return g_list_index (priv->projects, project) != -1;
}                                   

static void
remove_all_projects (CodeSlayerProfile *profile)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);  
  if (priv->projects)
    {
      g_list_foreach (priv->projects, (GFunc) g_object_unref, NULL);
      priv->projects = g_list_remove_all (priv->projects, NULL);
      g_list_free (priv->projects);
      priv->projects = NULL;
    }
}

/**
 * codeslayer_profile_get_documents:
 * @profile: a #CodeSlayerProfile.
 *
 * Returns: The list of #CodeSlayerDocuments objects within the profile.
 */
GList *
codeslayer_profile_get_documents (CodeSlayerProfile *profile)
{
  return CODESLAYER_PROFILE_GET_PRIVATE (profile)->documents;
}

/**
 * codeslayer_profile_set_documents:
 * @profile: a #CodeSlayerProfile.
 * @projects: the list of #CodeSlayerDocument objects to add to the profile.
 */
void
codeslayer_profile_set_documents (CodeSlayerProfile *profile, 
                                  GList             *documents)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  remove_all_documents (profile);
  priv->documents = documents;
  g_list_foreach (priv->projects, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_profile_add_document:
 * @profile: a #CodeSlayerProfile.
 * @document: the #CodeSlayerDocument to add to the profile.
 */
void
codeslayer_profile_add_document (CodeSlayerProfile   *profile,
                                 CodeSlayerDocument *document)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  priv->documents = g_list_append (priv->documents, document);
  /*g_object_ref_sink (G_OBJECT (document));*/
}

/**
 * codeslayer_profile_remove_document:
 * @profile: a #CodeSlayerProfile.
 * @document: the #CodeSlayerDocument to remove from the profile.
 */
void
codeslayer_profile_remove_document (CodeSlayerProfile  *profile,
                                    CodeSlayerDocument *document)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  priv->documents = g_list_remove (priv->documents, document);
  /*g_object_unref (document);*/
}

static void
remove_all_documents (CodeSlayerProfile *profile)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  if (priv->documents)
    {
      /*g_list_foreach (priv->documents, (GFunc) g_object_unref, NULL);*/
      priv->documents = g_list_remove_all (priv->documents, NULL);
      g_list_free (priv->documents);
      priv->documents = NULL;
    }  
}

/**
 * codeslayer_profile_get_plugins:
 * @profile: a #CodeSlayerProfile.
 *
 * Returns: The list of #CodeSlayerPlugin plugin objects within the profile. For
 *          internal use only.
 */
GList*
codeslayer_profile_get_plugins (CodeSlayerProfile *profile)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  return priv->plugins;
}

/**
 * codeslayer_profile_set_plugins:
 * @profile: a #CodeSlayerProfile.
 * @plugins: the list of #CodeSlayerPlugin plugin objects to add to the profile.
 */
void
codeslayer_profile_set_plugins (CodeSlayerProfile *profile, 
                                GList             *plugins)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  priv->plugins = plugins;
}

/**
 * codeslayer_profile_contains_plugin:
 * @profile: a #CodeSlayerProfile.
 * @plugin: the plugin to find.
 *
 * Returns: is TRUE if the plugin is found in the profile.
 */
gboolean
codeslayer_profile_contains_plugin (CodeSlayerProfile *profile, 
                                    const gchar       *plugin)
{
  CodeSlayerProfilePrivate *priv;
  GList *plugins;

  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);

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
 * codeslayer_profile_add_plugin:
 * @profile: a #CodeSlayerProfile.
 * @plugin: the plugin to add.
 */
void
codeslayer_profile_add_plugin (CodeSlayerProfile *profile, 
                               const gchar       *plugin)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  priv->plugins = g_list_prepend (priv->plugins, g_strdup (plugin));
}                                                        

static void
remove_all_plugins (CodeSlayerProfile *profile)
{
  CodeSlayerProfilePrivate *priv;
  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);
  if (priv->plugins)
    {
      g_list_foreach (priv->plugins, (GFunc) g_free, NULL);
      priv->plugins = g_list_remove_all (priv->plugins, NULL);
      g_list_free (priv->plugins);
      priv->plugins = NULL;
    }
}

/**
 * codeslayer_profile_remove_plugin:
 * @profile: a #CodeSlayerProfile.
 * @plugin: the plugin to remove.
 */
void
codeslayer_profile_remove_plugin (CodeSlayerProfile *profile, 
                                  const gchar       *plugin)
{
  CodeSlayerProfilePrivate *priv;
  GList *plugins;

  priv = CODESLAYER_PROFILE_GET_PRIVATE (profile);

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
 * codeslayer_profile_get_registry:
 * @profile: a #CodeSlayerProfile.
 *
 * Returns: The #CodeSlayerRegistry* for the profile.
 */
CodeSlayerRegistry*
codeslayer_profile_get_registry (CodeSlayerProfile *profile)
{
  return CODESLAYER_PROFILE_GET_PRIVATE (profile)->registry;
}
