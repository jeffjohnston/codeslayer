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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <codeslayer/codeslayer-profiles-manager.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-profiles-manager
 * @short_description: The profiles.
 * @title: CodeSlayerProfilesManager
 * @include: codeslayer/codeslayer-profiles-manager.h
 */

static void codeslayer_profiles_manager_class_init  (CodeSlayerProfilesManagerClass *klass);
static void codeslayer_profiles_manager_init        (CodeSlayerProfilesManager      *profiles_manager);
static void codeslayer_profiles_manager_finalize    (CodeSlayerProfilesManager      *profiles_manager);

#define CODESLAYER_PROFILES_MANAGER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROFILES_MANAGER_TYPE, CodeSlayerProfilesManagerPrivate))

typedef struct _CodeSlayerProfilesManagerPrivate CodeSlayerProfilesManagerPrivate;

struct _CodeSlayerProfilesManagerPrivate
{
  GtkWidget                *window;
  CodeSlayerProfiles       *profiles;
  CodeSlayerEngine         *engine;
  CodeSlayerProjectsEngine *projects_engine;
};

G_DEFINE_TYPE (CodeSlayerProfilesManager, codeslayer_profiles_manager, G_TYPE_OBJECT)

static void 
codeslayer_profiles_manager_class_init (CodeSlayerProfilesManagerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_profiles_manager_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerProfilesManagerPrivate));
}

static void
codeslayer_profiles_manager_init (CodeSlayerProfilesManager *profiles_manager)
{
}

static void
codeslayer_profiles_manager_finalize (CodeSlayerProfilesManager *profiles_manager)
{
  G_OBJECT_CLASS (codeslayer_profiles_manager_parent_class)->finalize (G_OBJECT (profiles_manager));
}

/**
 * codeslayer_profiles_manager_new:
 * @window: a #GtkWindow.
 *
 * Creates a new #CodeSlayerProfilesManager.
 *
 * Returns: a new #CodeSlayerProfilesManager. 
 */
CodeSlayerProfilesManager*
codeslayer_profiles_manager_new (GtkWidget                *window, 
                                 CodeSlayerProfiles       *profiles, 
                                 CodeSlayerEngine         *engine, 
                                 CodeSlayerProjectsEngine *projects_engine)
{
  CodeSlayerProfilesManagerPrivate *priv;
  CodeSlayerProfilesManager *profiles_manager;

  profiles_manager = g_object_new (codeslayer_profiles_manager_get_type (), NULL);
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  priv->window = window;
  priv->profiles = profiles;
  priv->engine = engine;
  priv->projects_engine = projects_engine;
  
  return profiles_manager;
}

/**
 * codeslayer_profiles_manager_run_dialog:
 * @profiles: a #CodeSlayerProfilesManager.
 *
 * Show the profiles dialog.
 */
void
codeslayer_profiles_manager_run_dialog (CodeSlayerProfilesManager *profiles_manager)
{
  CodeSlayerProfilesManagerPrivate *priv;
  /*CodeSlayerProfile *profile;*/
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *notebook;
  
  priv = CODESLAYER_PROFILES_MANAGER_GET_PRIVATE (profiles_manager);
  /*profile = codeslayer_profiles_get_profile (priv->profiles);*/

  dialog = gtk_dialog_new_with_buttons (_("Profiles"), 
                                        GTK_WINDOW (priv->window),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CLOSE,
                                        GTK_RESPONSE_OK,
                                        NULL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_widget_set_size_request (content_area, 350, -1);
  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 2);
  gtk_box_pack_start (GTK_BOX (content_area), notebook, TRUE, TRUE, 0);  
  
  gtk_widget_show_all (notebook);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}
