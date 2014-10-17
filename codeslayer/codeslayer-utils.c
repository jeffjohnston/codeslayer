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
#include <stdlib.h>
#include <string.h>
#include "encoding.h"
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-xml.h>
#include <codeslayer/codeslayer-preferences.h>

gint
codeslayer_utils_array_length (gchar **array)
{
  gint result = 0;
  while (*array)
    {
      array++;
      result++;
    }
  return result;
}

gboolean
codeslayer_utils_isdigit (gchar *string)
{
  while (*string)
    {
      if (!g_ascii_isdigit (*string))
          return FALSE;
      string++;
    }
  return TRUE;
}

gchar*
codeslayer_utils_strreplace (const gchar *string, 
                             const gchar *search,
                             const gchar *replacement)
{
  gchar *str, **arr;

  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (search != NULL, NULL);
  g_return_val_if_fail (replacement != NULL, NULL);

  arr = g_strsplit (string, search, -1);
  if (arr != NULL && arr[0] != NULL) 
    str = g_strjoinv (replacement, arr);
  else 
    str = g_strdup (string);

  g_strfreev (arr);

  return str;
}

gchar*
codeslayer_utils_substr (const gchar* string,
                         gint         start,
                         gint         end)
{
  gsize len = (end - start + 1);
  gchar *output = g_malloc0 (len + 1);
  return g_utf8_strncpy (output, &string[start], len);
} 

/*Recursively copy the source file/folder to the destination.*/
void
codeslayer_utils_file_copy (GFile  *source, 
                            GFile  *destination,
                            GError **error)
{
  GFileInfo *source_file_info;
  GFileEnumerator *enumerator;
  
  source_file_info = g_file_query_info (source, "standard::*",
                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                        NULL, NULL);
                                        
  if (g_file_info_get_file_type (source_file_info) == G_FILE_TYPE_DIRECTORY) 
    g_file_make_directory (destination, NULL, NULL);
  else
    g_file_copy (source, destination, G_FILE_COPY_NONE, NULL, NULL, NULL, NULL);
  
  g_object_unref (source_file_info);

  enumerator = g_file_enumerate_children (source, "standard::*",
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 
                                          NULL, NULL);
                                                                  
  if (enumerator != NULL)
    {
      GFileInfo *file_info;
      while ((file_info = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
        {
          GFile *source_child;
          GFile *destination_child;
        
          const char *file_name = g_file_info_get_name (file_info);

          source_child = g_file_get_child (source, file_name);
          destination_child = g_file_get_child (destination, file_name);

          if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY)
            codeslayer_utils_file_copy (source_child, destination_child, NULL);
          else 
            g_file_copy (source_child, destination_child, G_FILE_COPY_NONE, 
                         NULL, NULL, NULL, NULL);

          g_object_unref (source_child);
          g_object_unref (destination_child);
          g_object_unref (file_info);
        }
      g_object_unref (enumerator);
    }
}

/*Recursively delete the file.*/
void
codeslayer_utils_file_delete (GFile  *file, 
                              GError **error)
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
            codeslayer_utils_file_delete (child, error);
            
          g_file_delete (child, NULL, error);

          g_object_unref(child);
          g_object_unref (file_info);
        }
      g_object_unref (enumerator);
    }
    
  g_file_delete (file, NULL, error);
}

gboolean
codeslayer_utils_file_has_parent (GList *files, 
                                  GFile  *file)
{
  while (files != NULL)
    {
      if (g_file_has_parent (file, files->data))
        return TRUE;
      files = g_list_next (files);
    }
  return FALSE;
}

gboolean
codeslayer_utils_file_exists (const gchar *file_path)
{
  GFile *file;
  gboolean result;
  file = g_file_new_for_path (file_path);
  result = g_file_query_exists (file, NULL);
  g_object_unref (file);
  return result;
}

GList*
codeslayer_utils_string_to_list (const gchar *string)
{
  GList *results = NULL;
  gchar **split, **tmp;
  
  split = g_strsplit (string, ",", 0);
  
  if (split != NULL)
    {
      tmp = split;
      while (*tmp != NULL)
        {
          results = g_list_append (results, g_strstrip (g_strdup(*tmp)));
          tmp++;
        }
      g_strfreev (split);
    }
  
  return results;
}

