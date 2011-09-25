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

#ifndef __CODESLAYER_MENUBAR_SEARCH_H__
#define	__CODESLAYER_MENUBAR_SEARCH_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_MENUBAR_SEARCH_TYPE            (codeslayer_menubar_search_get_type ())
#define CODESLAYER_MENUBAR_SEARCH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_MENUBAR_SEARCH_TYPE, CodeSlayerMenuBarSearch))
#define CODESLAYER_MENUBAR_SEARCH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_MENUBAR_SEARCH_TYPE, CodeSlayerMenuBarSearchClass))
#define IS_CODESLAYER_MENUBAR_SEARCH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_MENUBAR_SEARCH_TYPE))
#define IS_CODESLAYER_MENUBAR_SEARCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_MENUBAR_SEARCH_TYPE))

typedef struct _CodeSlayerMenuBarSearch CodeSlayerMenuBarSearch;
typedef struct _CodeSlayerMenuBarSearchClass CodeSlayerMenuBarSearchClass;

struct _CodeSlayerMenuBarSearch
{
  GtkMenuItem parent_instance;
};

struct _CodeSlayerMenuBarSearchClass
{
  GtkMenuItemClass parent_class;
};

GType codeslayer_menubar_search_get_type (void) G_GNUC_CONST;
  
GtkWidget*  codeslayer_menubar_search_new                (GtkWidget               *menubar, 
                                                          GtkAccelGroup           *accel_group);
                                            
void        codeslayer_menubar_search_sync_with_notebook (CodeSlayerMenuBarSearch *menubar_search,
                                                          GtkWidget               *notebook);

G_END_DECLS

#endif /* __CODESLAYER_MENUBAR_SEARCH_H__ */
