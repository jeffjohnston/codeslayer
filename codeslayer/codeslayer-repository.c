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

#include <stdio.h>
#include <string.h>

#include <codeslayer/codeslayer-repository.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-group.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-plugin.h>

#define GROUPS_XML "groups.xml"
#define PROJECTS_XML "projects.xml"
#define DOCUMENTS_XML "documents.xml"
#define PLUGINS_XML "plugins.xml"
#define MAIN "main"

static GList* get_groups                        (void);
static gchar* get_active_group                  (void);
static CodeSlayerPlugin* load_plugin_from_file  (gchar *file_path);
static gboolean verify_group_conf_exists        (gchar *conf_path);

CodeSlayerGroups*
codeslayer_repository_get_groups (void)
{
  CodeSlayerGroups *groups;
  GList *list;
  GList *tmp;
  gchar *active_group;
  
  groups = codeslayer_groups_new ();
  
  active_group = get_active_group ();

  list = get_groups ();
  tmp = list;
        
  while (tmp != NULL)
    {
      CodeSlayerGroup *group = tmp->data;
      codeslayer_groups_add_group (groups, group);
      
      if (g_strcmp0 (codeslayer_group_get_name (group), active_group) == 0)
        codeslayer_groups_set_active_group (groups, group);        
      
      tmp = g_list_next (tmp);
    }
  
  g_free (active_group);
  g_list_free (list);

  return groups;
}

static GList*
get_groups (void)
{
  GList* list = NULL;
  
  GFile *file;
  gchar *file_path;
  GFileEnumerator *enumerator;

  file_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, NULL);
  
  file = g_file_new_for_path (file_path);

  enumerator = g_file_enumerate_children (file, "standard::*",
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 
                                          NULL, NULL);
  if (enumerator != NULL)
    {
      GFileInfo *file_info;
      while ((file_info = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
        {
          const char *file_name;
          GFileType file_type;
          file_name = g_file_info_get_name (file_info);
          file_type = g_file_info_get_file_type (file_info);
          if (file_type == G_FILE_TYPE_DIRECTORY)
            {
              CodeSlayerGroup *group;
              group = codeslayer_group_new ();
              codeslayer_group_set_name (group, file_name);
              g_object_force_floating (G_OBJECT (group));
              list = g_list_prepend (list, group);
            }
          g_object_unref (file_info);
        }      
      g_object_unref (enumerator);
    }
    
  g_object_unref (file);
  g_free (file_path);
  
  return list;
}

static gchar*
get_active_group ()
{
  gchar *result;
  GKeyFile *keyfile;
  gchar *conf_path;

  keyfile = g_key_file_new ();

  conf_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, 
                                CODESLAYER_GROUPS_DIR, 
                                CODESLAYER_GROUPS_CONF, 
                                NULL);
                                
  g_key_file_load_from_file (keyfile, conf_path, G_KEY_FILE_NONE, NULL);
  
  if (g_key_file_has_key (keyfile, MAIN, CODESLAYER_GROUPS_ACTVIE, NULL))
    result = g_key_file_get_string (keyfile, MAIN, CODESLAYER_GROUPS_ACTVIE, NULL);
  else
    result = g_strdup ("");

  g_free (conf_path);
  g_key_file_free (keyfile);
  
  return result;
}

void
codeslayer_repository_save_groups (CodeSlayerGroups *groups)
{
  CodeSlayerGroup *group;
  const gchar *group_name;
  GKeyFile *keyfile;
  gchar *conf_path;
  gchar *data;
  gsize size;

  group = codeslayer_groups_get_active_group (groups);
  group_name = codeslayer_group_get_name (group);
  
  keyfile = g_key_file_new ();

  conf_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, 
                                CODESLAYER_GROUPS_DIR, 
                                CODESLAYER_GROUPS_CONF, 
                                NULL);
                                
  g_key_file_load_from_file (keyfile, conf_path, G_KEY_FILE_NONE, NULL);
  g_key_file_set_string (keyfile, MAIN, CODESLAYER_GROUPS_ACTVIE, group_name);

  data = g_key_file_to_data (keyfile, &size, NULL);
  g_file_set_contents (conf_path, data, size, NULL);

  g_free (conf_path);
  g_free (data);
  g_key_file_free (keyfile);
}

void
codeslayer_repository_create_group (CodeSlayerGroup *group)
{
  gchar *group_path;
  GFile *group_file;

  group_path = g_build_filename (g_get_home_dir (), 
                                 CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, 
                                 codeslayer_group_get_name (group), NULL);
  group_file = g_file_new_for_path (group_path);
  if (!g_file_query_exists (group_file, NULL))
      g_file_make_directory (group_file, NULL, NULL);

  g_free (group_path);
  g_object_unref (group_file);
}

void
codeslayer_repository_delete_group (CodeSlayerGroup *group)
{
  gchar *group_path;
  GFile *group_file;

  group_path = g_build_filename (g_get_home_dir (), 
                                 CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, 
                                 codeslayer_group_get_name (group), NULL);
  group_file = g_file_new_for_path (group_path);
  if (g_file_query_exists (group_file, NULL))
    codeslayer_utils_file_delete (group_file, NULL);

  g_free (group_path);
  g_object_unref (group_file);
}

