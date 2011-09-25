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

#ifndef __CODESLAYER_MENU_ITEM_H__
#define	__CODESLAYER_MENU_ITEM_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-project.h>

G_BEGIN_DECLS

#define CODESLAYER_MENU_ITEM_TYPE            (codeslayer_menu_item_get_type ())
#define CODESLAYER_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_MENU_ITEM_TYPE, CodeSlayerMenuItem))
#define CODESLAYER_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_MENU_ITEM_TYPE, CodeSlayerMenuItemClass))
#define IS_CODESLAYER_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_MENU_ITEM_TYPE))
#define IS_CODESLAYER_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_MENU_ITEM_TYPE))

typedef struct _CodeSlayerMenuItem CodeSlayerMenuItem;
typedef struct _CodeSlayerMenuItemClass CodeSlayerMenuItemClass;

struct _CodeSlayerMenuItem
{
  GtkMenuItem parent_instance;
};

struct _CodeSlayerMenuItemClass
{
  GtkMenuItemClass parent_class;
  
  void (*projects_menu_selected) (CodeSlayerMenuItem *menuitem);  
};

GType codeslayer_menu_item_get_type (void) G_GNUC_CONST;

GtkWidget*  codeslayer_menu_item_new             (void);

GtkWidget*  codeslayer_menu_item_new_with_label  (const gchar *label);

G_END_DECLS

#endif /* __CODESLAYER_MENU_ITEM_H__ */
