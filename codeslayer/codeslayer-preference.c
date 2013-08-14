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

#include <codeslayer/codeslayer-preference.h>

/**
 * SECTION:codeslayer-preference
 * @short_description: Contains the documents.
 * @title: CodeSlayerPreference
 * @include: codeslayer/codeslayer-preference.h
 */

static void codeslayer_preference_class_init    (CodeSlayerPreferenceClass *klass);
static void codeslayer_preference_init          (CodeSlayerPreference      *preference);
static void codeslayer_preference_finalize      (CodeSlayerPreference      *preference);
static void codeslayer_preference_get_property  (GObject                   *object, 
                                                 guint                      prop_id,
                                                 GValue                    *value,
                                                 GParamSpec                *pspec);
static void codeslayer_preference_set_property  (GObject                   *object, 
                                                 guint                      prop_id,
                                                 const GValue              *value,
                                                 GParamSpec                *pspec);

#define CODESLAYER_PREFERENCE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PREFERENCE_TYPE, CodeSlayerPreferencePrivate))

typedef struct _CodeSlayerPreferencePrivate CodeSlayerPreferencePrivate;

struct _CodeSlayerPreferencePrivate
{
  gchar *name;
  gchar *value;
};

enum
{
  PROP_0,
  PROP_NAME,
  PROP_VALUE
};

G_DEFINE_TYPE (CodeSlayerPreference, codeslayer_preference, G_TYPE_OBJECT)
     
static void 
codeslayer_preference_class_init (CodeSlayerPreferenceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_preference_finalize;

  gobject_class->get_property = codeslayer_preference_get_property;
  gobject_class->set_property = codeslayer_preference_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerPreferencePrivate));

  /**
   * CodeSlayerPreference:name:
   *
   * The name for the preference.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_NAME,
                                   g_param_spec_string ("name", 
                                                        "Name",
                                                        "Name Object", "",
                                                        G_PARAM_READWRITE));

  /**
   * CodeSlayerPreference:value:
   *
   * The value for the preference.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_VALUE,
                                   g_param_spec_string ("value",
                                                        "Value",
                                                        "Value",
                                                        "",
                                                        G_PARAM_READWRITE));
}

static void
codeslayer_preference_init (CodeSlayerPreference *preference)
{
  CodeSlayerPreferencePrivate *priv;
  priv = CODESLAYER_PREFERENCE_GET_PRIVATE (preference);
  priv->name = NULL;
  priv->value = NULL;
}

static void
codeslayer_preference_finalize (CodeSlayerPreference *preference)
{
  CodeSlayerPreferencePrivate *priv;
  priv = CODESLAYER_PREFERENCE_GET_PRIVATE (preference);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  if (priv->value)
    {
      g_free (priv->value);
      priv->value = NULL;
    }
  G_OBJECT_CLASS (codeslayer_preference_parent_class)->finalize (G_OBJECT (preference));
}

static void
codeslayer_preference_get_property (GObject    *object, 
                                 guint       prop_id,
                                 GValue     *value, 
                                 GParamSpec *pspec)
{
  CodeSlayerPreference *preference;
  CodeSlayerPreferencePrivate *priv;
  
  preference = CODESLAYER_PREFERENCE (object);
  priv = CODESLAYER_PREFERENCE_GET_PRIVATE (preference);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_VALUE:
      g_value_set_string (value, priv->value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_preference_set_property (GObject      *object, 
                                 guint         prop_id,
                                 const GValue *value, 
                                 GParamSpec   *pspec)
{
  CodeSlayerPreference *preference;
  preference = CODESLAYER_PREFERENCE (object);

  switch (prop_id)
    {
    case PROP_NAME:
      codeslayer_preference_set_name (preference, g_value_get_string (value));
      break;
    case PROP_VALUE:
      codeslayer_preference_set_value (preference, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_preference_new:
 *
 * Creates a new #CodeSlayerPreference.
 *
 * Returns: a new #CodeSlayerPreference. 
 */
CodeSlayerPreference *
codeslayer_preference_new (void)
{
  return CODESLAYER_PREFERENCE (g_object_new (codeslayer_preference_get_type (), NULL));
}

/**
 * codeslayer_preference_get_name:
 * @preference: a #CodeSlayerPreference.
 *
 * Returns: the text to display for the preference.
 */
const gchar *
codeslayer_preference_get_name (CodeSlayerPreference *preference)
{
  return CODESLAYER_PREFERENCE_GET_PRIVATE (preference)->name;
}

/**
 * codeslayer_preference_set_name:
 * @preference: a #CodeSlayerPreference.
 * @name: the text to display for the preference.
 */
void
codeslayer_preference_set_name (CodeSlayerPreference *preference, 
                             const gchar       *name)
{
  CodeSlayerPreferencePrivate *priv;
  priv = CODESLAYER_PREFERENCE_GET_PRIVATE (preference);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

/**
 * codeslayer_preference_get_value:
 * @preference: a #CodeSlayerPreference.
 *
 * Returns: the fully qualified path to the preference.
 */
const gchar *
codeslayer_preference_get_value (CodeSlayerPreference *preference)
{
  return CODESLAYER_PREFERENCE_GET_PRIVATE (preference)->value;
}

/**
 * codeslayer_preference_set_value:
 * @preference: a #CodeSlayerDocument.
 * @value: the fully qualified path to the preference.
 *
 * Note: this is not a URI, but rather a normal file path.
 */
void
codeslayer_preference_set_value (CodeSlayerPreference *preference,
                                    const gchar       *value)
{
  CodeSlayerPreferencePrivate *priv;
  priv = CODESLAYER_PREFERENCE_GET_PRIVATE (preference);
  if (priv->value)
    {
      g_free (priv->value);
      priv->value = NULL;
    }
  priv->value = g_strdup (value);
}
