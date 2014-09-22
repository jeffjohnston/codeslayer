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

#ifndef __CODESLAYER_DOCUMENTSEARCH_INDEX_H__
#define	__CODESLAYER_DOCUMENTSEARCH_INDEX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_DOCUMENTSEARCH_INDEX_TYPE            (codeslayer_document_search_index_get_type ())
#define CODESLAYER_DOCUMENTSEARCH_INDEX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_DOCUMENTSEARCH_INDEX_TYPE, CodeSlayerDocumentSearchIndex))
#define CODESLAYER_DOCUMENTSEARCH_INDEX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_DOCUMENTSEARCH_INDEX_TYPE, CodeSlayerDocumentSearchIndexClass))
#define IS_CODESLAYER_DOCUMENTSEARCH_INDEX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_DOCUMENTSEARCH_INDEX_TYPE))
#define IS_CODESLAYER_DOCUMENTSEARCH_INDEX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_DOCUMENTSEARCH_INDEX_TYPE))

typedef struct _CodeSlayerDocumentSearchIndex CodeSlayerDocumentSearchIndex;
typedef struct _CodeSlayerDocumentSearchIndexClass CodeSlayerDocumentSearchIndexClass;

struct _CodeSlayerDocumentSearchIndex
{
  GObject parent_instance;
};

struct _CodeSlayerDocumentSearchIndexClass
{
  GObjectClass parent_class;
};

GType codeslayer_document_search_index_get_type (void) G_GNUC_CONST;

CodeSlayerDocumentSearchIndex*  codeslayer_document_search_index_new              (void);

const gchar*      codeslayer_document_search_index_get_project_key  (CodeSlayerDocumentSearchIndex *index);
void              codeslayer_document_search_index_set_project_key  (CodeSlayerDocumentSearchIndex *index,
                                                                     const gchar                   *project_key);
const gchar*      codeslayer_document_search_index_get_file_name    (CodeSlayerDocumentSearchIndex *index);
void              codeslayer_document_search_index_set_file_name    (CodeSlayerDocumentSearchIndex *index,
                                                                     const gchar                   *file_name);
const gchar*      codeslayer_document_search_index_get_file_path    (CodeSlayerDocumentSearchIndex *index);
void              codeslayer_document_search_index_set_file_path    (CodeSlayerDocumentSearchIndex *index,
                                                                     const gchar                   *file_path);

G_END_DECLS

#endif /* __CODESLAYER_DOCUMENTSEARCH_INDEX_H__ */
