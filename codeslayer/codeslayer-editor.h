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

#ifndef __CODESLAYER_EDITOR_H__
#define	__CODESLAYER_EDITOR_H__

#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-preferences.h>

G_BEGIN_DECLS

#define CODESLAYER_EDITOR_TYPE            (codeslayer_editor_get_type ())
#define CODESLAYER_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_EDITOR_TYPE, CodeSlayerEditor))
#define CODESLAYER_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_EDITOR_TYPE, CodeSlayerEditorClass))
#define IS_CODESLAYER_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_EDITOR_TYPE))
#define IS_CODESLAYER_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_EDITOR_TYPE))

typedef struct _CodeSlayerEditor CodeSlayerEditor;
typedef struct _CodeSlayerEditorClass CodeSlayerEditorClass;

struct _CodeSlayerEditor
{
  GtkSourceView parent_instance;
};

struct _CodeSlayerEditorClass
{
  GtkSourceViewClass parent_class;

  void (*copy_lines) (CodeSlayerEditor *editor);
  void (*to_uppercase) (CodeSlayerEditor *editor);
  void (*to_lowercase) (CodeSlayerEditor *editor);
};

GType codeslayer_editor_get_type (void) G_GNUC_CONST;

GtkWidget*           codeslayer_editor_new               (CodeSlayerDocument    *document,
                                                          CodeSlayerPreferences *preferences);

void                 codeslayer_editor_scroll_to_line    (CodeSlayerEditor      *editor,
                                                          gint                   line_number);
void                 codeslayer_editor_sync_preferences  (CodeSlayerEditor      *editor);

CodeSlayerDocument*  codeslayer_editor_get_document      (CodeSlayerEditor      *editor);

G_END_DECLS

#endif /* __CODESLAYER_EDITOR_H__ */
