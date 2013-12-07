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

#ifndef __CODESLAYER_NOTEBOOK_TAB_H__
#define	__CODESLAYER_NOTEBOOK_TAB_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_NOTEBOOK_TAB_TYPE            (codeslayer_notebook_tab_get_type ())
#define CODESLAYER_NOTEBOOK_TAB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_NOTEBOOK_TAB_TYPE, CodeSlayerNotebookTab))
#define CODESLAYER_NOTEBOOK_TAB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_NOTEBOOK_TAB_TYPE, CodeSlayerNotebookTabClass))
#define IS_CODESLAYER_NOTEBOOK_TAB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_NOTEBOOK_TAB_TYPE))
#define IS_CODESLAYER_NOTEBOOK_TAB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_NOTEBOOK_TAB_TYPE))

typedef struct _CodeSlayerNotebookTab CodeSlayerNotebookTab;
typedef struct _CodeSlayerNotebookTabClass CodeSlayerNotebookTabClass;

struct _CodeSlayerNotebookTab
{
  GtkHBox parent_instance;
};

struct _CodeSlayerNotebookTabClass
{
  GtkHBoxClass parent_class;

  void (*select_document) (CodeSlayerNotebookTab *notebook_tab);
  void (*close_document) (CodeSlayerNotebookTab *notebook_tab);
  void (*close_all_documents) (CodeSlayerNotebookTab *notebook_tab);
  void (*close_other_documents) (CodeSlayerNotebookTab *notebook_tab);
  void (*close_right_documents) (CodeSlayerNotebookTab *notebook_tab);
  void (*close_left_documents) (CodeSlayerNotebookTab *notebook_tab);
};

GType codeslayer_notebook_tab_get_type (void) G_GNUC_CONST;
  
GtkWidget*  codeslayer_notebook_tab_new                (GtkWidget             *notebook, 
                                                        const gchar           *name);
void        codeslayer_notebook_tab_show_buffer_dirty  (CodeSlayerNotebookTab *notebook_tab);
void        codeslayer_notebook_tab_show_buffer_clean  (CodeSlayerNotebookTab *notebook_tab);
GtkWidget*  codeslayer_notebook_tab_get_notebook_page  (CodeSlayerNotebookTab *notebook_tab);
void        codeslayer_notebook_tab_set_notebook_page  (CodeSlayerNotebookTab *notebook_tab, 
                                                        GtkWidget             *notebook_page);
void        codeslayer_notebook_tab_set_label_name     (CodeSlayerNotebookTab *notebook_tab, 
                                                        gchar                 *name);

G_END_DECLS

#endif /* __CODESLAYER_NOTEBOOK_TAB_H__ */
