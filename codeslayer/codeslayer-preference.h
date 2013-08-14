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

#ifndef __CODESLAYER_PREFERENCE_H__
#define	__CODESLAYER_PREFERENCE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_PREFERENCE_TYPE            (codeslayer_preference_get_type ())
#define CODESLAYER_PREFERENCE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PREFERENCE_TYPE, CodeSlayerPreference))
#define CODESLAYER_PREFERENCE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PREFERENCE_TYPE, CodeSlayerPreferenceClass))
#define IS_CODESLAYER_PREFERENCE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PREFERENCE_TYPE))
#define IS_CODESLAYER_PREFERENCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PREFERENCE_TYPE))

typedef struct _CodeSlayerPreference CodeSlayerPreference;
typedef struct _CodeSlayerPreferenceClass CodeSlayerPreferenceClass;

struct _CodeSlayerPreference
{
  GObject parent_instance;
};

struct _CodeSlayerPreferenceClass
{
  GObjectClass parent_class;
};

GType codeslayer_preference_get_type (void) G_GNUC_CONST;

CodeSlayerPreference*  codeslayer_preference_new        (void);

const gchar*           codeslayer_preference_get_name   (CodeSlayerPreference *preference);
void                   codeslayer_preference_set_name   (CodeSlayerPreference *preference,
                                                         const gchar          *name);
const gchar*           codeslayer_preference_get_value  (CodeSlayerPreference *preference);
void                   codeslayer_preference_set_value  (CodeSlayerPreference *preference,
                                                         const gchar          *value);

G_END_DECLS

#endif /* __CODESLAYER_PREFERENCE_H__ */
