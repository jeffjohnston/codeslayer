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
#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-group.h>
#include <codeslayer/codeslayer-project.h>

G_BEGIN_DECLS 

CodeSlayerGroups*  codeslayer_repository_get_groups      (void);
void               codeslayer_repository_save_groups     (CodeSlayerGroups  *groups);

void               codeslayer_repository_create_group    (CodeSlayerGroup   *group);
void               codeslayer_repository_delete_group    (CodeSlayerGroup   *group);
void               codeslayer_repository_rename_group    (CodeSlayerGroup   *group, 
                                                          const gchar       *name);

GList*             codeslayer_repository_get_projects    (CodeSlayerGroup   *group);
void               codeslayer_repository_save_projects   (CodeSlayerGroup   *group);

GList*             codeslayer_repository_get_libs        (CodeSlayerGroup   *group);
void               codeslayer_repository_save_libs       (CodeSlayerGroup   *group);

GList*             codeslayer_repository_get_documents   (CodeSlayerGroup   *group);
void               codeslayer_repository_save_documents  (CodeSlayerGroup   *group, 
                                                          GList             *documents);

GList*             codeslayer_repository_get_plugins     (GObject           *data);


G_END_DECLS

#endif /* __CODESLAYER_REPOSITORY_H__ */
