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

#ifndef __CODESLAYER_WINDOW_H__
#define	__CODESLAYER_WINDOW_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-profile.h>

G_BEGIN_DECLS

#define CODESLAYER_WINDOW_TYPE            (codeslayer_window_get_type ())
#define CODESLAYER_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_WINDOW_TYPE, CodeSlayerWindow))
#define CODESLAYER_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_WINDOW_TYPE, CodeSlayerWindowClass))
#define IS_CODESLAYER_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_WINDOW_TYPE))
#define IS_CODESLAYER_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_WINDOW_TYPE))

typedef struct _CodeSlayerWindow CodeSlayerWindow;
typedef struct _CodeSlayerWindowClass CodeSlayerWindowClass;

struct _CodeSlayerWindow
{
  GtkWindow parent_instance;
};

struct _CodeSlayerWindowClass
{
  GtkWindowClass parent_class;
};

GType
codeslayer_window_get_type (void) G_GNUC_CONST;
  
GtkWidget*          codeslayer_window_new          (GtkApplication   *application, 
                                                    gchar            *profile_name);                                    
void                codeslayer_window_open_editor  (CodeSlayerWindow *window, 
                                                    gchar            *file_path);
CodeSlayerProfile*  codeslayer_window_get_profile  (CodeSlayerWindow *window);
                                    

G_END_DECLS

#endif /* __CODESLAYER_WINDOW_H__ */
