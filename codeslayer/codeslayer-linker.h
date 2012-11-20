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

#ifndef __CODESLAYER_LINKER_H__
#define	__CODESLAYER_LINKER_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define CODESLAYER_LINKER_TYPE            (codeslayer_linker_get_type ())
#define CODESLAYER_LINKER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_LINKER_TYPE, CodeSlayerLinker))
#define CODESLAYER_LINKER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_LINKER_TYPE, CodeSlayerLinkerClass))
#define IS_CODESLAYER_LINKER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_LINKER_TYPE))
#define IS_CODESLAYER_LINKER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_LINKER_TYPE))

typedef struct _CodeSlayerLinker CodeSlayerLinker;
typedef struct _CodeSlayerLinkerClass CodeSlayerLinkerClass;

struct _CodeSlayerLinker
{
  GObject parent_instance;
};

struct _CodeSlayerLinkerClass
{
  GObjectClass parent_class;
};

GType codeslayer_linker_get_type (void) G_GNUC_CONST;

CodeSlayerLinker*  codeslayer_linker_new               (CodeSlayer       *codeslayer, 
                                                        GtkTextView      *text_view);
CodeSlayerLinker*  codeslayer_linker_new_with_pattern  (CodeSlayer       *codeslayer, 
                                                        GtkTextView      *text_view,
                                                        gchar            *pattern);
                                                    
const gchar *      codeslayer_linker_get_pattern       (CodeSlayerLinker *linker);
void               codeslayer_linker_set_pattern       (CodeSlayerLinker *linker, 
                                                        const gchar      *pattern);

G_END_DECLS

#endif /* __CODESLAYER_LINKER_H__ */
