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

#ifndef __CODESLAYER_XML_H__
#define	__CODESLAYER_XML_H__

#include <gtk/gtk.h>
#include <stdarg.h>

G_BEGIN_DECLS

GHashTable*  codeslayer_xml_create_hashtable      (va_list      var_arg);
void         codeslayer_xml_free_hashtable        (GHashTable  *table);
gchar*       codeslayer_xml_serialize_gobjects    (GList       *objects,
                                                   gpointer     name, 
                                                   GHashTable  *table);
GList*       codeslayer_xml_deserialize_gobjects  (GType        type,
                                                   gboolean     floating,
                                                   const gchar *content,
                                                   gchar       *name,
                                                   GHashTable  *table);                                     

G_END_DECLS

#endif /* __CODESLAYER_XML_H__ */
