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

#ifndef __CODESLAYER_SEARCH_TAB_H__
#define	__CODESLAYER_SEARCH_TAB_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_SEARCH_TAB_TYPE            (codeslayer_search_tab_get_type ())
#define CODESLAYER_SEARCH_TAB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_SEARCH_TAB_TYPE, CodeSlayerSearchTab))
#define CODESLAYER_SEARCH_TAB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_SEARCH_TAB_TYPE, CodeSlayerSearchTabClass))
#define IS_CODESLAYER_SEARCH_TAB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_SEARCH_TAB_TYPE))
#define IS_CODESLAYER_SEARCH_TAB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_SEARCH_TAB_TYPE))

typedef struct _CodeSlayerSearchTab CodeSlayerSearchTab;
typedef struct _CodeSlayerSearchTabClass CodeSlayerSearchTabClass;

struct _CodeSlayerSearchTab
{
  GtkHBox parent_instance;
};

struct _CodeSlayerSearchTabClass
{
  GtkHBoxClass parent_class;

  void (*close_page) (CodeSlayerSearchTab *search_tab);
};

GType codeslayer_search_tab_get_type (void) G_GNUC_CONST;
  
GtkWidget*  codeslayer_search_tab_new              (gboolean closeable);

GtkWidget*  codeslayer_search_tab_get_search_page  (CodeSlayerSearchTab  *search_tab);
void        codeslayer_search_tab_set_search_page  (CodeSlayerSearchTab  *search_tab, 
                                                    GtkWidget            *search_page);

void        codeslayer_search_tab_set_label_name   (CodeSlayerSearchTab  *search_tab, 
                                                    const gchar          *name);
void        codeslayer_search_tab_set_tooltip      (CodeSlayerSearchTab  *search_tab, 
                                                    const gchar          *tooltip);

G_END_DECLS

#endif /* __CODESLAYER_SEARCH_TAB_H__ */
