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

#include "codeslayer-documentsearch.h"
#include "codeslayer-documentsearch-dialog.h"
#include "codeslayer-documentsearch-index.h"
#include <codeslayer/codeslayer-utils.h>

static void codeslayer_documentsearch_class_init  (CodeSlayerDocumentSearchClass *klass);
static void codeslayer_documentsearch_init        (CodeSlayerDocumentSearch      *search);
static void codeslayer_documentsearch_finalize    (CodeSlayerDocumentSearch      *search);

static void execute                               (CodeSlayerDocumentSearch      *search);
static GList* get_indexes                         (CodeSlayerDocumentSearch      *search);
static void get_project_indexes                   (CodeSlayerProject             *project, 
                                                   GFile                         *file, 
                                                   GList                         **indexes, 
                                                   GList                         *exclude_types,
                                                   GList                         *exclude_dirs);
static void write_indexes                         (GIOChannel                    *channel,
                                                   GList                         *indexes);                                        
                            
#define CODESLAYER_DOCUMENTSEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_DOCUMENTSEARCH_TYPE, CodeSlayerDocumentSearchPrivate))

typedef struct _CodeSlayerDocumentSearchPrivate CodeSlayerDocumentSearchPrivate;

struct _CodeSlayerDocumentSearchPrivate
{
  CodeSlayerProfile              *profile;
  CodeSlayerProjects             *projects;
  CodeSlayerRegistry             *registry;
  CodeSlayerDocumentSearchDialog *dialog;
};

G_DEFINE_TYPE (CodeSlayerDocumentSearch, codeslayer_documentsearch, G_TYPE_OBJECT)

static void
codeslayer_documentsearch_class_init (CodeSlayerDocumentSearchClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_documentsearch_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerDocumentSearchPrivate));
}

static void
codeslayer_documentsearch_init (CodeSlayerDocumentSearch *search) 
{
}

static void
codeslayer_documentsearch_finalize (CodeSlayerDocumentSearch *search)
{
  CodeSlayerDocumentSearchPrivate *priv;
  priv = CODESLAYER_DOCUMENTSEARCH_GET_PRIVATE (search);
  g_object_unref (priv->dialog);
  G_OBJECT_CLASS (codeslayer_documentsearch_parent_class)->finalize (G_OBJECT(search));
}

CodeSlayerDocumentSearch*
codeslayer_documentsearch_new (CodeSlayerProfile  *profile, 
                               CodeSlayerProjects *projects, 
                               CodeSlayerRegistry *registry)
{
  CodeSlayerDocumentSearchPrivate *priv;
  CodeSlayerDocumentSearch *search;

  search = CODESLAYER_DOCUMENTSEARCH (g_object_new (codeslayer_documentsearch_get_type (), NULL));
  priv = CODESLAYER_DOCUMENTSEARCH_GET_PRIVATE (search);

  priv->profile = profile;
  priv->projects = projects;
  priv->registry = registry;
  
  priv->dialog = codeslayer_documentsearch_dialog_new (profile, projects);
  
  g_signal_connect_swapped (G_OBJECT (projects), "projects-changed",
                            G_CALLBACK (codeslayer_documentsearch_index_files), search);

  return search;
}

void
codeslayer_documentsearch_index_files (CodeSlayerDocumentSearch *search)
{
  g_thread_new ("index files", (GThreadFunc) execute, search); 
}

void
codeslayer_documentsearch_run_dialog (CodeSlayerDocumentSearch *search)
{
  CodeSlayerDocumentSearchPrivate *priv;
  priv = CODESLAYER_DOCUMENTSEARCH_GET_PRIVATE (search);
  codeslayer_documentsearch_dialog_run  (priv->dialog);
}

static void
execute (CodeSlayerDocumentSearch *search)
{
  CodeSlayerDocumentSearchPrivate *priv;
  GList *indexes;
  gchar *profile_folder_path;
  gchar *profile_indexes_file;
  GIOChannel *channel;
  GError *error = NULL;
  
  priv = CODESLAYER_DOCUMENTSEARCH_GET_PRIVATE (search);

  profile_folder_path = codeslayer_profile_get_config_folder_path (priv->profile);
  profile_indexes_file = g_strconcat (profile_folder_path, G_DIR_SEPARATOR_S, "filesearch", NULL);
  
  channel = g_io_channel_new_file (profile_indexes_file, "w", &error);
  if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_EXISTS))
    {
      g_warning ("Error creating file search file: %s\n", error->message);
      g_error_free (error);
    }

  indexes = get_indexes (search);
  if (indexes != NULL)
    {
      write_indexes (channel, indexes);
      g_io_channel_shutdown (channel, TRUE, NULL);
      g_list_foreach (indexes, (GFunc) g_object_unref, NULL);
      g_list_free (indexes);
      g_io_channel_unref (channel);
    }
    
  g_free (profile_folder_path);
  g_free (profile_indexes_file);
}