void
codeslayer_repository_rename_group (CodeSlayerGroup *group, 
                                    const gchar     *name)
{
  gchar *source_path;
  GFile *source_file;
  gchar *destination_path;
  GFile *destination_file;

  source_path = g_build_filename (g_get_home_dir (), 
                                  CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, 
                                  codeslayer_group_get_name (group), NULL);
  
  destination_path = g_build_filename (g_get_home_dir (), 
                                 CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, 
                                 name, NULL);
  
  source_file = g_file_new_for_path (source_path);
  destination_file = g_file_new_for_path (destination_path);
  
  g_file_move (source_file, destination_file, 
               G_FILE_COPY_NONE, NULL, NULL, NULL, NULL);

  g_free (source_path);
  g_object_unref (source_file);
  g_free (destination_path);
  g_object_unref (destination_file);
}

GList*
codeslayer_repository_get_libs (CodeSlayerGroup *group)
{
  GList *list = NULL;
  gchar *libs = NULL;
  GKeyFile *keyfile;
  gchar *conf_path;
  const gchar *group_name;

  keyfile = g_key_file_new ();

  group_name = codeslayer_group_get_name (group);

  conf_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, 
                                CODESLAYER_GROUPS_DIR, 
                                group_name,
                                CODESLAYER_GROUP_CONF, 
                                NULL);
                                
  verify_group_conf_exists (conf_path);
                                
  g_key_file_load_from_file (keyfile, conf_path, G_KEY_FILE_NONE, NULL);
  
  if (g_key_file_has_key (keyfile, MAIN, CODESLAYER_GROUP_LIBS, NULL))
    libs = g_key_file_get_string (keyfile, MAIN, CODESLAYER_GROUP_LIBS, NULL);

  g_free (conf_path);
  g_key_file_free (keyfile);
  
  if (libs != NULL)
    {
      list = codeslayer_utils_string_to_list (libs);
      g_free (libs);
    }
  
  return list;
}

void
codeslayer_repository_save_libs (CodeSlayerGroup *group)
{
  const gchar *group_name;
  GList *group_libs;
  GKeyFile *keyfile;
  gchar *conf_path;
  gchar *value;
  gchar *data;
  gsize size;

  group_name = codeslayer_group_get_name (group);
  group_libs = codeslayer_group_get_libs (group);
  
  value = codeslayer_utils_list_to_string (group_libs);
  
  keyfile = g_key_file_new ();

  conf_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, 
                                CODESLAYER_GROUPS_DIR, 
                                group_name,
                                CODESLAYER_GROUP_CONF, 
                                NULL);

  verify_group_conf_exists (conf_path);
                                
  g_key_file_load_from_file (keyfile, conf_path, G_KEY_FILE_NONE, NULL);
  g_key_file_set_string (keyfile, MAIN, CODESLAYER_GROUP_LIBS, value);

  data = g_key_file_to_data (keyfile, &size, NULL);
  g_file_set_contents (conf_path, data, size, NULL);

  g_free (conf_path);
  g_free (value);
  g_free (data);
  g_key_file_free (keyfile);
}

static gboolean
verify_group_conf_exists (gchar *conf_path)
{
  gboolean result = TRUE;
  GFile *conf_file;

  conf_file = g_file_new_for_path (conf_path);
  if (!g_file_query_exists (conf_file, NULL))
    {
      GFileIOStream *stream;
      stream = g_file_create_readwrite (conf_file, G_FILE_CREATE_NONE, 
                                        NULL, NULL);
      g_io_stream_close (G_IO_STREAM (stream), NULL, NULL);
      g_object_unref (stream);
      result = FALSE;
    }

  g_object_unref (conf_file);

  return result;
}

GList*
codeslayer_repository_get_projects (CodeSlayerGroup *group)
{
  GList *gobjects;
  gchar *file_path;

  file_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, 
                                codeslayer_group_get_name (group),
                                PROJECTS_XML, NULL);

  gobjects = codeslayer_utils_get_gobjects (CODESLAYER_PROJECT_TYPE,
                                            TRUE,
                                            file_path, 
                                            "project",
                                            "name", G_TYPE_STRING, 
                                            "key", G_TYPE_STRING, 
                                            "folder_path", G_TYPE_STRING, 
                                            NULL);

  g_free (file_path);
  
  return gobjects;
}

void
codeslayer_repository_save_projects (CodeSlayerGroup *group)
{
  GList *projects;
  gchar *file_path;

  projects = codeslayer_group_get_projects (group);
  
  file_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, 
                                codeslayer_group_get_name (group),
                                PROJECTS_XML, NULL);

  codeslayer_utils_save_gobjects (projects, 
                                  file_path, 
                                  "project",
                                  "name", G_TYPE_STRING, 
                                  "key", G_TYPE_STRING, 
                                  "folder_path", G_TYPE_STRING, 
                                  NULL);

  g_free (file_path);
}

