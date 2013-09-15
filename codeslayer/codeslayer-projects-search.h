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

#ifndef __CODESLAYER_PROJECTS_SEARCH_H__
#define	__CODESLAYER_PROJECTS_SEARCH_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-config.h>

G_BEGIN_DECLS

#define CODESLAYER_PROJECTS_SEARCH_TYPE            (codeslayer_projects_search_get_type ())
#define CODESLAYER_PROJECTS_SEARCH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROJECTS_SEARCH_TYPE, CodeSlayerProjectsSearch))
#define CODESLAYER_PROJECTS_SEARCH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROJECTS_SEARCH_TYPE, CodeSlayerProjectsSearchClass))
#define IS_CODESLAYER_PROJECTS_SEARCH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROJECTS_SEARCH_TYPE))
#define IS_CODESLAYER_PROJECTS_SEARCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROJECTS_SEARCH_TYPE))

typedef struct _CodeSlayerProjectsSearch CodeSlayerProjectsSearch;
typedef struct _CodeSlayerProjectsSearchClass CodeSlayerProjectsSearchClass;

struct _CodeSlayerProjectsSearch
{
  GtkWindow parent_instance;
};

struct _CodeSlayerProjectsSearchClass
{
  GtkWindowClass parent_class;

  void (*select_document) (CodeSlayerProjectsSearch *search);
  void (*close) (CodeSlayerProjectsSearch *search);
};

GType codeslayer_projects_search_get_type (void) G_GNUC_CONST;
     
GtkWidget*  codeslayer_projects_search_new             (GtkWindow          *window, 
                                                        CodeSlayerRegistry *registry);

void        codeslayer_projects_search_find_projects   (CodeSlayerProjectsSearch *search);
void        codeslayer_projects_search_find_selection  (CodeSlayerProjectsSearch *search, 
                                                        const gchar              *file_paths);
void        codeslayer_projects_search_clear           (CodeSlayerProjectsSearch *search);
void        codeslayer_projects_search_set_config      (CodeSlayerProjectsSearch *search, 
                                                        CodeSlayerConfig         *config);

G_END_DECLS

#endif /* __CODESLAYER_PROJECTS_SEARCH_H__ */