static GList*
get_indexes (CodeSlayerDocumentSearch *search)
{
  CodeSlayerDocumentSearchPrivate *priv;
  GList *results = NULL;
  GList *projects;
  
  gchar *exclude_types_str;
  gchar *exclude_dirs_str;
  GList *exclude_types = NULL;
  GList *exclude_dirs = NULL;
  
  priv = CODESLAYER_DOCUMENTSEARCH_GET_PRIVATE (search);
  
  exclude_types_str = codeslayer_registry_get_string (priv->registry,
                                                      CODESLAYER_REGISTRY_PROJECTS_EXCLUDE_TYPES);
  exclude_dirs_str = codeslayer_registry_get_string (priv->registry,
                                                     CODESLAYER_REGISTRY_PROJECTS_EXCLUDE_DIRS);
  exclude_types = codeslayer_utils_string_to_list (exclude_types_str);
  exclude_dirs = codeslayer_utils_string_to_list (exclude_dirs_str);
  
  projects = codeslayer_profile_get_projects (priv->profile);
  while (projects != NULL)
    {
      CodeSlayerProject *project = projects->data;
      GList *indexes = NULL;
      GFile *file;
      const gchar *folder_path;
      
      folder_path = codeslayer_project_get_folder_path (project);
      file = g_file_new_for_path (folder_path);
      
      get_project_indexes (project, file, &indexes, exclude_types, exclude_dirs);
      if (indexes != NULL)
        results = g_list_concat (results, indexes);
        
      g_object_unref (file);

      projects = g_list_next (projects);
    }
    
  g_free (exclude_types_str);
  g_free (exclude_dirs_str);
  if (exclude_types)
    {
      g_list_foreach (exclude_types, (GFunc) g_free, NULL);
      g_list_free (exclude_types);
    }
  if (exclude_dirs)
    {
      g_list_foreach (exclude_dirs, (GFunc) g_free, NULL);
      g_list_free (exclude_dirs);
    }    
    
  return results;    
}

static void
get_project_indexes (CodeSlayerProject *project, 
                     GFile             *file,
                     GList             **indexes, 
                     GList             *exclude_types,
                     GList             *exclude_dirs)
{
  GFileEnumerator *enumerator;
  
  enumerator = g_file_enumerate_children (file, "standard::*",
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 
                                          NULL, NULL);
                                                                  
  if (enumerator != NULL)
    {
      GFileInfo *file_info;
      while ((file_info = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
        {
          GFile *child;
        
          const char *file_name = g_file_info_get_name (file_info);

          child = g_file_get_child (file, file_name);

          if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY)
            {
              if (!codeslayer_utils_contains_element (exclude_dirs, file_name))
                get_project_indexes (project, child, indexes, exclude_types, exclude_dirs);            
            }
          else
            {
              if (!codeslayer_utils_contains_element_with_suffix (exclude_types, file_name))
                {
                  CodeSlayerDocumentSearchIndex *index;
                  gchar *file_path;
                  file_path = g_file_get_path (child);
                  
                  index = codeslayer_documentsearch_index_new ();
                  codeslayer_documentsearch_index_set_file_name (index, file_name);
                  codeslayer_documentsearch_index_set_file_path (index, file_path);
                  
                  g_free (file_path);
                  
                  *indexes = g_list_prepend (*indexes, index);
                }
            }

          g_object_unref(child);
          g_object_unref (file_info);
        }
      g_object_unref (enumerator);
    }
}

static void
write_indexes (GIOChannel *channel, 
               GList      *indexes)
{
  while (indexes != NULL)
    {
      CodeSlayerDocumentSearchIndex *index = indexes->data;
      GIOStatus status;
      gchar *line;
      
      line = g_strdup_printf ("%s\t%s\t%s\n", 
                              codeslayer_documentsearch_index_get_file_name (index), 
                              codeslayer_documentsearch_index_get_file_path (index), 
                              codeslayer_documentsearch_index_get_project_key (index));

      status = g_io_channel_write_chars (channel, line, -1, NULL, NULL);
      
      g_free (line);
      
      if (status != G_IO_STATUS_NORMAL)
        g_warning ("Error writing to file search file.");

      indexes = g_list_next (indexes);
    }
    
  g_io_channel_flush (channel, NULL);
}
