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

#ifndef __CODESLAYER_COMPLETION_H__
#define	__CODESLAYER_COMPLETION_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-completion-provider.h>

G_BEGIN_DECLS

#define CODESLAYER_COMPLETION_TYPE            (codeslayer_completion_get_type ())
#define CODESLAYER_COMPLETION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_COMPLETION_TYPE, CodeSlayerCompletion))
#define CODESLAYER_COMPLETION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_COMPLETION_TYPE, CodeSlayerCompletionClass))
#define IS_CODESLAYER_COMPLETION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_COMPLETION_TYPE))
#define IS_CODESLAYER_COMPLETION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_COMPLETION_TYPE))

typedef struct _CodeSlayerCompletion CodeSlayerCompletion;
typedef struct _CodeSlayerCompletionClass CodeSlayerCompletionClass;

struct _CodeSlayerCompletion
{
  GObject parent_instance;
};

struct _CodeSlayerCompletionClass
{
  GObjectClass parent_class;
};

GType codeslayer_completion_get_type (void) G_GNUC_CONST;

CodeSlayerCompletion*  codeslayer_completion_new           (void);

void                   codeslayer_completion_add_provider  (CodeSlayerCompletion         *completion, 
                                                            CodeSlayerCompletionProvider *provider);

void                   codeslayer_completion_invoke        (CodeSlayerCompletion         *completion, 
                                                            GtkTextIter                   iter);

G_END_DECLS

#endif /* __CODESLAYER_COMPLETION_H__ */
