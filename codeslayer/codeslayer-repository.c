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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-repository.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-plugin.h>

#define CONFIG "codeslayer.config"

static void load_config                                   (CodeSlayerConfig *config, 
                                                           xmlNode          *a_node);
static CodeSlayerPlugin* load_plugin_from_file            (gchar            *file_path);
static void              set_config_preferences_defaults  (CodeSlayerConfig *config);

CodeSlayerConfig*
codeslayer_repository_get_default_config ()
{
  CodeSlayerConfig *config;
  gchar *file_path;
  GFile *file;
  
  file_path = g_build_filename (g_get_home_dir (), CODESLAYER_HOME, CONFIG, NULL);

  file = g_file_new_for_path (file_path);
  if (!g_file_query_exists (file, NULL))
    {
      config = codeslayer_config_new ();      
      codeslayer_config_set_file_path (config, file_path);      
      set_config_preferences_defaults (config);
    }
  else
    {
      config = codeslayer_repository_get_config (file);
    }    
    
  g_object_unref (file);    
                                
  return config;
}

static void
set_config_preferences_defaults (CodeSlayerConfig *config)
{
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_DISPLAY_LINE_NUMBERS, "true");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_CURRENT_LINE, "true");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_DISPLAY_RIGHT_MARGIN, "true");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_MATCHING_BRACKET, "true");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_INSERT_SPACES_INSTEAD_OF_TABS, "true");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_ENABLE_AUTOMATIC_INDENTATION, "true");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_RIGHT_MARGIN_POSITION, "80");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_TAB_WIDTH, "2");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_FONT, "Monospace 9");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_THEME, "classic");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_TAB_POSITION, "top");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_SIDE_PANE_TAB_POSITION, "top");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_BOTTOM_PANE_TAB_POSITION, "left");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_PROJECTS_EXCLUDE_DIRS, ".csv,.git,.svn");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_WORD_WRAP_TYPES, ".txt");
}

CodeSlayerConfig*
codeslayer_repository_get_config (GFile *file)
{
  CodeSlayerConfig *config;
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  gchar *file_path = NULL;
  
  file_path = g_file_get_path (file);
  
  doc = xmlReadFile (file_path, NULL, 0);
  if (doc == NULL) 
    {
      g_warning ("could not parse projects file %s\n", file_path);
      xmlCleanupParser();
      return NULL;
    }

  config = codeslayer_config_new ();
  
  root_element = xmlDocGetRootElement (doc);

  load_config (config, root_element);
  codeslayer_config_set_file_path (config, file_path);

  xmlFreeDoc (doc);
  xmlCleanupParser ();
  
  return config;
}

static void
load_config (CodeSlayerConfig *config, 
             xmlNode          *a_node)
{
  xmlNode *cur_node = NULL;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
    {
      if (cur_node->type == XML_ELEMENT_NODE)
        {
          if (g_strcmp0 ((gchar*)cur_node->name, "project") == 0)
            {
              CodeSlayerProject *project;
              xmlChar *name;
              xmlChar *folder_path;
              
              name = xmlGetProp (cur_node, (const xmlChar*)"name");
              folder_path = xmlGetProp (cur_node, (const xmlChar*)"folder_path");
              
              project = codeslayer_project_new ();
              codeslayer_project_set_name (project, (gchar*) name);
              codeslayer_project_set_folder_path (project, (gchar*) folder_path);
              codeslayer_config_add_project (config, project);
              
              xmlFree (name);
              xmlFree (folder_path);
            }
          else if (g_strcmp0 ((gchar*)cur_node->name, "document") == 0)
            {
              CodeSlayerProject *project;
              CodeSlayerDocument *document;
              xmlChar *file_path;
              xmlChar *line_number;
              
              file_path = xmlGetProp (cur_node, (const xmlChar*)"file_path");
              line_number = xmlGetProp (cur_node, (const xmlChar*)"line_number");
              
              document = codeslayer_document_new ();
              codeslayer_document_set_file_path (document, (gchar*) file_path);
              codeslayer_document_set_line_number (document, atoi ((gchar*) line_number));
              codeslayer_config_add_document (config, document);
              
              project = codeslayer_config_get_project_by_file_path (config, (gchar*) file_path);
              codeslayer_document_set_project (document, project);
              
              xmlFree (file_path);
              xmlFree (line_number);
            }
          else if (g_strcmp0 ((gchar*)cur_node->name, "preference") == 0)
            {
              xmlChar *name;
              xmlChar *value;
              
              name = xmlGetProp (cur_node, (const xmlChar*)"name");
              value = xmlGetProp (cur_node, (const xmlChar*)"value");
              
              codeslayer_config_set_preference (config, (gchar*) name, (gchar*) value);
              
              xmlFree (name);
              xmlFree (value);
            }
          else if (g_strcmp0 ((gchar*)cur_node->name, "plugin") == 0)
            {
              xmlChar *name;
              name = xmlGetProp (cur_node, (const xmlChar*)"name");

              codeslayer_config_add_plugin (config, (gchar*) name);
              xmlFree (name);
            }
        }
      load_config (config, cur_node->children);
    }
}

