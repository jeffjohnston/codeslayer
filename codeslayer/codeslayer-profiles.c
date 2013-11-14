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

#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-profiles.h>

#define CONFIG "codeslayer.config"

/**
 * SECTION:codeslayer-profiles
 * @short_description: Contains the projects.
 * @title: CodeSlayerProfiles
 * @include: codeslayer/codeslayer-profiles.h
 */

static void codeslayer_profiles_class_init  (CodeSlayerProfilesClass *klass);
static void codeslayer_profiles_init        (CodeSlayerProfiles      *profiles);
static void codeslayer_profiles_finalize    (CodeSlayerProfiles      *profiles);

static void load_profile                    (CodeSlayerProfile       *profile, 
                                             xmlNode                 *a_node);
static void set_profile_registry_defaults   (CodeSlayerProfile       *profile);

static void build_projects_xml              (CodeSlayerProject       *project,
                                             GString                 **xml);
static void build_documents_xml             (CodeSlayerDocument      *document,
                                             GString                 **xml);
static void build_plugins_xml               (gchar                   *name, 
                                             GString                 **xml);
static void build_registry_xml              (gchar                   *name,
                                             gchar                   *value, 
                                             GString                 **xml);

static void verify_directory_exists         (const gchar             *name);
static CodeSlayerProfile* retrieve_profile  (GFile                   *file);
                                                    
#define CODESLAYER_PROFILES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROFILES_TYPE, CodeSlayerProfilesPrivate))

typedef struct _CodeSlayerProfilesPrivate CodeSlayerProfilesPrivate;

struct _CodeSlayerProfilesPrivate
{
  CodeSlayerProfile *profile;
  GObject *registry;
};

G_DEFINE_TYPE (CodeSlayerProfiles, codeslayer_profiles, G_TYPE_OBJECT)
     
static void 
codeslayer_profiles_class_init (CodeSlayerProfilesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_profiles_finalize;

  g_type_class_add_private (klass, sizeof (CodeSlayerProfilesPrivate));                                                      
}

static void
codeslayer_profiles_init (CodeSlayerProfiles *profiles)
{
  CodeSlayerProfilesPrivate *priv; 
  priv = CODESLAYER_PROFILES_GET_PRIVATE (profiles);  
  priv->profile = NULL;
}

static void
codeslayer_profiles_finalize (CodeSlayerProfiles *profiles)
{
  CodeSlayerProfilesPrivate *priv; 
  priv = CODESLAYER_PROFILES_GET_PRIVATE (profiles);  
  g_object_unref (priv->profile);
  G_OBJECT_CLASS (codeslayer_profiles_parent_class)->finalize (G_OBJECT (profiles));
}

/**
 * codeslayer_profiles_new:
 *
 * Creates a new #CodeSlayerProfiles.
 *
 * Returns: a new #CodeSlayerProfiles. 
 */
CodeSlayerProfiles*
codeslayer_profiles_new ()
{
  CodeSlayerProfiles *profiles;
  profiles = CODESLAYER_PROFILES (g_object_new (codeslayer_profiles_get_type (), NULL));
  return profiles;
}

CodeSlayerProfile*
codeslayer_profiles_get_profile (CodeSlayerProfiles *profiles)
{
  CodeSlayerProfilesPrivate *priv; 
  priv = CODESLAYER_PROFILES_GET_PRIVATE (profiles);  
  return priv->profile;
}

void
codeslayer_profiles_load_profile (CodeSlayerProfiles *profiles, 
                                  CodeSlayerProfile  *profile)
{
  CodeSlayerProfilesPrivate *priv;
  priv = CODESLAYER_PROFILES_GET_PRIVATE (profiles);

  if (priv->profile)
    g_object_unref (priv->profile);

  priv->profile = profile;
}

CodeSlayerProfile*       
codeslayer_profiles_create_profile (CodeSlayerProfiles *profiles, 
                                    const gchar        *name)
{
  CodeSlayerProfile *profile;

  gchar *file_path;
  gchar *profile_name;
  GFile *file;

  file_path = g_build_filename (g_get_home_dir (),
                                CODESLAYER_HOME,
                                CODESLAYER_PROFILES_DIR,
                                name,
                                CODESLAYER_PROFILE_FILE,
                                NULL);
  
  verify_directory_exists (name);
  
  file = g_file_new_for_path (file_path);
  
  profile_name = g_strdup (name);
  
  profile = codeslayer_profile_new ();
  codeslayer_profile_set_file_path (profile, file_path);
  codeslayer_profile_set_name (profile, profile_name);
  set_profile_registry_defaults (profile);
  
  g_free (file_path);
  g_free (profile_name);
  g_object_unref (file);
  
  return profile;
}

CodeSlayerProfile*       
codeslayer_profiles_retrieve_profile (CodeSlayerProfiles *profiles, 
                                      const gchar        *name)
{
  CodeSlayerProfile *profile;
  gchar *file_path;
  gchar *profile_name;
  GFile *file;
  
  file_path = g_build_filename (g_get_home_dir (),
                                CODESLAYER_HOME,
                                CODESLAYER_PROFILES_DIR,
                                name,
                                CODESLAYER_PROFILE_FILE,
                                NULL);

  file = g_file_new_for_path (file_path);
  
  if (!g_file_query_exists (file, NULL))
    {
      g_free (file_path);
      g_object_unref (file);    
      return NULL;
    }
    
  profile_name = g_strdup (name);

  profile = retrieve_profile (file);
  codeslayer_profile_set_name (profile, profile_name);
  
  g_free (file_path);
  g_free (profile_name);
  g_object_unref (file);    
                                
  return profile;
}

