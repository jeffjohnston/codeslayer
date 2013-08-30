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

#ifndef __CODESLAYER_DOCUMENT_H__
#define __CODESLAYER_DOCUMENT_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-project.h>

G_BEGIN_DECLS

#define CODESLAYER_DOCUMENT_TYPE            (codeslayer_document_get_type ())
#define CODESLAYER_DOCUMENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_DOCUMENT_TYPE, CodeSlayerDocument))
#define CODESLAYER_DOCUMENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_DOCUMENT_TYPE, CodeSlayerDocumentClass))
#define IS_CODESLAYER_DOCUMENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_DOCUMENT_TYPE))
#define IS_CODESLAYER_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_DOCUMENT_TYPE))

typedef struct _CodeSlayerDocument CodeSlayerDocument;
typedef struct _CodeSlayerDocumentClass CodeSlayerDocumentClass;

struct _CodeSlayerDocument
{
  GInitiallyUnowned parent_instance;
};

struct _CodeSlayerDocumentClass
{
  GInitiallyUnownedClass parent_class;
};

GType codeslayer_document_get_type (void) G_GNUC_CONST;

CodeSlayerDocument*  codeslayer_document_new (void);

const gint           codeslayer_document_get_line_number         (CodeSlayerDocument *document);
void                 codeslayer_document_set_line_number         (CodeSlayerDocument *document, 
                                                                  const gint          line_number);
GtkTreeRowReference* codeslayer_document_get_tree_row_reference  (CodeSlayerDocument *document);
void                 codeslayer_document_set_tree_row_reference  (CodeSlayerDocument *document, 
                                                                  GtkTreeRowReference *tree_row_reference);
const gchar*         codeslayer_document_get_file_path           (CodeSlayerDocument *document);
void                 codeslayer_document_set_file_path           (CodeSlayerDocument *document, 
                                                                  const gchar        *file_path);
const gboolean       codeslayer_document_get_active              (CodeSlayerDocument *document);
void                 codeslayer_document_set_active              (CodeSlayerDocument *document, 
                                                                  const gboolean      active);
CodeSlayerProject*   codeslayer_document_get_project             (CodeSlayerDocument *document);
void                 codeslayer_document_set_project             (CodeSlayerDocument *document, 
                                                                  CodeSlayerProject  *project);

G_END_DECLS

#endif /* __CODESLAYER_DOCUMENT_H__ */
