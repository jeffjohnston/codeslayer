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

#ifndef __CODESLAYER_DOCUMENT_LINKER_H__
#define	__CODESLAYER_DOCUMENT_LINKER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_DOCUMENT_LINKER_TYPE            (codeslayer_document_linker_get_type ())
#define CODESLAYER_DOCUMENT_LINKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_DOCUMENT_LINKER_TYPE, CodeSlayerDocumentLinker))
#define CODESLAYER_DOCUMENT_LINKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_DOCUMENT_LINKER_TYPE, CodeSlayerDocumentLinkerClass))
#define IS_CODESLAYER_DOCUMENT_LINKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_DOCUMENT_LINKER_TYPE))
#define IS_CODESLAYER_DOCUMENT_LINKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_DOCUMENT_LINKER_TYPE))

typedef struct _CodeSlayerDocumentLinker CodeSlayerDocumentLinker;
typedef struct _CodeSlayerDocumentLinkerClass CodeSlayerDocumentLinkerClass;

struct _CodeSlayerDocumentLinker
{
  GObject parent_instance;
};

struct _CodeSlayerDocumentLinkerClass
{
  GObjectClass parent_class;
};

GType codeslayer_document_linker_get_type (void) G_GNUC_CONST;

CodeSlayerDocumentLinker*  codeslayer_document_linker_new           (GObject                *codeslayer, 
                                                                     GtkTextView            *text_view);
const gchar *              codeslayer_document_linker_get_pattern   (CodeSlayerDocumentLinker *linker);
void                       codeslayer_document_linker_set_pattern   (CodeSlayerDocumentLinker *linker, 
                                                                     const gchar            *pattern);
gint                       codeslayer_document_linker_create_links  (CodeSlayerDocumentLinker *linker);

G_END_DECLS

#endif /* __CODESLAYER_DOCUMENT_LINKER_H__ */