gchar*
codeslayer_utils_list_to_string (GList *list)
{
  GString *gs;
    
  gs = g_string_new ("");

  if (list != NULL)
    {
      gchar *value = list->data;
      gs = g_string_append (gs, value);
      list = g_list_next (list);
    }
  
  while (list != NULL)
    {
      gchar *value = list->data;
      gs = g_string_append (gs, ",");
      gs = g_string_append (gs, value);
      list = g_list_next (list);
    }

  return g_string_free (gs, FALSE);
}

gboolean
codeslayer_utils_contains_element (GList       *list, 
                                   const gchar *string)
{
  GList *tmp;

  tmp = list;
  while (tmp != NULL)
    {
      gchar *data = tmp->data;
      if (g_strcmp0 (data, string) == 0)
        return TRUE;
      tmp = g_list_next (tmp);
    }

  return FALSE;
}

gboolean
codeslayer_utils_contains_element_with_suffix (GList       *list, 
                                               const gchar *string)
{
  GList *tmp;

  tmp = list;
  while (tmp != NULL)
    {
      gchar *data = tmp->data;
      if (g_str_has_suffix (string, data))
        return TRUE;
      tmp = g_list_next (tmp);
    }

  return FALSE;
}

gchar*
codeslayer_utils_to_lowercase (gchar *string)
{
  GString *gstring;
  
  gstring = g_string_new (string);
  
  gstring = g_string_ascii_down (gstring);
  
  return g_string_free (gstring, FALSE);
}

gchar*    
codeslayer_utils_create_key (void)
{
  GTimeVal timeval;
  GString *string;
  
  g_get_current_time (&timeval);
  string = g_string_new (NULL);
  g_string_append_printf (string, "%li", timeval.tv_sec);
    
  return g_string_free (string, FALSE);
}

gboolean  
codeslayer_utils_has_text (const gchar *string)
{
  gboolean result;
  gchar *tmp;
  
  if (string == NULL)
    return FALSE;
  
  tmp = g_strdup (string);
  g_strstrip(tmp);
  
  result = strlen (tmp) > 0;
  
  g_free (tmp);
  
  return result;
}

GList*
codeslayer_utils_deep_strcopy (GList *list)
{
  GList *values = NULL;
  
  while (list != NULL)
    {
      gchar *value = list->data;
      values = g_list_prepend (values, g_strdup (value));
      list = g_list_next (list);
    }
  
  return values;
}

