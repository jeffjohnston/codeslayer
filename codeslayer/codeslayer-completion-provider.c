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

#include <codeslayer/codeslayer-completion-provider.h>

static void codeslayer_completion_provider_class_init (gpointer provider);

GType
codeslayer_completion_provider_get_type ()
{
  static GType type = 0;
  
  if (!type)
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE, "CodeSlayerCompletionProvider", 
                                            sizeof (CodeSlayerCompletionProviderInterface), 
                                            (GClassInitFunc) codeslayer_completion_provider_class_init,
                                            0, NULL, 0);
                                            
      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
    }
  
  return type;  
}

static void
codeslayer_completion_provider_class_init (gpointer page)
{
}

gboolean 
codeslayer_completion_provider_has_match (CodeSlayerCompletionProvider *provider, 
                                          GtkTextIter                  *iter)
{
  return CODESLAYER_COMPLETION_PROVIDER_GET_INTERFACE (provider)->has_match (provider, iter);
}

GList*  
codeslayer_completion_provider_get_proposals (CodeSlayerCompletionProvider *provider, 
                                              GtkTextIter                  *iter)
{
  return CODESLAYER_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_proposals (provider, iter);
}
