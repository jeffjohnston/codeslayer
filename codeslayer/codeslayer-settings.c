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

static gboolean verify_conf_exists (void); 

static void codeslayer_settings_class_init  (CodeSlayerSettingsClass *klass);
static void codeslayer_settings_init        (CodeSlayerSettings      *settings);
static void codeslayer_settings_finalize    (CodeSlayerSettings      *settings);

static void set_defaults                    (CodeSlayerSettings      *settings);
static gchar *get_conf_path                 (void);

#define CODESLAYER_SETTINGS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_SETTINGS_TYPE, CodeSlayerSettingsPrivate))

#define MAIN "main"
#define SETTINGS_CONF "settings.conf"

typedef struct _CodeSlayerSettingsPrivate CodeSlayerSettingsPrivate;

struct _CodeSlayerSettingsPrivate
{
  GKeyFile *keyfile;
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
  CodeSlayerSettingsPrivate *priv;
  gboolean conf_exists;
  GKeyFile *keyfile;
  gchar *conf;
  
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  conf_exists = verify_conf_exists ();
  keyfile = g_key_file_new ();

  conf = get_conf_path ();
  g_key_file_load_from_file (keyfile, conf, G_KEY_FILE_NONE, NULL);
  priv->keyfile = keyfile;
  g_free (conf);

  if (!conf_exists)
    set_defaults (settings);
}

static void
codeslayer_settings_finalize (CodeSlayerSettings *settings)
{
  CodeSlayerSettingsPrivate *priv;
  
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);

  if (priv->keyfile)
    {
      g_key_file_free (priv->keyfile);
      priv->keyfile = NULL;
    }
    
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
 * @key: a property name.
 *
 * Returns: the value as an integer for the given key.
 */
gint
codeslayer_settings_get_integer (CodeSlayerSettings *settings,
                                 gchar              *key)
{
  CodeSlayerSettingsPrivate *priv;
  
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  if (g_key_file_has_key (priv->keyfile, MAIN, key, NULL))
    return g_key_file_get_integer (priv->keyfile, MAIN, key, NULL);
  
  return -1;
}

/**
 * codeslayer_settings_set_integer:
 * @settings: a #CodeSlayerSettings.
 * @key: a property name.
 * @value: a property value as a gint.
 */
void
codeslayer_settings_set_integer (CodeSlayerSettings *settings,
                                 gchar              *key, 
                                 gint                value)
{
  CodeSlayerSettingsPrivate *priv;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  g_key_file_set_integer (priv->keyfile, MAIN, key, value);
}

/**
 * codeslayer_settings_get_double:
 * @settings: a #CodeSlayerSettings.
 * @key: a property name.
 *
 * Returns: the value as a double for the given key.
 */
gdouble
codeslayer_settings_get_double (CodeSlayerSettings *settings,
                                   gchar                 *key)
{
  CodeSlayerSettingsPrivate *priv;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);

  if (g_key_file_has_key (priv->keyfile, MAIN, key, NULL))
    return g_key_file_get_double (priv->keyfile, MAIN, key, NULL);

  return -1;
}

/**
 * codeslayer_settings_set_double:
 * @settings: a #CodeSlayerSettings.
 * @key: a property name.
 * @value: a property value as a gdouble.
 */
void
codeslayer_settings_set_double (CodeSlayerSettings *settings,
                                gchar              *key, 
                                gdouble             value)
{
  CodeSlayerSettingsPrivate *priv;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  g_key_file_set_double (priv->keyfile, MAIN, key, value);
}

/**
 * codeslayer_settings_get_boolean:
 * @settings: a #CodeSlayerSettings.
 * @key: a property name.
 *
 * Returns: the value as a boolean for the given key.
 */
