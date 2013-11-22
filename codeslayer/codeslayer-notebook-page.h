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

#ifndef __CODESLAYER_NOTEBOOK_PAGE_H__
#define	__CODESLAYER_NOTEBOOK_PAGE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-document.h>

G_BEGIN_DECLS

#define CODESLAYER_NOTEBOOK_PAGE_TYPE            (codeslayer_notebook_page_get_type ())
#define CODESLAYER_NOTEBOOK_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_NOTEBOOK_PAGE_TYPE, CodeSlayerNotebookPage))
#define CODESLAYER_NOTEBOOK_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_NOTEBOOK_PAGE_TYPE, CodeSlayerNotebookPageClass))
#define IS_CODESLAYER_NOTEBOOK_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_NOTEBOOK_PAGE_TYPE))
#define IS_CODESLAYER_NOTEBOOK_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_NOTEBOOK_PAGE_TYPE))

typedef struct _CodeSlayerNotebookPage CodeSlayerNotebookPage;
typedef struct _CodeSlayerNotebookPageClass CodeSlayerNotebookPageClass;

struct _CodeSlayerNotebookPage
{
  GtkVBox parent_instance;
};

struct _CodeSlayerNotebookPageClass
{
  GtkVBoxClass parent_class;
};

GType codeslayer_notebook_page_get_type (void) G_GNUC_CONST;
     
GtkWidget*           codeslayer_notebook_page_new                               (GtkWidget              *editor);
GtkWidget*           codeslayer_notebook_page_get_source_view                   (CodeSlayerNotebookPage *notebook_page);
CodeSlayerDocument*  codeslayer_notebook_page_get_document                      (CodeSlayerNotebookPage *notebook_page);
void                 codeslayer_notebook_page_show_document_not_found_info_bar  (CodeSlayerNotebookPage *notebook_page);
void                 codeslayer_notebook_page_show_external_changes_info_bar    (CodeSlayerNotebookPage *notebook_page);

G_END_DECLS

#endif /* __CODESLAYER_NOTEBOOK_PAGE_H__ */
