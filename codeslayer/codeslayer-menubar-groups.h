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

#ifndef __CODESLAYER_MENU_BAR_GROUPS_H__
#define	__CODESLAYER_MENU_BAR_GROUPS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-plugins.h>
#include <codeslayer/codeslayer-group.h>
#include <codeslayer/codeslayer-preferences.h>

G_BEGIN_DECLS

#define CODESLAYER_MENU_BAR_GROUPS_TYPE            (codeslayer_menu_bar_groups_get_type ())
#define CODESLAYER_MENU_BAR_GROUPS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_MENU_BAR_GROUPS_TYPE, CodeSlayerMenuBarGroups))
#define CODESLAYER_MENU_BAR_GROUPS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_MENU_BAR_GROUPS_TYPE, CodeSlayerMenuBarGroupsClass))
#define IS_CODESLAYER_MENU_BAR_GROUPS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_MENU_BAR_GROUPS_TYPE))
#define IS_CODESLAYER_MENU_BAR_GROUPS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_MENU_BAR_GROUPS_TYPE))

typedef struct _CodeSlayerMenuBarGroups CodeSlayerMenuBarGroups;
typedef struct _CodeSlayerMenuBarGroupsClass CodeSlayerMenuBarGroupsClass;

struct _CodeSlayerMenuBarGroups
{
  GtkMenuItem parent_instance;
};

struct _CodeSlayerMenuBarGroupsClass
{
  GtkMenuItemClass parent_class;
};

GType codeslayer_menu_bar_groups_get_type (void) G_GNUC_CONST;
  
GtkWidget*  codeslayer_menu_bar_groups_new             (GtkWidget                *window, 
                                                        GtkWidget                *menu_bar, 
                                                        GtkAccelGroup            *accel_group,
                                                        CodeSlayerGroups         *groups);
void        codeslayer_menu_bar_groups_refresh_groups  (CodeSlayerMenuBarGroups  *menu_bar_groups,
                                                        CodeSlayerGroups         *groups);

G_END_DECLS

#endif /* __CODESLAYER_MENU_BAR_GROUPS_H__ */
