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

#ifndef __CODESLAYER_PROFILES_MANAGER_H__
#define __CODESLAYER_PROFILES_MANAGER_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-profile.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-profiles.h>

G_BEGIN_DECLS

#define CODESLAYER_PROFILES_MANAGER_TYPE            (codeslayer_profiles_manager_get_type ())
#define CODESLAYER_PROFILES_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROFILES_MANAGER_TYPE, CodeSlayerProfilesManager))
#define CODESLAYER_PROFILES_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROFILES_MANAGER_TYPE, CodeSlayerProfilesManagerClass))
#define IS_CODESLAYER_PROFILES_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROFILES_MANAGER_TYPE))
#define IS_CODESLAYER_PROFILES_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROFILES_MANAGER_TYPE))

typedef struct _CodeSlayerProfilesManager CodeSlayerProfilesManager;
typedef struct _CodeSlayerProfilesManagerClass CodeSlayerProfilesManagerClass;

struct _CodeSlayerProfilesManager
{
  GObject parent_instance;
};

struct _CodeSlayerProfilesManagerClass
{
  GObjectClass parent_class;
};

GType codeslayer_profiles_manager_get_type (void) G_GNUC_CONST;

CodeSlayerProfilesManager*  codeslayer_profiles_manager_new         (GtkWidget                 *window, 
                                                                     CodeSlayerProfiles        *profiles, 
                                                                     CodeSlayerRegistry        *registry);

void                        codeslayer_profiles_manager_run_dialog  (CodeSlayerProfilesManager *profiles_manager);

G_END_DECLS

#endif /* __CODESLAYER_PROFILES_MANAGER_H__ */
