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

#ifndef __CODESLAYER_PREFERENCES_H__
#define __CODESLAYER_PREFERENCES_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-profile.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-profiles.h>

G_BEGIN_DECLS

#define CODESLAYER_PREFERENCES_TYPE            (codeslayer_preferences_get_type ())
#define CODESLAYER_PREFERENCES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PREFERENCES_TYPE, CodeSlayerPreferences))
#define CODESLAYER_PREFERENCES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PREFERENCES_TYPE, CodeSlayerPreferencesClass))
#define IS_CODESLAYER_PREFERENCES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PREFERENCES_TYPE))
#define IS_CODESLAYER_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PREFERENCES_TYPE))

typedef struct _CodeSlayerPreferences CodeSlayerPreferences;
typedef struct _CodeSlayerPreferencesClass CodeSlayerPreferencesClass;

struct _CodeSlayerPreferences
{
  GObject parent_instance;
};

struct _CodeSlayerPreferencesClass
{
  GObjectClass parent_class;
};

GType codeslayer_preferences_get_type (void) G_GNUC_CONST;

CodeSlayerPreferences*  codeslayer_preferences_new          (GtkWidget             *window, 
                                                             CodeSlayerProfile     *profile);

void                    codeslayer_preferences_run_dialog   (CodeSlayerPreferences *preferences);

G_END_DECLS

#endif /* __CODESLAYER_PREFERENCES_H__ */
