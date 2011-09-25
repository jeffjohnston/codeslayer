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

#ifndef __CODESLAYER_GROUPS_H__
#define	__CODESLAYER_GROUPS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-group.h>

G_BEGIN_DECLS

#define CODESLAYER_GROUPS_TYPE            (codeslayer_groups_get_type ())
#define CODESLAYER_GROUPS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_GROUPS_TYPE, CodeSlayerGroups))
#define CODESLAYER_GROUPS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_GROUPS_TYPE, CodeSlayerGroupsClass))
#define IS_CODESLAYER_GROUPS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_GROUPS_TYPE))
#define IS_CODESLAYER_GROUPS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_GROUPS_TYPE))

#define CODESLAYER_GROUPS_DIR "groups"
#define CODESLAYER_GROUPS_ACTVIE "active"
#define CODESLAYER_GROUPS_CONF "groups.conf"
#define CODESLAYER_DEFAULT_GROUP_DIR "default"

typedef struct _CodeSlayerGroups CodeSlayerGroups;
typedef struct _CodeSlayerGroupsClass CodeSlayerGroupsClass;

struct _CodeSlayerGroups
{
  GObject parent_instance;
};

struct _CodeSlayerGroupsClass
{
  GObjectClass parent_class;
};

GType codeslayer_groups_get_type (void) G_GNUC_CONST;

CodeSlayerGroups*  codeslayer_groups_new (void);

GList*            codeslayer_groups_get_list          (CodeSlayerGroups *groups);
void              codeslayer_groups_set_list          (CodeSlayerGroups *groups, 
                                                       GList            *list);
void              codeslayer_groups_add_group         (CodeSlayerGroups *groups, 
                                                       CodeSlayerGroup  *group);
void              codeslayer_groups_remove_group      (CodeSlayerGroups *groups, 
                                                       CodeSlayerGroup  *group);
CodeSlayerGroup*  codeslayer_groups_find_group        (CodeSlayerGroups *groups, 
                                                       const gchar      *name);
CodeSlayerGroup*  codeslayer_groups_find_next_group   (CodeSlayerGroups *groups, 
                                                       const gchar      *name);
CodeSlayerGroup*  codeslayer_groups_get_active_group  (CodeSlayerGroups *groups);
void              codeslayer_groups_set_active_group  (CodeSlayerGroups *groups, 
                                                       CodeSlayerGroup  *group);

G_END_DECLS

#endif /* __CODESLAYER_GROUPS_H__ */
