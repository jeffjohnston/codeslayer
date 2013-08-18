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

#ifndef __CODESLAYER_REPOSITORY_H__
#define __CODESLAYER_REPOSITORY_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-group.h>
#include <codeslayer/codeslayer-project.h>

G_BEGIN_DECLS 

CodeSlayerGroup*  codeslayer_repository_get_group    (GFile   *file);
GList*            codeslayer_repository_get_plugins  (GObject *data);

G_END_DECLS

#endif /* __CODESLAYER_REPOSITORY_H__ */