GList*
codeslayer_repository_get_documents (CodeSlayerGroup *group)
{
  GList *gobjects;  
  GList *tmp;  
  gchar *file_path;
  
  file_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, 
                                codeslayer_group_get_name (group),
                                DOCUMENTS_XML, NULL);

  gobjects = codeslayer_utils_get_gobjects (CODESLAYER_DOCUMENT_TYPE,
                                            FALSE,
                                            file_path, 
                                            "document",
                                            "project_key", G_TYPE_STRING, 
                                            "file_path", G_TYPE_STRING, 
                                            "line_number", G_TYPE_INT, 
                                            NULL);
     
  tmp = gobjects;

  while (tmp != NULL)
    {
      CodeSlayerDocument *document = tmp->data;
      CodeSlayerProject *project;
      gchar *project_key;
      g_object_get (document, "project_key", &project_key, NULL);
      project = codeslayer_group_find_project (group, project_key);
      if (project != NULL) /*here for backwards compatibility with file format change*/
        {
          codeslayer_document_set_project (document, project);
          g_free (project_key);        
        }
      tmp = g_list_next (tmp);
    }
                                            
  g_free (file_path);                                  
  
  return gobjects;
}

void
codeslayer_repository_save_documents (CodeSlayerGroup *group, 
                                      GList           *documents)
{
  gchar *file_path;

  file_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, CODESLAYER_GROUPS_DIR, 
                                codeslayer_group_get_name (group),
                                DOCUMENTS_XML, NULL);

  codeslayer_utils_save_gobjects (documents, 
                                  file_path, 
                                  "document",
                                  "project_key", G_TYPE_STRING, 
                                  "file_path", G_TYPE_STRING, 
                                  "line_number", G_TYPE_INT, 
                                  NULL);

  g_free (file_path);
}

GList*
codeslayer_repository_get_plugins (GObject *data)
{
  GList *plugins = NULL;  
  GFile *file;
  char *plugins_dir;
  GFileEnumerator *enumerator;
  
  plugins_dir = g_build_filename (g_get_home_dir (), 
                                  CODESLAYER_HOME, CODESLAYER_PLUGINS_DIR, NULL);

  file = g_file_new_for_path (plugins_dir);

  enumerator = g_file_enumerate_children (file, "standard::*",
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 
                                          NULL, NULL);
  if (enumerator != NULL)
    {
      GFileInfo *file_info;
      while ((file_info = g_file_enumerator_next_file (enumerator, 
                                                       NULL, NULL)) != NULL)
        {
          GFileType file_type;
          const char *file_name;          

          if (g_file_info_get_is_hidden (file_info))
            {
              g_object_unref (file_info);
              continue;
            }

          file_type = g_file_info_get_file_type (file_info);
          file_name = g_file_info_get_name (file_info);

          if (file_type == G_FILE_TYPE_REGULAR &&
              g_str_has_suffix (file_name, ".codeslayer-plugin"))
            {
              CodeSlayerPlugin *plugin;
              gchar *file_path;
              file_path = g_build_filename (plugins_dir, file_name, NULL);
              plugin = load_plugin_from_file (file_path);
              codeslayer_plugin_set_data (plugin, data);
              plugins = g_list_prepend (plugins, plugin);

              g_free (file_path);
            }

          g_object_unref (file_info);
        }
      g_object_unref (enumerator);
    }

  g_object_unref (file);
  g_free (plugins_dir);
  
  return plugins;
}

static CodeSlayerPlugin*
load_plugin_from_file (gchar *file_path)
{
  CodeSlayerPlugin *plugin;
  GKeyFile *keyfile;
  gchar *value;

  keyfile = g_key_file_new ();
  g_key_file_load_from_file (keyfile, file_path, G_KEY_FILE_NONE, NULL);
  
  plugin = codeslayer_plugin_new ();
  
  value = g_key_file_get_string (keyfile, "plugin", "lib", NULL);
  codeslayer_plugin_set_lib (plugin, value);
  g_free (value);
  
  value = g_key_file_get_string (keyfile, "plugin", "version", NULL);
  codeslayer_plugin_set_version (plugin, value);
  g_free (value);
  
  value = g_key_file_get_string (keyfile, "plugin", "name", NULL);
  codeslayer_plugin_set_name (plugin, value);
  g_free (value);
  
  value = g_key_file_get_string (keyfile, "plugin", "description", NULL);
  codeslayer_plugin_set_description (plugin, value);
  g_free (value);
  
  value = g_key_file_get_string (keyfile, "plugin", "authors", NULL);
  codeslayer_plugin_set_authors (plugin, value);
  g_free (value);
  
  value = g_key_file_get_string (keyfile, "plugin", "copyright", NULL);
  codeslayer_plugin_set_copyright (plugin, value);
  g_free (value);
  
  value = g_key_file_get_string (keyfile, "plugin", "website", NULL);
  codeslayer_plugin_set_website (plugin, value);
  g_free (value);
  
  g_key_file_free (keyfile);
  
  return plugin;
}
