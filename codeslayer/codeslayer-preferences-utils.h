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

#ifndef __CODESLAYER_PREFERENCES_UTILS_H__
#define	__CODESLAYER_PREFERENCES_UTILS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-preferences.h>

G_BEGIN_DECLS

GtkBox*  codeslayer_preferences_utils_content_area  (GtkBox             *tab, 
                                                     gchar              *title);                                                 
void     codeslayer_preferences_utils_notify        (CodeSlayerRegistry *registry);

G_END_DECLS

#endif /* __CODESLAYER_PREFERENCES_UTILS_H__ */
