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

#ifndef __CODESLAYER_MENU_BAR_FILE_H__
#define	__CODESLAYER_MENU_BAR_FILE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-profile.h>

G_BEGIN_DECLS

#define CODESLAYER_MENU_BAR_FILE_TYPE            (codeslayer_menu_bar_file_get_type ())
#define CODESLAYER_MENU_BAR_FILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_MENU_BAR_FILE_TYPE, CodeSlayerMenuBarFile))
#define CODESLAYER_MENU_BAR_FILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_MENU_BAR_FILE_TYPE, CodeSlayerMenuBarFileClass))
#define IS_CODESLAYER_MENU_BAR_FILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_MENU_BAR_FILE_TYPE))
#define IS_CODESLAYER_MENU_BAR_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_MENU_BAR_FILE_TYPE))

typedef struct _CodeSlayerMenuBarFile CodeSlayerMenuBarFile;
typedef struct _CodeSlayerMenuBarFileClass CodeSlayerMenuBarFileClass;

struct _CodeSlayerMenuBarFile
{
  GtkMenuItem parent_instance;
};

struct _CodeSlayerMenuBarFileClass
{
  GtkMenuItemClass parent_class;
};

GType codeslayer_menu_bar_file_get_type (void) G_GNUC_CONST;
  
GtkWidget*  codeslayer_menu_bar_file_new  (GtkWidget         *menu_bar, 
                                           GtkAccelGroup     *accel_group, 
                                           CodeSlayerProfile *profile);

G_END_DECLS

#endif /* __CODESLAYER_MENU_BAR_FILE_H__ */
