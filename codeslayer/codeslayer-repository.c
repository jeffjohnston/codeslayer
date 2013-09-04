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
#include <codeslayer/codeslayer-settings.h>
#include <codeslayer/codeslayer-repository.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-plugin.h>


static CodeSlayerPlugin*  load_plugin_from_file  (gchar *file_path);

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
