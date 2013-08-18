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

#ifndef __CODESLAYER_ABSTRACT_PANE_H__
#define	__CODESLAYER_ABSTRACT_PANE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-preferences.h>

G_BEGIN_DECLS

#define CODESLAYER_ABSTRACT_PANE_TYPE            (codeslayer_abstract_pane_get_type ())
#define CODESLAYER_ABSTRACT_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_ABSTRACT_PANE_TYPE, CodeSlayerAbstractPane))
#define CODESLAYER_ABSTRACT_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_ABSTRACT_PANE_TYPE, CodeSlayerAbstractPaneClass))
#define IS_CODESLAYER_ABSTRACT_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_ABSTRACT_PANE_TYPE))
#define IS_CODESLAYER_ABSTRACT_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_ABSTRACT_PANE_TYPE))
#define CODESLAYER_ABSTRACT_PANE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CODESLAYER_ABSTRACT_PANE_TYPE, CodeSlayerAbstractPaneClass))

typedef struct _CodeSlayerAbstractPane CodeSlayerAbstractPane;
typedef struct _CodeSlayerAbstractPaneClass CodeSlayerAbstractPaneClass;

struct _CodeSlayerAbstractPane
{
  GtkVBox parent_instance;
};

struct _CodeSlayerAbstractPaneClass
{
  GtkVBoxClass parent_class;

  void (*open_pane) (CodeSlayerAbstractPane *abstract_pane);
  void (*close_pane) (CodeSlayerAbstractPane *abstract_pane);
  gchar* (*abstract_tab_position_key) (CodeSlayerAbstractPane *abstract_pane);
  gint   (*abstract_tab_start_sort) (CodeSlayerAbstractPane *abstract_pane);
};

GType codeslayer_abstract_pane_get_type (void) G_GNUC_CONST;

void        codeslayer_abstract_pane_create_notebook   (CodeSlayerAbstractPane *abstract_pane);
void        codeslayer_abstract_pane_sync_preferences  (CodeSlayerAbstractPane *abstract_pane);
void        codeslayer_abstract_pane_set_preferences   (CodeSlayerAbstractPane *abstract_pane, 
                                                        CodeSlayerPreferences  *preferences);
void        codeslayer_abstract_pane_add               (CodeSlayerAbstractPane *abstract_pane, 
                                                        GtkWidget              *widget,
                                                        const gchar            *title);
void        codeslayer_abstract_pane_insert            (CodeSlayerAbstractPane *abstract_pane, 
                                                        GtkWidget              *widget,
                                                        const gchar            *title, 
                                                        gint                    position);
void        codeslayer_abstract_pane_remove            (CodeSlayerAbstractPane *abstract_pane, 
                                                        GtkWidget              *widget);                                         

void        codeslayer_abstract_pane_open              (CodeSlayerAbstractPane *abstract_pane,
                                                        GtkWidget              *widget);
void        codeslayer_abstract_pane_close             (CodeSlayerAbstractPane *abstract_pane);

G_END_DECLS

#endif /* __CODESLAYER_ABSTRACT_PANE_H__ */
