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
#include <codeslayer/codeslayer-utils.h>
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
codeslayer_utils_save_gobjects (GList       *objects,
                                const gchar *file_path, 
                                gpointer     name, ...)
{
  va_list var_arg;
  
  GString *xml;
  GFile *file;
  gchar *contents;
  
  xml = g_string_new ("<");
  xml = g_string_append (xml, name);
  xml = g_string_append (xml, "s>\n");
  
  while (objects != NULL)
    {
      GObject *object = objects->data;
      gchar *property;
      GType type;

      va_start (var_arg, name);
      
      xml = g_string_append (xml, "\t<");
      xml = g_string_append (xml, name);
      
      do 
        {
          property = va_arg (var_arg, gchar*);
          if (property != NULL)
            {
              type = va_arg (var_arg, GType);
              
              if (type == G_TYPE_STRING)
                {
                  gchar *val;
                  gchar *attr; 
                  g_object_get (object, property, &val, NULL);  
                  xml = g_string_append (xml, " ");
                  xml = g_string_append (xml, property);
                  attr = g_markup_printf_escaped ("=\"%s", val);
                  xml = g_string_append (xml, attr);
                  xml = g_string_append (xml, "\"");
                  g_free (val);         
                  g_free (attr);         
                }              
              else if (type == G_TYPE_BOOLEAN)
                {
                  gboolean val;
                  gchar *attr; 
                  g_object_get (object, property, &val, NULL);  
                  xml = g_string_append (xml, " ");
                  xml = g_string_append (xml, property);
                  attr = g_markup_printf_escaped ("=\"%d", val);
                  xml = g_string_append (xml, attr);
                  xml = g_string_append (xml, "\"");
                  g_free (attr);         
                }              
              else if (type == G_TYPE_INT)
                {
                  gint val;
                  gchar *attr; 
                  g_object_get (object, property, &val, NULL);  
                  xml = g_string_append (xml, " ");
                  xml = g_string_append (xml, property);
                  attr = g_markup_printf_escaped ("=\"%d", val);
                  xml = g_string_append (xml, attr);
                  xml = g_string_append (xml, "\"");
                  g_free (attr);         
                }              
            }
        }
      while (property != NULL);
      
      xml = g_string_append (xml, "/>\n");

      va_end (var_arg);
      
      objects = g_list_next (objects);
    }

  xml = g_string_append (xml, "</");
  xml = g_string_append (xml, name);
  xml = g_string_append (xml, "s>");

  file = g_file_new_for_path (file_path);
  if (!g_file_query_exists (file, NULL))
    {
      GFileIOStream *stream;           
      stream = g_file_create_readwrite (file, G_FILE_CREATE_NONE, NULL, NULL);
      if (g_io_stream_close (G_IO_STREAM (stream), NULL, NULL))
        g_object_unref (stream);
    }

  contents = g_string_free (xml, FALSE);

  g_file_set_contents (file_path, contents, -1, NULL);

  g_object_unref (file);
  g_free (contents);
}

typedef struct
{
  GList      *list;
  GHashTable *args;
  gchar      *name;
  GType       type;
  gboolean    floating;
} XmlData;

static void 
xml_start (GMarkupParseContext *context,
           const gchar         *element_name,
           const gchar         **attribute_names,
           const gchar         **attribute_values,
           gpointer             data,
           GError              **error)
{
  XmlData *xml_data = data;
  const gchar** names = attribute_names;
  const gchar** values = attribute_values;
  
  if (g_strcmp0 (element_name, (gchar*)xml_data->name) == 0)
    {
      GObject *object;
      object = g_object_new (xml_data->type, NULL);
      
      for (; *names; names++, values++)
        {
          GType *type;
          type = g_hash_table_lookup (xml_data->args, *names);
          if (type != NULL) /*here for backwards compatibility with file format change*/
            {
              if (*type == G_TYPE_STRING)
                g_object_set (object, *names, *values, NULL);
              else if (*type == G_TYPE_BOOLEAN)
                g_object_set (object, *names, atoi(*values), NULL);
            }
        }
        
      if (xml_data->floating)
        g_object_force_floating (object);        
        
      xml_data->list = g_list_prepend (xml_data->list, object);
    }
}

static void
xml_err (GMarkupParseContext *context,
         GError              *error,
         gpointer             data)
{
  g_critical ("object configuration error %s", error->message);
}

static gboolean
xml_remove (gpointer key,
            gpointer value)
{
  g_free (key);
  g_free (value);
  return TRUE;
}
                       
GList*
codeslayer_utils_get_gobjects (GType        type,
                               gboolean     floating,
                               const gchar *file_path,
                               gpointer     name, ...)
{
  GMarkupParser parser = { xml_start, NULL, NULL, NULL, xml_err };
  GMarkupParseContext *context;
  GList *results = NULL;
  gchar *content;
  XmlData *xml_data;
  va_list var_arg;
  gchar *property;
      
  if (!g_file_get_contents (file_path, &content, NULL, NULL))
    return NULL;
    
  xml_data = g_malloc (sizeof (XmlData));
  xml_data->list = NULL;
  xml_data->args = NULL;
  xml_data->type = type;
  xml_data->floating = floating;
  xml_data->name = (gchar*)name;
  
  va_start (var_arg, name);
  
  xml_data->args = g_hash_table_new ((GHashFunc)g_str_hash, (GEqualFunc)g_str_equal);
  
  do 
    {
      property = va_arg (var_arg, gchar*);
      if (property != NULL)
        {
          GType type;
          GType *type_copy;
          type = va_arg (var_arg, GType);
          type_copy = g_malloc (sizeof (GType));
          *type_copy = type;
          g_hash_table_insert (xml_data->args, g_strdup (property), type_copy);
        }
    }
  while (property != NULL);

  context = g_markup_parse_context_new (&parser, 0, xml_data, NULL);
  g_markup_parse_context_parse (context, content, -1, NULL);
  
  results = g_list_copy (xml_data->list);

  g_list_free (xml_data->list);
  g_hash_table_foreach_remove (xml_data->args, (GHRFunc) xml_remove, NULL);
  g_hash_table_destroy (xml_data->args);
  g_free (xml_data);
  g_free (content);
  g_markup_parse_context_free (context);
  return results;
}
