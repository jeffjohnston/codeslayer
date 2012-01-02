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
#include <codeslayer/codeslayer-xml.h>

static void xml_start                     (GMarkupParseContext *context,
                                           const gchar         *element_name,
                                           const gchar         **attribute_names,
                                           const gchar         **attribute_values,
                                           gpointer             data,
                                           GError              **error);
static void xml_err                       (GMarkupParseContext *context,
                                           GError              *error,
                                           gpointer             data);
static gboolean hashtable_free            (gpointer             key,
                                           gpointer             value);
GHashTable* convert_varargs_to_hashtable  (va_list              var_arg);

typedef struct
{
  GList      *list;
  GHashTable *args;
  gchar      *name;
  GType       type;
  gboolean    floating;
} XmlData;

GHashTable*
codeslayer_xml_create_hashtable (va_list var_arg)
{
  GHashTable *results;
  gchar *property;
      
  results = g_hash_table_new ((GHashFunc)g_str_hash, (GEqualFunc)g_str_equal);
  
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
          g_hash_table_insert (results, g_strdup (property), type_copy);
        }
    }
  while (property != NULL);

  return results;
}

void  
codeslayer_xml_free_hashtable (GHashTable  *table)
{
  g_hash_table_foreach_remove (table, (GHRFunc) hashtable_free, NULL);
  g_hash_table_destroy (table);
}

gchar*
codeslayer_xml_serialize_gobjects (GList      *objects,
                                   gpointer    name, 
                                   GHashTable *table)
{
  GString *xml;
  GList *keys; 
  
  xml = g_string_new ("<");
  xml = g_string_append (xml, name);
  xml = g_string_append (xml, "s>\n");

  keys = g_hash_table_get_keys (table);
  
  while (objects != NULL)
    {
      GObject *object = objects->data;
      GList *list; 
      
      xml = g_string_append (xml, "\t<");
      xml = g_string_append (xml, name);
      
      list = keys;
      
      while (list != NULL)
        {
          gchar *property = list->data;
          GType *type = (GType*)g_hash_table_lookup (table, property);
          
          if (*type == G_TYPE_STRING)
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
          else if (*type == G_TYPE_BOOLEAN)
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
          else if (*type == G_TYPE_INT)
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
                        
          list = g_list_next (list);          
        }
      
      xml = g_string_append (xml, "/>\n");

      objects = g_list_next (objects);
    }
    
  g_list_free (keys);

  xml = g_string_append (xml, "</");
  xml = g_string_append (xml, name);
  xml = g_string_append (xml, "s>");

  return g_string_free (xml, FALSE);
}

GList*
codeslayer_xml_deserialize_gobjects (GType        type,
                                     gboolean     floating,
                                     const gchar *content,
                                     gchar       *name,
                                     GHashTable  *table)
{
  GMarkupParser parser = { xml_start, NULL, NULL, NULL, xml_err };
  GMarkupParseContext *context;
  GList *results = NULL;
  XmlData *xml_data;
  xml_data = g_malloc (sizeof (XmlData));
  xml_data->list = NULL;
  xml_data->args = table;
  xml_data->type = type;
  xml_data->floating = floating;
  xml_data->name = name;
  
  context = g_markup_parse_context_new (&parser, 0, xml_data, NULL);
  g_markup_parse_context_parse (context, content, -1, NULL);
  
  results = g_list_copy (xml_data->list);
  results = g_list_reverse (results);

  g_list_free (xml_data->list);
  g_free (xml_data);
  g_markup_parse_context_free (context);
  return results;
}

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
              else if (*type == G_TYPE_INT)
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
hashtable_free (gpointer key,
                gpointer value)
{
  g_free (key);
  g_free (value);
  return TRUE;
}
