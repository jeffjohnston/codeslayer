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

#ifndef __CODESLAYER_NOTEBOOK_SEARCH_H__
#define	__CODESLAYER_NOTEBOOK_SEARCH_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-profiles.h>

G_BEGIN_DECLS

#define CODESLAYER_NOTEBOOK_SEARCH_TYPE            (codeslayer_notebook_search_get_type ())
#define CODESLAYER_NOTEBOOK_SEARCH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_NOTEBOOK_SEARCH_TYPE, CodeSlayerNotebookSearch))
#define CODESLAYER_NOTEBOOK_SEARCH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_NOTEBOOK_SEARCH_TYPE, CodeSlayerNotebookSearchClass))
#define IS_CODESLAYER_NOTEBOOK_SEARCH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_NOTEBOOK_SEARCH_TYPE))
#define IS_CODESLAYER_NOTEBOOK_SEARCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_NOTEBOOK_SEARCH_TYPE))

typedef struct _CodeSlayerNotebookSearch CodeSlayerNotebookSearch;
typedef struct _CodeSlayerNotebookSearchClass CodeSlayerNotebookSearchClass;

struct _CodeSlayerNotebookSearch
{
  GtkVBox parent_instance;
};

struct _CodeSlayerNotebookSearchClass
{
  GtkVBoxClass parent_class;

  void (*close_search) (CodeSlayerNotebookSearch * notebook_search);
};

GType codeslayer_notebook_search_get_type (void) G_GNUC_CONST;

GtkWidget*  codeslayer_notebook_search_new                  (GtkWidget                *notebook, 
                                                             CodeSlayerProfile        *profile);
void        codeslayer_notebook_search_find                 (CodeSlayerNotebookSearch *notebook_search);
void        codeslayer_notebook_search_replace              (CodeSlayerNotebookSearch *notebook_search);
void        codeslayer_notebook_search_find_next            (CodeSlayerNotebookSearch *notebook_search);
void        codeslayer_notebook_search_find_previous        (CodeSlayerNotebookSearch *notebook_search);
void        codeslayer_notebook_search_create_search_marks  (CodeSlayerNotebookSearch *notebook_search, 
                                                             gboolean                  scrollable);

G_END_DECLS

#endif /* __CODESLAYER_NOTEBOOK_SEARCH_H__ */
