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

#ifndef __CODESLAYER_NOTEBOOK_H__
#define	__CODESLAYER_NOTEBOOK_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-sourceview.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-profiles.h>

G_BEGIN_DECLS

#define CODESLAYER_NOTEBOOK_TYPE            (codeslayer_notebook_get_type ())
#define CODESLAYER_NOTEBOOK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_NOTEBOOK_TYPE, CodeSlayerNotebook))
#define CODESLAYER_NOTEBOOK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_NOTEBOOK_TYPE, CodeSlayerNotebookClass))
#define IS_CODESLAYER_NOTEBOOK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_NOTEBOOK_TYPE))
#define IS_CODESLAYER_NOTEBOOK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_NOTEBOOK_TYPE))

typedef struct _CodeSlayerNotebook CodeSlayerNotebook;
typedef struct _CodeSlayerNotebookClass CodeSlayerNotebookClass;

struct _CodeSlayerNotebook
{
  GtkNotebook parent_instance;
};

struct _CodeSlayerNotebookClass
{
  GtkNotebookClass parent_class;

  void (*select_document) (CodeSlayerNotebook *notebook);
  void (*document_saved) (CodeSlayerNotebook *notebook);
  void (*documents_all_saved) (CodeSlayerNotebook *notebook);
};

GType
codeslayer_notebook_get_type (void) G_GNUC_CONST;
  
GtkWidget*  codeslayer_notebook_new                     (GtkWindow          *window,
                                                         CodeSlayerProfile  *profile);

void        codeslayer_notebook_add_document            (CodeSlayerNotebook *notebook, 
                                                         CodeSlayerDocument *document);
gboolean    codeslayer_notebook_select_document         (CodeSlayerNotebook *notebook, 
                                                         CodeSlayerDocument *document);
void        codeslayer_notebook_save_document           (CodeSlayerNotebook *notebook, 
                                                         gint                page_num);
void        codeslayer_notebook_save_all_documents      (CodeSlayerNotebook *notebook);
gboolean    codeslayer_notebook_has_unsaved_documents   (CodeSlayerNotebook *notebook);
gboolean    codeslayer_notebook_close_document          (CodeSlayerNotebook *notebook, 
                                                         gint                page_num);
void        codeslayer_notebook_close_all_documents     (CodeSlayerNotebook *notebook);
GtkWidget*  codeslayer_notebook_get_active_source_view  (CodeSlayerNotebook *notebook);
GList*      codeslayer_notebook_get_all_source_views    (CodeSlayerNotebook *notebook);

G_END_DECLS

#endif /* __CODESLAYER_NOTEBOOK_H__ */