void build_projects_xml (CodeSlayerProject *project,
                         GString           **xml)
{
  const gchar *name;
  const gchar *folder_path;
  
  name = codeslayer_project_get_name (project);
  folder_path = codeslayer_project_get_folder_path (project);

  *xml = g_string_append (*xml, "\n\t\t<project ");
  *xml = g_string_append (*xml, "name=\"");
  *xml = g_string_append (*xml, name);
  *xml = g_string_append (*xml, "\" ");
  *xml = g_string_append (*xml, "folder_path=\"");
  *xml = g_string_append (*xml, folder_path);
  *xml = g_string_append (*xml, "\"/>");
}

void build_documents_xml (CodeSlayerDocument *document,
                          GString            **xml)
{
  const gchar *file_path;
  gint line_number;
  gchar *line_number_str;
  
  file_path = codeslayer_document_get_file_path (document);
  line_number = codeslayer_document_get_line_number (document);
  line_number_str = g_strdup_printf ("%d", line_number);

  *xml = g_string_append (*xml, "\n\t\t<document ");
  *xml = g_string_append (*xml, "file_path=\"");
  *xml = g_string_append (*xml, file_path);
  *xml = g_string_append (*xml, "\" ");
  *xml = g_string_append (*xml, "line_number=\"");
  *xml = g_string_append (*xml, line_number_str);
  *xml = g_string_append (*xml, "\"/>");
  
  g_free (line_number_str);
}

void build_plugins_xml (gchar   *name, 
                     GString **xml)
{
  *xml = g_string_append (*xml, "\n\t\t<plugin ");
  *xml = g_string_append (*xml, "name=\"");
  *xml = g_string_append (*xml, name);
  *xml = g_string_append (*xml, "\"/>");
}

void build_preferences_xml (gchar   *name,
                            gchar   *value, 
                            GString **xml)
{
  *xml = g_string_append (*xml, "\n\t\t<preference ");
  *xml = g_string_append (*xml, "name=\"");
  *xml = g_string_append (*xml, name);
  *xml = g_string_append (*xml, "\" ");
  *xml = g_string_append (*xml, "value=\"");
  *xml = g_string_append (*xml, value);
  *xml = g_string_append (*xml, "\"/>");
}

void
codeslayer_repository_save_config (CodeSlayerConfig *config)
{
  GString *xml;
  gchar *contents;
  const gchar *file_path;
  
  GList *projects;
  GList *documents;
  GList *plugins;
  GHashTable *preferences;
  
  projects = codeslayer_config_get_projects (config);         
  documents = codeslayer_config_get_documents (config);         
  plugins = codeslayer_config_get_plugins (config);         
  preferences = codeslayer_config_get_preferences (config);
  
  xml = g_string_new ("<config>");
  
  if (projects != NULL)
    {
      xml = g_string_append (xml, "\n\t<projects>");
      g_list_foreach (projects, (GFunc)build_projects_xml, &xml);
      xml = g_string_append (xml, "\n\t</projects>");    
    }

  if (documents != NULL)
    {
      xml = g_string_append (xml, "\n\t<documents>");
      g_list_foreach (documents, (GFunc)build_documents_xml, &xml);
      xml = g_string_append (xml, "\n\t</documents>");    
    }

  if (plugins != NULL)
    {
      xml = g_string_append (xml, "\n\t<plugins>");
      g_list_foreach (plugins, (GFunc)build_plugins_xml, &xml);
      xml = g_string_append (xml, "\n\t</plugins>");    
    }

  xml = g_string_append (xml, "\n\t<preferences>");
  g_hash_table_foreach (preferences, (GHFunc)build_preferences_xml, &xml);
  xml = g_string_append (xml, "\n\t</preferences>");

  xml = g_string_append (xml, "\n</config>");
  
  contents = g_string_free (xml, FALSE);
  
  file_path = codeslayer_config_get_file_path (config);

  g_file_set_contents (file_path, contents, -1, NULL);
  
  g_free (contents);
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
