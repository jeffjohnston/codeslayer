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

#ifndef __CODESLAYER_COMPLETION_PROVIDER_H__
#define	__CODESLAYER_COMPLETION_PROVIDER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_COMPLETION_PROVIDER_TYPE                (codeslayer_completion_provider_get_type ())
#define CODESLAYER_COMPLETION_PROVIDER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_COMPLETION_PROVIDER_TYPE, CodeSlayerCompletionProvider))
#define IS_CODESLAYER_COMPLETION_PROVIDER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_COMPLETION_PROVIDER_TYPE))
#define CODESLAYER_COMPLETION_PROVIDER_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), CODESLAYER_COMPLETION_PROVIDER_TYPE, CodeSlayerCompletionProviderInterface))

typedef struct _CodeSlayerCompletionProvider CodeSlayerCompletionProvider;
typedef struct _CodeSlayerCompletionProviderInterface CodeSlayerCompletionProviderInterface;

struct _CodeSlayerCompletionProviderInterface
{
  GTypeInterface parent;
  
  gboolean (*has_match) (CodeSlayerCompletionProvider *provider, GtkTextIter iter);
  GList* (*get_proposals) (CodeSlayerCompletionProvider *provider, GtkTextIter iter);
};

GType codeslayer_completion_provider_get_type (void);

GList*    codeslayer_completion_provider_get_proposals  (CodeSlayerCompletionProvider *provider, 
                                                         GtkTextIter                   iter);

G_END_DECLS

#endif /* __CODESLAYER_COMPLETION_PROVIDER_H__ */
