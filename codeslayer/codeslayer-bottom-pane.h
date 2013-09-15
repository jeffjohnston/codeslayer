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

#ifndef __CODESLAYER_BOTTOM_PANE_H__
#define	__CODESLAYER_BOTTOM_PANE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-abstract-pane.h>

G_BEGIN_DECLS

#define CODESLAYER_BOTTOM_PANE_TYPE            (codeslayer_bottom_pane_get_type ())
#define CODESLAYER_BOTTOM_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_BOTTOM_PANE_TYPE, CodeSlayerBottomPane))
#define CODESLAYER_BOTTOM_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_BOTTOM_PANE_TYPE, CodeSlayerBottomPaneClass))
#define IS_CODESLAYER_BOTTOM_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_BOTTOM_PANE_TYPE))
#define IS_CODESLAYER_BOTTOM_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_BOTTOM_PANE_TYPE))

typedef struct _CodeSlayerBottomPane CodeSlayerBottomPane;
typedef struct _CodeSlayerBottomPaneClass CodeSlayerBottomPaneClass;

struct _CodeSlayerBottomPane
{
  CodeSlayerAbstractPane parent_instance;
};

struct _CodeSlayerBottomPaneClass
{
  CodeSlayerAbstractPaneClass parent_class;
};

GType codeslayer_bottom_pane_get_type (void) G_GNUC_CONST;

GtkWidget*  codeslayer_bottom_pane_new  (CodeSlayerRegistry *registry);

G_END_DECLS

#endif /* __CODESLAYER_BOTTOM_PANE_H__ */
