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

#ifndef __CODESLAYER_EDITOR_LINKER_H__
#define	__CODESLAYER_EDITOR_LINKER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_EDITOR_LINKER_TYPE            (codeslayer_editor_linker_get_type ())
#define CODESLAYER_EDITOR_LINKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_EDITOR_LINKER_TYPE, CodeSlayerEditorLinker))
#define CODESLAYER_EDITOR_LINKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_EDITOR_LINKER_TYPE, CodeSlayerEditorLinkerClass))
#define IS_CODESLAYER_EDITOR_LINKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_EDITOR_LINKER_TYPE))
#define IS_CODESLAYER_EDITOR_LINKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_EDITOR_LINKER_TYPE))

typedef struct _CodeSlayerEditorLinker CodeSlayerEditorLinker;
typedef struct _CodeSlayerEditorLinkerClass CodeSlayerEditorLinkerClass;

struct _CodeSlayerEditorLinker
{
  GObject parent_instance;
};

struct _CodeSlayerEditorLinkerClass
{
  GObjectClass parent_class;
};

GType codeslayer_editor_linker_get_type (void) G_GNUC_CONST;

CodeSlayerEditorLinker*  codeslayer_editor_linker_new           (GObject                *codeslayer, 
                                                                 GtkTextView            *text_view);
const gchar *            codeslayer_editor_linker_get_pattern   (CodeSlayerEditorLinker *linker);
void                     codeslayer_editor_linker_set_pattern   (CodeSlayerEditorLinker *linker, 
                                                                 const gchar            *pattern);
gint                     codeslayer_editor_linker_create_links  (CodeSlayerEditorLinker *linker);

G_END_DECLS

#endif /* __CODESLAYER_EDITOR_LINKER_H__ */
