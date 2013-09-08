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

#include <codeslayer/codeslayer-settings.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-config-handler.h>

#define CONFIG "codeslayer.config"

/**
 * SECTION:codeslayer-config_handler
 * @short_description: Contains the projects.
 * @title: CodeSlayerConfigHandler
 * @include: codeslayer/codeslayer-config_handler.h
 */

static void codeslayer_config_handler_class_init  (CodeSlayerConfigHandlerClass *klass);
static void codeslayer_config_handler_init        (CodeSlayerConfigHandler      *config_handler);
static void codeslayer_config_handler_finalize    (CodeSlayerConfigHandler      *config_handler);

static void load_config                           (CodeSlayerConfig             *config, 
                                                   xmlNode                      *a_node);
static void set_config_preferences_defaults       (CodeSlayerConfig             *config);
static void set_config_settings_defaults          (CodeSlayerConfig             *config);

static void build_projects_xml                    (CodeSlayerProject            *project,
                                                   GString                      **xml);
static void build_documents_xml                   (CodeSlayerDocument           *document,
                                                   GString                      **xml);
static void build_plugins_xml                     (gchar                        *name, 
                                                   GString                      **xml);
static void build_settings_xml                    (gchar                        *name,
                                                   gchar                        *value, 
                                                   GString                      **xml);
static void build_preferences_xml                 (gchar                        *name,
                                                   gchar                        *value, 
                                                   GString                      **xml);

#define CODESLAYER_CONFIG_HANDLER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_CONFIG_HANDLER_TYPE, CodeSlayerConfigHandlerPrivate))

typedef struct _CodeSlayerConfigHandlerPrivate CodeSlayerConfigHandlerPrivate;

struct _CodeSlayerConfigHandlerPrivate
{
  CodeSlayerConfig *config;  
};

G_DEFINE_TYPE (CodeSlayerConfigHandler, codeslayer_config_handler, G_TYPE_OBJECT)
     
static void 
codeslayer_config_handler_class_init (CodeSlayerConfigHandlerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_config_handler_finalize;

  g_type_class_add_private (klass, sizeof (CodeSlayerConfigHandlerPrivate));                                                      
}

static void
codeslayer_config_handler_init (CodeSlayerConfigHandler *config_handler)
{
}

static void
codeslayer_config_handler_finalize (CodeSlayerConfigHandler *config_handler)
{
  CodeSlayerConfigHandlerPrivate *priv; 
  priv = CODESLAYER_CONFIG_HANDLER_GET_PRIVATE (config_handler);  
  g_object_unref (priv->config);
  G_OBJECT_CLASS (codeslayer_config_handler_parent_class)->finalize (G_OBJECT (config_handler));
}

/**
 * codeslayer_config_handler_new:
 *
 * Creates a new #CodeSlayerConfigHandler.
 *
 * Returns: a new #CodeSlayerConfigHandler. 
 */
CodeSlayerConfigHandler*
codeslayer_config_handler_new (void)
{
  CodeSlayerConfigHandler *config_handler;
  config_handler = CODESLAYER_CONFIG_HANDLER (g_object_new (codeslayer_config_handler_get_type (), NULL));
  return config_handler;
}

CodeSlayerConfig*
codeslayer_config_handler_get_config (CodeSlayerConfigHandler *config_handler)
{
  CodeSlayerConfigHandlerPrivate *priv; 
  priv = CODESLAYER_CONFIG_HANDLER_GET_PRIVATE (config_handler);  
  return priv->config;
}

CodeSlayerConfig*       
codeslayer_config_handler_load_new_config (CodeSlayerConfigHandler *config_handler, 
                                           GFile                   *file)
{
  CodeSlayerConfigHandlerPrivate *priv;
  gchar *file_path;

  priv = CODESLAYER_CONFIG_HANDLER_GET_PRIVATE (config_handler);
  
  file_path = g_file_get_path (file);
  
  priv->config = codeslayer_config_new ();      

  codeslayer_config_set_file_path (priv->config, file_path);
  set_config_preferences_defaults (priv->config);
  set_config_settings_defaults (priv->config);
  
  g_free (file_path);
  
  return priv->config;
}

CodeSlayerConfig*       
codeslayer_config_handler_load_default_config (CodeSlayerConfigHandler *config_handler)
{
  CodeSlayerConfigHandlerPrivate *priv; 
  gchar *file_path;
  GFile *file;
  
  priv = CODESLAYER_CONFIG_HANDLER_GET_PRIVATE (config_handler);  
  
  file_path = g_build_filename (g_get_home_dir (), CODESLAYER_HOME, CONFIG, NULL);

  file = g_file_new_for_path (file_path);
  if (!g_file_query_exists (file, NULL))
    {
      priv->config = codeslayer_config_new ();      
      codeslayer_config_set_file_path (priv->config, file_path);
      set_config_preferences_defaults (priv->config);
      set_config_settings_defaults (priv->config);
    }
  else
    {
      priv->config = codeslayer_config_handler_load_file_config (config_handler, file);
    }    
    
  g_free (file_path);    
  g_object_unref (file);    
                                
  return priv->config;
}

