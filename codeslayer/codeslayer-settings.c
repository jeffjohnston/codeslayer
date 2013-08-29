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
#include <codeslayer/codeslayer-settings.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-settings
 * @short_description: The application properties.
 * @title: CodeSlayerSettings
 * @include: codeslayer/codeslayer-settings.h
 *
 * Saves the application settings using the GKeyFile. The file is saved in the 
 * users home directory under the .codeslayer folder.
 */

static void codeslayer_settings_class_init  (CodeSlayerSettingsClass *klass);

static void codeslayer_settings_class_init  (CodeSlayerSettingsClass *klass);
static void codeslayer_settings_init        (CodeSlayerSettings      *settings);
static void codeslayer_settings_finalize    (CodeSlayerSettings      *settings);

#define CODESLAYER_SETTINGS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_SETTINGS_TYPE, CodeSlayerSettingsPrivate))

typedef struct _CodeSlayerSettingsPrivate CodeSlayerSettingsPrivate;

struct _CodeSlayerSettingsPrivate
{
  CodeSlayerConfig *config;
};

G_DEFINE_TYPE (CodeSlayerSettings, codeslayer_settings, G_TYPE_OBJECT)

static void 
codeslayer_settings_class_init (CodeSlayerSettingsClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_settings_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerSettingsPrivate));
}

static void
codeslayer_settings_init (CodeSlayerSettings *settings)
{
}

static void
codeslayer_settings_finalize (CodeSlayerSettings *settings)
{
  G_OBJECT_CLASS (codeslayer_settings_parent_class)->finalize (G_OBJECT (settings));
}

/**
 * codeslayer_settings_new:
 *
 * Creates a new #CodeSlayerSettings.
 *
 * Returns: a new #CodeSlayerSettings. 
 */
CodeSlayerSettings*
codeslayer_settings_new ()
{
  CodeSlayerSettings *settings;
  settings = g_object_new (codeslayer_settings_get_type (), NULL);
  return settings;
}

/**
 * codeslayer_settings_get_integer:
 * @settings: a #CodeSlayerSettings.
 * @key: a property key.
 *
 * Returns: the value as an integer for the given key.
 */
gint
codeslayer_settings_get_integer (CodeSlayerSettings *settings,
                                 gchar              *key)
{
  CodeSlayerSettingsPrivate *priv;
  const gchar *value;

  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);

  value = codeslayer_config_get_setting (priv->config, key);
  if (value != NULL)
    return atoi (value);

  return -1;
}

/**
 * codeslayer_settings_set_integer:
 * @settings: a #CodeSlayerSettings.
 * @key: a property key.
 * @value: a property value as a gint.
 */
void
codeslayer_settings_set_integer (CodeSlayerSettings *settings,
                                 gchar              *key, 
                                 gint                value)
{
  CodeSlayerSettingsPrivate *priv;
  gchar *val;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  val = g_strdup_printf ("%d", value);
  codeslayer_config_set_setting (priv->config, key, val);
  g_free (val);
}

/**
 * codeslayer_settings_get_boolean:
 * @settings: a #CodeSlayerSettings.
 * @key: a property key.
 *
 * Returns: the value as a boolean for the given key.
 */
gboolean
codeslayer_settings_get_boolean (CodeSlayerSettings *settings,
                                 gchar              *key)
{
  CodeSlayerSettingsPrivate *priv;
  const gchar *value;

  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);

  value = codeslayer_config_get_setting (priv->config, key);
  if (value != NULL)
    {
      if (g_strcmp0 (value, "true") == 0)
        return TRUE;
      else
        return FALSE;
    }
  
  return FALSE;
}

/**
 * codeslayer_settings_set_boolean:
 * @settings: a #CodeSlayerSettings.
 * @key: a property key.
 * @value: a property value as a gboolean.
 */
void
codeslayer_settings_set_boolean (CodeSlayerSettings *settings,
                                 gchar                 *key, 
                                 gboolean               value)
{
  CodeSlayerSettingsPrivate *priv;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  
  if (value == TRUE)  
    codeslayer_config_set_setting (priv->config, key, "true");
  else
    codeslayer_config_set_setting (priv->config, key, "false");
}

/**
 * codeslayer_settings_get_string:
 * @settings: a #CodeSlayerSettings.
 * @key: a property key.
 *
 * Returns: the value as a string for the given key.
 */
gchar*
codeslayer_settings_get_string (CodeSlayerSettings *settings,
                                gchar              *key)
{
  CodeSlayerSettingsPrivate *priv;
  const gchar *value;

  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);

  value = codeslayer_config_get_setting (priv->config, key);
  if (value != NULL)
    return g_strdup (value);

  return g_strdup ("");
}

/**
 * codeslayer_settings_set_string:
 * @settings: a #CodeSlayerSettings.
 * @key: a property key.
 * @value: a property value as a gchar pointer.
 */
void
codeslayer_settings_set_string (CodeSlayerSettings *settings,
                                gchar              *key, 
                                gchar              *value)
{
  CodeSlayerSettingsPrivate *priv;  
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  codeslayer_config_set_setting (priv->config, key, value);
}

/**
 * codeslayer_settings_load:
 * @settings: a #CodeSlayerSettings.
 * @config: a #CodeSlayerConfig.
 *
 * Load the config settings.
 */
void
codeslayer_settings_load (CodeSlayerSettings *settings, 
                          CodeSlayerConfig   *config)
{
  CodeSlayerSettingsPrivate *priv;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  priv->config = config;
}                             
