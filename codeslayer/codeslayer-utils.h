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

#ifndef __CODESLAYER_UTILS_H__
#define	__CODESLAYER_UTILS_H__

#include <locale.h>
#include <libintl.h>
#include <stdarg.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop(String)
  
gint      codeslayer_utils_array_length                  (gchar       **array);
gboolean  codeslayer_utils_isdigit                       (gchar       *string);
void      codeslayer_utils_file_copy                     (GFile       *source, 
                                                          GFile       *destination, 
                                                          GError      **error);
void      codeslayer_utils_file_delete                   (GFile       *file, 
                                                          GError      **error);                                                    
gboolean  codeslayer_utils_file_exists                   (const gchar *file_path);
gboolean  codeslayer_utils_file_has_parent               (GList       *sources, 
                                                          GFile       *source);
gchar*    codeslayer_utils_substr                        (const gchar *string,
                                                          gint         start,
                                                          gint         end);                                             
gchar*    codeslayer_utils_strreplace                    (const gchar *string, 
                                                          const gchar *search,
                                                          const gchar *replacement);
GList*    codeslayer_utils_string_to_list                (const gchar *string);
gchar*    codeslayer_utils_list_to_string                (GList       *list);
gboolean  codeslayer_utils_contains_element              (GList       *list, 
                                                          const gchar *string);                                                    
gboolean  codeslayer_utils_contains_element_with_suffix  (GList       *list, 
                                                          const gchar *string);                                                          
gchar*    codeslayer_utils_to_lowercase                  (gchar       *string);
gchar*    codeslayer_utils_create_key                    (void);
gboolean  codeslayer_utils_has_text                      (const gchar *string);
GList*    codeslayer_utils_deep_strcopy                  (GList       *list);

GList*   codeslayer_utils_deserialize_gobjects           (GType        type,
                                                          gboolean     floating,
                                                          const gchar *contents,
                                                          gpointer name, ...);
void      codeslayer_utils_save_gobjects                 (GList       *objects,
                                                          const gchar *file_path, 
                                                          gpointer name, ...);
GList*    codeslayer_utils_get_gobjects                  (GType        type,
                                                          gboolean     floating,
                                                          gchar       *file_path,
                                                          gpointer  name, ...);
gchar*    codeslayer_utils_get_file_path                 (const gchar *folder_path, 
                                                          const gchar *file_name);
GKeyFile* codeslayer_utils_get_keyfile                   (const gchar *file_path);
void codeslayer_utils_save_keyfile                       (GKeyFile    *keyfile, 
                                                          const gchar *file_path);
void      codeslayer_utils_style_close_button            (GtkWidget   *widget);
GTimeVal* codeslayer_utils_get_modification_time         (const gchar *file_path);
gchar*    codeslayer_utils_get_utf8_text                 (const gchar *file_path);

G_END_DECLS

#endif /* __CODESLAYER_UTILS_H__ */