CodeSlayerConfig*
codeslayer_config_handler_load_file_config (CodeSlayerConfigHandler *config_handler, 
                                            GFile                   *file)
{
  CodeSlayerConfigHandlerPrivate *priv;
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  gchar *file_path = NULL;
  
  priv = CODESLAYER_CONFIG_HANDLER_GET_PRIVATE (config_handler);
  
  if (priv->config)
    g_object_unref (priv->config);
  
  file_path = g_file_get_path (file);
  
  doc = xmlReadFile (file_path, NULL, 0);
  if (doc == NULL)
    {
      g_warning ("could not parse projects file %s\n", file_path);
      xmlCleanupParser();
      return NULL;
    }

  priv->config = codeslayer_config_new ();
  
  root_element = xmlDocGetRootElement (doc);

  load_config (priv->config, root_element);
  codeslayer_config_set_file_path (priv->config, file_path);

  xmlFreeDoc (doc);
  xmlCleanupParser ();
  
  return priv->config;
}

void
codeslayer_config_handler_save_config (CodeSlayerConfigHandler *config_handler)
{
  CodeSlayerConfigHandlerPrivate *priv; 
  GString *xml;
  gchar *contents;
  const gchar *file_path;
  GList *projects;
  GList *documents;
  GList *plugins;
  GHashTable *preferences;
  GHashTable *settings;
  
  priv = CODESLAYER_CONFIG_HANDLER_GET_PRIVATE (config_handler);  

  projects = codeslayer_config_get_projects (priv->config);         
  documents = codeslayer_config_get_documents (priv->config);         
  plugins = codeslayer_config_get_plugins (priv->config);         
  preferences = codeslayer_config_get_preferences (priv->config);
  settings = codeslayer_config_get_settings (priv->config);
  
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

  xml = g_string_append (xml, "\n\t<settings>");
  g_hash_table_foreach (settings, (GHFunc)build_settings_xml, &xml);
  xml = g_string_append (xml, "\n\t</settings>");

  xml = g_string_append (xml, "\n</config>");
  
  contents = g_string_free (xml, FALSE);
  
  file_path = codeslayer_config_get_file_path (priv->config);

  g_file_set_contents (file_path, contents, -1, NULL);
  
  g_free (contents);
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
              
              /*g_object_force_floating (G_OBJECT (document));*/
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
              
              name = xmlGetProp (cur_node, (const xmlChar*)"key");
              value = xmlGetProp (cur_node, (const xmlChar*)"value");
              
              codeslayer_config_set_preference (config, (gchar*) name, (gchar*) value);
              
              xmlFree (name);
              xmlFree (value);
            }
          else if (g_strcmp0 ((gchar*)cur_node->name, "setting") == 0)
            {
              xmlChar *name;
              xmlChar *value;
              
              name = xmlGetProp (cur_node, (const xmlChar*)"key");
              value = xmlGetProp (cur_node, (const xmlChar*)"value");
              
              codeslayer_config_set_setting (config, (gchar*) name, (gchar*) value);
              
              xmlFree (name);
              xmlFree (value);
            }
          else if (g_strcmp0 ((gchar*)cur_node->name, "plugin") == 0)
            {
              xmlChar *lib;
              lib = xmlGetProp (cur_node, (const xmlChar*)"lib");

              codeslayer_config_add_plugin (config, (gchar*) lib);
              xmlFree (lib);
            }
        }
      load_config (config, cur_node->children);
    }
}

static void
build_projects_xml (CodeSlayerProject *project,
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

static void 
build_documents_xml (CodeSlayerDocument *document,
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

static void 
build_plugins_xml (gchar   *name, 
                   GString **xml)
{
  *xml = g_string_append (*xml, "\n\t\t<plugin ");
  *xml = g_string_append (*xml, "lib=\"");
  *xml = g_string_append (*xml, name);
  *xml = g_string_append (*xml, "\"/>");
}

static void 
build_preferences_xml (gchar   *name,
                       gchar   *value, 
                       GString **xml)
{
  *xml = g_string_append (*xml, "\n\t\t<preference ");
  *xml = g_string_append (*xml, "key=\"");
  *xml = g_string_append (*xml, name);
  *xml = g_string_append (*xml, "\" ");
  *xml = g_string_append (*xml, "value=\"");
  *xml = g_string_append (*xml, value);
  *xml = g_string_append (*xml, "\"/>");
}

static void 
build_settings_xml (gchar   *name,
                    gchar   *value, 
                    GString **xml)
{
  *xml = g_string_append (*xml, "\n\t\t<setting ");
  *xml = g_string_append (*xml, "key=\"");
  *xml = g_string_append (*xml, name);
  *xml = g_string_append (*xml, "\" ");
  *xml = g_string_append (*xml, "value=\"");
  *xml = g_string_append (*xml, value);
  *xml = g_string_append (*xml, "\"/>");
}

static void
set_config_preferences_defaults (CodeSlayerConfig *config)
{
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_DISPLAY_LINE_NUMBERS, "true");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_CURRENT_LINE, "true");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_DISPLAY_RIGHT_MARGIN, "false");
  codeslayer_config_set_preference (config, CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_MATCHING_BRACKET, "false");
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

static void
set_config_settings_defaults (CodeSlayerConfig *config)
{
  codeslayer_config_set_setting (config, CODESLAYER_SETTINGS_SIDE_PANE_VISIBLE, "false");
  codeslayer_config_set_setting (config, CODESLAYER_SETTINGS_BOTTOM_PANE_VISIBLE, "false");
  codeslayer_config_set_setting (config, CODESLAYER_SETTINGS_DRAW_SPACES, "false");
  codeslayer_config_set_setting (config, CODESLAYER_SETTINGS_SYNC_WITH_EDITOR, "true");
}