gboolean
codeslayer_settings_get_boolean (CodeSlayerSettings *settings,
                                 gchar              *key)
{
  CodeSlayerSettingsPrivate *priv;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);

  if (g_key_file_has_key (priv->keyfile, MAIN, key, NULL))
    return g_key_file_get_boolean (priv->keyfile, MAIN, key, NULL);
  
  return -1;
}

/**
 * codeslayer_settings_set_boolean:
 * @settings: a #CodeSlayerSettings.
 * @key: a property name.
 * @value: a property value as a gboolean.
 */
void
codeslayer_settings_set_boolean (CodeSlayerSettings *settings,
                                 gchar                 *key, 
                                 gboolean               value)
{
  CodeSlayerSettingsPrivate *priv;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  g_key_file_set_boolean (priv->keyfile, MAIN, key, value);
}

/**
 * codeslayer_settings_get_string:
 * @settings: a #CodeSlayerSettings.
 * @key: a property name.
 *
 * Returns: the value as a string for the given key.
 */
gchar *
codeslayer_settings_get_string (CodeSlayerSettings *settings,
                                   gchar                 *key)
{
  CodeSlayerSettingsPrivate *priv;
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  
  if (g_key_file_has_key (priv->keyfile, MAIN, key, NULL))
    return g_key_file_get_string (priv->keyfile, MAIN, key, NULL);

  return g_strdup ("");
}

/**
 * codeslayer_settings_set_string:
 * @settings: a #CodeSlayerSettings.
 * @key: a property name.
 * @value: a property value as a gchar pointer.
 */
void
codeslayer_settings_set_string (CodeSlayerSettings *settings,
                                   gchar                 *key, 
                                   const gchar           *value)
{
  CodeSlayerSettingsPrivate *priv;  
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);
  g_key_file_set_string (priv->keyfile, MAIN, key, value);
}

/**
 * codeslayer_settings_save:
 * @settings: a #CodeSlayerSettings.
 *
 * Save the users preference to disk.
 */
void
codeslayer_settings_save (CodeSlayerSettings *settings)
{
  CodeSlayerSettingsPrivate *priv;
  gchar *data;
  gchar *conf_path;
  gsize size;
  
  priv = CODESLAYER_SETTINGS_GET_PRIVATE (settings);

  data = g_key_file_to_data (priv->keyfile, &size, NULL);

  conf_path = get_conf_path ();

  g_file_set_contents (conf_path, data, size, NULL);

  g_free (conf_path);
  g_free (data);
}

static void
set_defaults (CodeSlayerSettings *settings)
{
  codeslayer_settings_set_boolean (settings,
                                   CODESLAYER_SETTINGS_SIDE_PANE_VISIBLE,
                                   TRUE);
  codeslayer_settings_set_boolean (settings,
                                   CODESLAYER_SETTINGS_BOTTOM_PANE_VISIBLE,
                                   FALSE);
  codeslayer_settings_set_boolean (settings,
                                   CODESLAYER_SETTINGS_DRAW_SPACES,
                                   FALSE);
  codeslayer_settings_set_boolean (settings,
                                   CODESLAYER_SETTINGS_SYNC_WITH_EDITOR,
                                   TRUE);
  codeslayer_settings_save (settings);
}

static gboolean
verify_conf_exists ()
{
  gboolean result = TRUE;
  gchar *conf_path;
  GFile *conf_file;

  conf_path = get_conf_path ();
  conf_file = g_file_new_for_path (conf_path);
  if (!g_file_query_exists (conf_file, NULL))
    {
      GFileIOStream *stream;
      stream = g_file_create_readwrite (conf_file, G_FILE_CREATE_NONE, 
                                        NULL, NULL);
      g_io_stream_close (G_IO_STREAM (stream), NULL, NULL);
      g_object_unref (stream);
      result = FALSE;
    }

  g_free (conf_path);
  g_object_unref (conf_file);

  return result;
}

static gchar*
get_conf_path ()
{
  return g_build_filename (g_get_home_dir (), CODESLAYER_HOME, SETTINGS_CONF, NULL);
}
