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

#include <codeslayer/codeslayer-preferences-misc.h>
#include <codeslayer/codeslayer-preferences-listview.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-registry.h>

/**
 * SECTION:codeslayer-preferences-misc
 * @short_description: The preferences misc tab.
 * @title: CodeSlayerPreferencesMisc
 * @include: codeslayer/codeslayer-preferences-misc.h
 */

static void codeslayer_preferences_misc_class_init  (CodeSlayerPreferencesMiscClass *klass);
static void codeslayer_preferences_misc_init        (CodeSlayerPreferencesMisc      *preferences_search);
static void codeslayer_preferences_misc_finalize    (CodeSlayerPreferencesMisc      *preferences_search);

#define CODESLAYER_PREFERENCES_MISC_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PREFERENCES_MISC_TYPE, CodeSlayerPreferencesMiscPrivate))

typedef struct _CodeSlayerPreferencesMiscPrivate CodeSlayerPreferencesMiscPrivate;

struct _CodeSlayerPreferencesMiscPrivate
{
  GObject *word_wrap_types;
};

G_DEFINE_TYPE (CodeSlayerPreferencesMisc, codeslayer_preferences_misc, GTK_TYPE_VBOX)

static void
codeslayer_preferences_misc_class_init (CodeSlayerPreferencesMiscClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_preferences_misc_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPreferencesMiscPrivate));
}

static void
codeslayer_preferences_misc_init (CodeSlayerPreferencesMisc *preferences_search)
{
  gtk_box_set_homogeneous (GTK_BOX (preferences_search), FALSE);
  gtk_box_set_spacing (GTK_BOX (preferences_search), 15);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_search), 5);
}

static void
codeslayer_preferences_misc_finalize (CodeSlayerPreferencesMisc *preferences_misc)
{
  CodeSlayerPreferencesMiscPrivate *priv;
  priv = CODESLAYER_PREFERENCES_MISC_GET_PRIVATE (preferences_misc);
  g_object_unref (priv->word_wrap_types);
  G_OBJECT_CLASS (codeslayer_preferences_misc_parent_class)->finalize (G_OBJECT (preferences_misc));
}

/**
 * codeslayer_preferences_misc_new:
 * @preferences: a #CodeSlayerPreferences.
 *
 * Creates a new #CodeSlayerPreferencesMisc.
 *
 * Returns: a new #CodeSlayerPreferencesMisc. 
 */
GtkWidget*
codeslayer_preferences_misc_new (CodeSlayerPreferences *preferences, 
                                 CodeSlayerProfiles    *profiles)
{
  CodeSlayerPreferencesMiscPrivate *priv;
  GtkWidget *preferences_misc;

  preferences_misc = g_object_new (codeslayer_preferences_misc_get_type (), NULL);
  priv = CODESLAYER_PREFERENCES_MISC_GET_PRIVATE (preferences_misc);

  priv->word_wrap_types = codeslayer_preferences_list_view_new (preferences, profiles,
                                       CODESLAYER_REGISTRY_EDITOR_WORD_WRAP_TYPES,
                                       preferences_misc, _("Word Wrap File Types"));
  
  return preferences_misc;
}
