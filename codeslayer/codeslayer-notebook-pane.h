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

#ifndef __CODESLAYER_NOTEBOOK_PANE_H__
#define	__CODESLAYER_NOTEBOOK_PANE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_NOTEBOOK_PANE_TYPE            (codeslayer_notebook_pane_get_type ())
#define CODESLAYER_NOTEBOOK_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_NOTEBOOK_PANE_TYPE, CodeSlayerNotebookPane))
#define CODESLAYER_NOTEBOOK_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_NOTEBOOK_PANE_TYPE, CodeSlayerNotebookPaneClass))
#define IS_CODESLAYER_NOTEBOOK_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_NOTEBOOK_PANE_TYPE))
#define IS_CODESLAYER_NOTEBOOK_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_NOTEBOOK_PANE_TYPE))

typedef struct _CodeSlayerNotebookPane CodeSlayerNotebookPane;
typedef struct _CodeSlayerNotebookPaneClass CodeSlayerNotebookPaneClass;

struct _CodeSlayerNotebookPane
{
  GtkVBox parent_instance;
};

struct _CodeSlayerNotebookPaneClass
{
  GtkVBoxClass parent_class;

  void (*close_page) (CodeSlayerNotebookPane * notebook_pane);
  void (*find_next) (CodeSlayerNotebookPane * notebook_pane);
  void (*find_undo_incremental) (CodeSlayerNotebookPane * notebook_pane);
};

GType codeslayer_notebook_pane_get_type (void) G_GNUC_CONST;

GtkWidget*  codeslayer_notebook_pane_new (void);
GtkWidget*  codeslayer_notebook_pane_get_notebook          (CodeSlayerNotebookPane   *notebook_pane);
void        codeslayer_notebook_pane_set_notebook          (CodeSlayerNotebookPane   *notebook_pane, 
                                                            GtkWidget       *notebook);
GtkWidget*  codeslayer_notebook_pane_get_notebook_search   (CodeSlayerNotebookPane   *notebook_pane);
void        codeslayer_notebook_pane_set_notebook_search   (CodeSlayerNotebookPane   *notebook_pane,
                                                            GtkWidget                *notebook_search);
void        codeslayer_notebook_pane_search_find           (CodeSlayerNotebookPane   *notebook_pane);
void        codeslayer_notebook_pane_search_replace        (CodeSlayerNotebookPane   *notebook_pane);
void        codeslayer_notebook_pane_search_find_next      (CodeSlayerNotebookPane   *notebook_pane);
void        codeslayer_notebook_pane_search_find_previous  (CodeSlayerNotebookPane   *notebook_pane);

G_END_DECLS

#endif /* __CODESLAYER_NOTEBOOK_PANE_H__ */