static CodeSlayerProfile*
retrieve_profile (GFile *file)
{
  CodeSlayerProfile *profile;
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  gchar *file_path = NULL;
  
  file_path = g_file_get_path (file);
  
  doc = xmlReadFile (file_path, NULL, 0);
  if (doc == NULL)
    {
      g_warning ("Could not parse profile %s\n", file_path);
      xmlCleanupParser();
      if (file_path)
        g_free (file_path);
      return NULL;
    }

  profile = codeslayer_profile_new ();
  
  root_element = xmlDocGetRootElement (doc);

  load_profile (profile, root_element);
  codeslayer_profile_set_file_path (profile, file_path);

  xmlFreeDoc (doc);
  xmlCleanupParser ();
  g_free (file_path);
  
  return profile;
}

static void
load_profile (CodeSlayerProfile *profile, 
              xmlNode          *a_node)
{
  xmlNode *cur_node = NULL;
  CodeSlayerRegistry *registry;
  
  registry = codeslayer_profile_get_registry (profile);

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
              codeslayer_profile_add_project (profile, project);
              
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
              codeslayer_profile_add_document (profile, document);
              
              project = codeslayer_profile_get_project_by_file_path (profile, (gchar*) file_path);
              codeslayer_document_set_project (document, project);
              
              xmlFree (file_path);
              xmlFree (line_number);
            }
          else if (g_strcmp0 ((gchar*)cur_node->name, "setting") == 0)
            {
              xmlChar *name;
              xmlChar *value;
              
              name = xmlGetProp (cur_node, (const xmlChar*)"key");
              value = xmlGetProp (cur_node, (const xmlChar*)"value");
              
              codeslayer_registry_set_setting (registry, (gchar*) name, (gchar*) value);
              
              xmlFree (name);
              xmlFree (value);
            }
          else if (g_strcmp0 ((gchar*)cur_node->name, "plugin") == 0)
            {
              xmlChar *lib;
              lib = xmlGetProp (cur_node, (const xmlChar*)"lib");

              codeslayer_profile_add_plugin (profile, (gchar*) lib);
              xmlFree (lib);
            }
        }
      load_profile (profile, cur_node->children);
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
build_registry_xml (gchar   *name,
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
set_profile_registry_defaults (CodeSlayerProfile *profile)
{
  CodeSlayerRegistry *registry;
  registry = codeslayer_profile_get_registry (profile);

  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE, "false");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE, "false");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_DRAW_SPACES, "false");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_SYNC_WITH_EDITOR, "true");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_ENABLE_PROJECTS, "false");
  
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_DISPLAY_LINE_NUMBERS, "false");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_HIGHLIGHT_CURRENT_LINE, "false");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_DISPLAY_RIGHT_MARGIN, "false");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_HIGHLIGHT_MATCHING_BRACKET, "false");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_INSERT_SPACES_INSTEAD_OF_TABS, "true");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_ENABLE_AUTOMATIC_INDENTATION, "true");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_RIGHT_MARGIN_POSITION, "80");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_TAB_WIDTH, "2");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_FONT, "Monospace 9");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_THEME, "classic");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_TAB_POSITION, "top");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_SIDE_PANE_TAB_POSITION, "top");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_BOTTOM_PANE_TAB_POSITION, "left");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_PROJECTS_EXCLUDE_DIRS, ".csv,.git,.svn");
  codeslayer_registry_set_setting (registry, CODESLAYER_REGISTRY_EDITOR_WORD_WRAP_TYPES, ".txt");
}

void
codeslayer_profiles_save_profile (CodeSlayerProfiles *profiles, 
                                  CodeSlayerProfile  *profile)
{
  GString *xml;
  gchar *contents;
  const gchar *file_path;
  GList *projects;
  GList *documents;
  GList *plugins;
  CodeSlayerRegistry *registry;
  GHashTable *hashtable;
  
  projects = codeslayer_profile_get_projects (profile);
  documents = codeslayer_profile_get_documents (profile);
  plugins = codeslayer_profile_get_plugins (profile);
  registry = codeslayer_profile_get_registry (profile);
  hashtable = codeslayer_registry_get_hashtable (registry);
  
  xml = g_string_new ("<profile>");
  
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

  xml = g_string_append (xml, "\n\t<registry>");
  g_hash_table_foreach (hashtable, (GHFunc)build_registry_xml, &xml);
  xml = g_string_append (xml, "\n\t</registry>");

  xml = g_string_append (xml, "\n</profile>");
  
  contents = g_string_free (xml, FALSE);
  
  file_path = codeslayer_profile_get_file_path (profile);

  g_file_set_contents (file_path, contents, -1, NULL);
  
  g_free (contents);
}

GList*
codeslayer_profiles_get_profile_names (CodeSlayerProfiles *profiles)
{
  GList *results = NULL;
  gchar *file_path;
  GFile *file;
  GFileEnumerator *enumerator;
  
  file_path = g_build_filename (g_get_home_dir (),
                                CODESLAYER_HOME,
                                CODESLAYER_PROFILES_DIR,
                                NULL);
  
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
          file_name = g_file_info_get_name (file_info);
          results = g_list_append (results, g_strdup (file_name));
          g_object_unref (file_info);
        }
      g_object_unref (enumerator);
    }

  g_free (file_path);
  g_object_unref (file);
  
  return results;
}

static void
verify_directory_exists (const gchar* name)
{
  gchar *file_path;
  GFile *file;
  
  file_path = g_build_filename (g_get_home_dir (),
                                CODESLAYER_HOME,
                                CODESLAYER_PROFILES_DIR,
                                name,
                                NULL);

  file = g_file_new_for_path (file_path);

  if (!g_file_query_exists (file, NULL))
    g_file_make_directory (file, NULL, NULL);
    
  g_free (file_path);
  g_object_unref (file);
}
