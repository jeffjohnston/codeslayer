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

#ifndef __CODESLAYER_DOCUMENTSEARCH_DIALOG_H__
#define	__CODESLAYER_DOCUMENTSEARCH_DIALOG_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-profile.h>

G_BEGIN_DECLS

#define CODESLAYER_DOCUMENTSEARCH_DIALOG_TYPE            (codeslayer_documentsearch_dialog_get_type ())
#define CODESLAYER_DOCUMENTSEARCH_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_DOCUMENTSEARCH_DIALOG_TYPE, CodeSlayerDocumentSearchDialog))
#define CODESLAYER_DOCUMENTSEARCH_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_DOCUMENTSEARCH_DIALOG_TYPE, CodeSlayerDocumentSearchDialogClass))
#define IS_CODESLAYER_DOCUMENTSEARCH_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_DOCUMENTSEARCH_DIALOG_TYPE))
#define IS_CODESLAYER_DOCUMENTSEARCH_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_DOCUMENTSEARCH_DIALOG_TYPE))

#define CODESLAYER_DOCUMENT_SEARCH_FILE "documentsearch"

typedef struct _CodeSlayerDocumentSearchDialog CodeSlayerDocumentSearchDialog;
typedef struct _CodeSlayerDocumentSearchDialogClass CodeSlayerDocumentSearchDialogClass;

struct _CodeSlayerDocumentSearchDialog
{
  GObject parent_instance;
};

struct _CodeSlayerDocumentSearchDialogClass
{
  GObjectClass parent_class;
};

GType codeslayer_documentsearch_dialog_get_type (void) G_GNUC_CONST;
     
CodeSlayerDocumentSearchDialog*  codeslayer_documentsearch_dialog_new  (CodeSlayerProfile  *profile, 
                                                                        CodeSlayerProjects *projects);

void                             codeslayer_documentsearch_dialog_run  (CodeSlayerDocumentSearchDialog *dialog);
                                     
G_END_DECLS

#endif /* __CODESLAYER_DOCUMENTSEARCH_DIALOG_H__ */
