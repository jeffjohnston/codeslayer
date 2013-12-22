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

#ifndef __CODESLAYER_SOURCE_VIEW_H__
#define	__CODESLAYER_SOURCE_VIEW_H__

#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-search.h>
#include <codeslayer/codeslayer-profile.h>
#include <codeslayer/codeslayer-completion-provider.h>

G_BEGIN_DECLS

#define CODESLAYER_SOURCE_VIEW_TYPE            (codeslayer_source_view_get_type ())
#define CODESLAYER_SOURCE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_SOURCE_VIEW_TYPE, CodeSlayerSourceView))
#define CODESLAYER_SOURCE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_SOURCE_VIEW_TYPE, CodeSlayerSourceViewClass))
#define IS_CODESLAYER_SOURCE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_SOURCE_VIEW_TYPE))
#define IS_CODESLAYER_SOURCE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_SOURCE_VIEW_TYPE))

#define CODESLAYER_SOURCE_VIEW_UNTITLED "Document"

typedef struct _CodeSlayerSourceView CodeSlayerSourceView;
typedef struct _CodeSlayerSourceViewClass CodeSlayerSourceViewClass;

struct _CodeSlayerSourceView
{
  GtkSourceView parent_instance;
};

struct _CodeSlayerSourceViewClass
{
  GtkSourceViewClass parent_class;
  void (*completion) (CodeSlayerSourceView *source_view);
  void (*to_lowercase) (CodeSlayerSourceView *source_view);
  void (*to_uppercase) (CodeSlayerSourceView *source_view);
  void (*copy_lines) (CodeSlayerSourceView *source_view);
};

GType codeslayer_source_view_get_type (void) G_GNUC_CONST;

GtkWidget*           codeslayer_source_view_new                      (GtkWindow                    *window,
                                                                      CodeSlayerDocument           *document,
                                                                      CodeSlayerProfile            *profile);

CodeSlayerDocument*  codeslayer_source_view_get_document             (CodeSlayerSourceView         *source_view);
CodeSlayerSearch*    codeslayer_source_view_get_search               (CodeSlayerSourceView         *source_view);
GTimeVal*            codeslayer_source_view_get_modification_time    (CodeSlayerSourceView         *source_view);
void                 codeslayer_source_view_set_modification_time    (CodeSlayerSourceView         *source_view, 
                                                                      GTimeVal                     *modification_time);
void                 codeslayer_source_view_add_completion_provider  (CodeSlayerSourceView         *source_view, 
                                                                      CodeSlayerCompletionProvider *provider);
void                 codeslayer_source_view_set_text                 (CodeSlayerSourceView         *source_view, 
                                                                      gchar                        *text);
gboolean             codeslayer_source_view_scroll_to_line           (CodeSlayerSourceView         *source_view,
                                                                      gint                          line_number);
void                 codeslayer_source_view_sync_registry            (CodeSlayerSourceView         *source_view);

G_END_DECLS

#endif /* __CODESLAYER_SOURCE_VIEW_H__ */
