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

#ifndef __CODESLAYER_SEARCH_H__
#define	__CODESLAYER_SEARCH_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-groups.h>

G_BEGIN_DECLS

#define CODESLAYER_SEARCH_TYPE            (codeslayer_search_get_type ())
#define CODESLAYER_SEARCH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_SEARCH_TYPE, CodeSlayerSearch))
#define CODESLAYER_SEARCH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_SEARCH_TYPE, CodeSlayerSearchClass))
#define IS_CODESLAYER_SEARCH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_SEARCH_TYPE))
#define IS_CODESLAYER_SEARCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_SEARCH_TYPE))

typedef struct _CodeSlayerSearch CodeSlayerSearch;
typedef struct _CodeSlayerSearchClass CodeSlayerSearchClass;

struct _CodeSlayerSearch
{
  GtkWindow parent_instance;
};

struct _CodeSlayerSearchClass
{
  GtkWindowClass parent_class;

  void (*select_document) (CodeSlayerSearch *search);
  void (*close) (CodeSlayerSearch *search);
};

GType codeslayer_search_get_type (void) G_GNUC_CONST;
     
GtkWidget*  codeslayer_search_new           (GtkWindow               *window,
                                             CodeSlayerPreferences   *preferences, 
                                             CodeSlayerGroups        *groups);
                                     
void        codeslayer_search_add_page      (CodeSlayerSearch        *search, 
                                             const gchar             *file_paths);
void        codeslayer_search_clear         (CodeSlayerSearch        *search);

G_END_DECLS

#endif /* __CODESLAYER_SEARCH_H__ */