void
codeslayer_utils_style_close_button (GtkWidget *button)
{
  GtkCssProvider *provider;
  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider,
                                   "#my-close-button {\n"
                                   " -GtkButton-default-border : 0;\n"
                                   " -GtkButton-default-outside-border : 0;\n"
                                   " -GtkButton-inner-border: 0;\n"
                                   " -GtkWidget-focus-line-width: 0px;\n"
                                   " -GtkWidget-focus-padding: 0px;\n"
                                   " padding: 0;}",
                                   -1, NULL);
  gtk_style_context_add_provider (gtk_widget_get_style_context (button),
                                  GTK_STYLE_PROVIDER (provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  gtk_widget_set_name (button, "my-close-button");
  g_object_unref (provider);
}

GList*
codeslayer_utils_deserialize_gobjects (GType        type,
                                       gboolean     floating,
                                       const gchar *contents,
                                       gpointer     name, ...)
{
  GList *results = NULL;
  GHashTable *table;
  va_list var_arg;

  va_start (var_arg, name);  
  table = codeslayer_xml_create_hashtable (var_arg);
  va_end (var_arg);

  results = codeslayer_xml_deserialize_gobjects (type, floating, contents, (gchar*)name, table);

  codeslayer_xml_free_hashtable (table);
  
  return results;
}                                       

GList*
codeslayer_utils_get_gobjects (GType     type,
                               gboolean  floating,
                               gchar    *file_path,
                               gpointer  name, ...)
{
  GList *results = NULL;
  GHashTable *table;
  va_list var_arg;
  gchar *contents;

  if (!g_file_get_contents (file_path, &contents, NULL, NULL))
    return NULL;
    
  va_start (var_arg, name);  
  table = codeslayer_xml_create_hashtable (var_arg);
  va_end (var_arg);

  results = codeslayer_xml_deserialize_gobjects (type, floating, contents, (gchar*)name, table);

  g_free (contents);
  codeslayer_xml_free_hashtable (table);
  
  return results;
}

void
codeslayer_utils_save_gobjects (GList       *objects,
                                const gchar *file_path, 
                                gpointer     name, ...)
{
  gchar *contents;
  GHashTable *table;
  va_list var_arg;
  GFile *file;

  file = g_file_new_for_path (file_path);
  if (!g_file_query_exists (file, NULL))
    {
      GFileIOStream *stream;           
      stream = g_file_create_readwrite (file, G_FILE_CREATE_NONE, NULL, NULL);
      if (g_io_stream_close (G_IO_STREAM (stream), NULL, NULL))
        g_object_unref (stream);
    }

  va_start (var_arg, name);
  table = codeslayer_xml_create_hashtable (var_arg);
  va_end (var_arg);
  
  contents = codeslayer_xml_serialize_gobjects (objects, name, table);

  g_file_set_contents (file_path, contents, -1, NULL);

  g_object_unref (file);
  g_free (contents);
  codeslayer_xml_free_hashtable (table);
}

GTimeVal*
codeslayer_utils_get_modification_time (const gchar *file_path)
{
  GFile *file;
  GFileInfo *info;
  GTimeVal time;
  GTimeVal *result;
  
  file = g_file_new_for_path (file_path);
  
  info = g_file_query_info (file, "*", 
                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, NULL);
                            
  g_file_info_get_modification_time (info, &time);
  
  result = g_malloc (sizeof (GTimeVal));
  
  result->tv_sec = time.tv_sec;
  result->tv_usec = time.tv_usec;
  
  g_object_unref (file);
  g_object_unref (info);
  
  return result;
}

gchar* 
codeslayer_utils_get_utf8_text (const gchar *file_path) 
{
  gchar *contents;
	gsize bytes;
	const gchar *charset;
	gchar *result;
	gint lineend;
	
	if (!g_file_get_contents (file_path, &contents, &bytes, NULL)) 
    return NULL;	    
	
	lineend = detect_line_ending (contents);
	if (lineend != LF)
	  convert_line_ending_to_lf (contents);
	
  charset = detect_charset (contents);
	if (charset == NULL)
    charset = get_default_charset ();
    
  if (g_strcmp0 (charset, "UTF-8") == 0)
    return contents;
  
  result = g_convert (contents, -1, "UTF-8", charset, NULL, NULL, NULL);
	  
  g_free(contents);
  
  return result;
}

gchar*
codeslayer_utils_get_file_path (const gchar *folder_path, 
                                const gchar *file_name)
{
  gchar *file_path;
  GFile *file;

  file_path = g_build_filename (folder_path, file_name, NULL);  
  file = g_file_new_for_path (file_path);
  if (!g_file_query_exists (file, NULL))
    {
      GFileIOStream *stream;
      stream = g_file_create_readwrite (file, G_FILE_CREATE_NONE, NULL, NULL);
      g_io_stream_close (G_IO_STREAM (stream), NULL, NULL);
      g_object_unref (stream);
    }

  g_object_unref (file);

  return file_path;
}

GKeyFile*      
codeslayer_utils_get_key_file (const gchar *file_path)
{  
  GKeyFile *key_file;
  key_file = g_key_file_new ();
  g_key_file_load_from_file (key_file, file_path, G_KEY_FILE_NONE, NULL);
  return key_file;
}

void 
codeslayer_utils_save_key_file (GKeyFile    *key_file, 
                                const gchar *file_path)
{
  gchar *data;
  gsize size;
  data = g_key_file_to_data (key_file, &size, NULL);
  g_file_set_contents (file_path, data, size, NULL);
  g_free (data);
}

GList*
codeslayer_utils_list_copy (GList *list)
{
  if (list == NULL)
    return NULL;

  return g_list_copy (list);
}

GList*
codeslayer_utils_get_profile_names ()
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

gboolean
codeslayer_utils_profile_exists (gchar *profile_name)
{
  gboolean result = TRUE;
  gchar *file_path;
  GFile *file;
  
  file_path = g_build_filename (g_get_home_dir (),
                                CODESLAYER_HOME,
                                CODESLAYER_PROFILES_DIR,
                                profile_name,
                                CODESLAYER_PROFILE_FILE,
                                NULL);

  file = g_file_new_for_path (file_path);

  result = g_file_query_exists (file, NULL);

  g_free (file_path);
  g_object_unref (file);    
  
  return result;
}
