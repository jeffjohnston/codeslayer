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

#ifndef __CODESLAYER_PROFILES_H__
#define	__CODESLAYER_PROFILES_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-profile.h>

G_BEGIN_DECLS

#define CODESLAYER_PROFILES_TYPE            (codeslayer_profiles_get_type ())
#define CODESLAYER_PROFILES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROFILES_TYPE, CodeSlayerProfiles))
#define CODESLAYER_PROFILES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROFILES_TYPE, CodeSlayerProfilesClass))
#define IS_CODESLAYER_PROFILES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROFILES_TYPE))
#define IS_CODESLAYER_PROFILES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROFILES_TYPE))

#define CODESLAYER_PROFILES_DIR "profiles"

typedef struct _CodeSlayerProfiles CodeSlayerProfiles;
typedef struct _CodeSlayerProfilesClass CodeSlayerProfilesClass;

struct _CodeSlayerProfiles
{
  GObject parent_instance;
};

struct _CodeSlayerProfilesClass
{
  GObjectClass parent_class;
};

GType
codeslayer_profiles_get_type (void) G_GNUC_CONST;

CodeSlayerProfiles*  codeslayer_profiles_new                   (void);
                                               
CodeSlayerProfile*   codeslayer_profiles_get_profile           (CodeSlayerProfiles *profiles);
CodeSlayerProfile*   codeslayer_profiles_load_default_profile  (CodeSlayerProfiles *profiles);
CodeSlayerProfile*   codeslayer_profiles_load_new_profile      (CodeSlayerProfiles *profiles, 
                                                                GFile              *file);
CodeSlayerProfile*   codeslayer_profiles_load_file_profile     (CodeSlayerProfiles *profiles, 
                                                                GFile              *file);
void                 codeslayer_profiles_save_profile          (CodeSlayerProfiles *profiles);
GObject*             codeslayer_profiles_get_registry          (CodeSlayerProfiles *profiles);

G_END_DECLS

#endif /* __CODESLAYER_PROFILES_H__ */
