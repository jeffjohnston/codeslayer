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
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-registry
 * @short_description: The application properties.
 * @title: CodeSlayerRegistry
 * @include: codeslayer/codeslayer-registry.h
 */

static void codeslayer_registry_class_init  (CodeSlayerRegistryClass *klass);

static void codeslayer_registry_class_init  (CodeSlayerRegistryClass *klass);
static void codeslayer_registry_init        (CodeSlayerRegistry      *registry);
static void codeslayer_registry_finalize    (CodeSlayerRegistry      *registry);

#define CODESLAYER_REGISTRY_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_REGISTRY_TYPE, CodeSlayerRegistryPrivate))

typedef struct _CodeSlayerRegistryPrivate CodeSlayerRegistryPrivate;

struct _CodeSlayerRegistryPrivate
{
  CodeSlayerConfigHandler *config_handler;
};

G_DEFINE_TYPE (CodeSlayerRegistry, codeslayer_registry, G_TYPE_OBJECT)

static void 
codeslayer_registry_class_init (CodeSlayerRegistryClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_registry_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerRegistryPrivate));
}

static void
codeslayer_registry_init (CodeSlayerRegistry *registry)
{
}

static void
codeslayer_registry_finalize (CodeSlayerRegistry *registry)
{
  G_OBJECT_CLASS (codeslayer_registry_parent_class)->finalize (G_OBJECT (registry));
}

/**
 * codeslayer_registry_new:
 *
 * Creates a new #CodeSlayerRegistry.
 *
 * Returns: a new #CodeSlayerRegistry. 
 */
CodeSlayerRegistry*
codeslayer_registry_new (CodeSlayerConfigHandler *config_handler)
{
  CodeSlayerRegistry *registry;
  CodeSlayerRegistryPrivate *priv;

  registry = g_object_new (codeslayer_registry_get_type (), NULL);
  priv = CODESLAYER_REGISTRY_GET_PRIVATE (registry);
  priv->config_handler = config_handler;

  return registry;
}

/**
 * codeslayer_registry_get_integer:
 * @registry: a #CodeSlayerRegistry.
 * @key: a property key.
 *
 * Returns: the value as an integer for the given key.
 */
gint
codeslayer_registry_get_integer (CodeSlayerRegistry *registry,
                                 gchar              *key)
{
  CodeSlayerRegistryPrivate *priv;
  CodeSlayerConfig *config;
  const gchar *value;

  priv = CODESLAYER_REGISTRY_GET_PRIVATE (registry);
  config = codeslayer_config_handler_get_config (priv->config_handler);

  value = codeslayer_config_get_setting (config, key);
  if (value != NULL)
    return atoi (value);

  return -1;
}

/**
 * codeslayer_registry_set_integer:
 * @registry: a #CodeSlayerRegistry.
 * @key: a property key.
 * @value: a property value as a gint.
 */
void
codeslayer_registry_set_integer (CodeSlayerRegistry *registry,
                                 gchar              *key, 
                                 gint                value)
{
  CodeSlayerRegistryPrivate *priv;
  CodeSlayerConfig *config;
  gchar *val;

  priv = CODESLAYER_REGISTRY_GET_PRIVATE (registry);
  config = codeslayer_config_handler_get_config (priv->config_handler);

  val = g_strdup_printf ("%d", value);
  codeslayer_config_set_setting (config, key, val);
  g_free (val);
}

/**
 * codeslayer_registry_get_boolean:
 * @registry: a #CodeSlayerRegistry.
 * @key: a property key.
 *
 * Returns: the value as a boolean for the given key.
 */
gboolean
codeslayer_registry_get_boolean (CodeSlayerRegistry *registry,
                                 gchar              *key)
{
  CodeSlayerRegistryPrivate *priv;
  CodeSlayerConfig *config;
  const gchar *value;

  priv = CODESLAYER_REGISTRY_GET_PRIVATE (registry);
  config = codeslayer_config_handler_get_config (priv->config_handler);

  value = codeslayer_config_get_setting (config, key);
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
 * codeslayer_registry_set_boolean:
 * @registry: a #CodeSlayerRegistry.
 * @key: a property key.
 * @value: a property value as a gboolean.
 */
void
codeslayer_registry_set_boolean (CodeSlayerRegistry *registry,
                                 gchar                 *key, 
                                 gboolean               value)
{
  CodeSlayerRegistryPrivate *priv;
  CodeSlayerConfig *config;
  
  priv = CODESLAYER_REGISTRY_GET_PRIVATE (registry);
  config = codeslayer_config_handler_get_config (priv->config_handler);
  
  if (value == TRUE)  
    codeslayer_config_set_setting (config, key, "true");
  else
    codeslayer_config_set_setting (config, key, "false");
}

/**
 * codeslayer_registry_get_string:
 * @registry: a #CodeSlayerRegistry.
 * @key: a property key.
 *
 * Returns: the value as a string for the given key.
 */
gchar*
codeslayer_registry_get_string (CodeSlayerRegistry *registry,
                                gchar              *key)
{
  CodeSlayerRegistryPrivate *priv;
  CodeSlayerConfig *config;
  const gchar *value;

  priv = CODESLAYER_REGISTRY_GET_PRIVATE (registry);
  config = codeslayer_config_handler_get_config (priv->config_handler);

  value = codeslayer_config_get_setting (config, key);
  if (value != NULL)
    return g_strdup (value);

  return g_strdup ("");
}

/**
 * codeslayer_registry_set_string:
 * @registry: a #CodeSlayerRegistry.
 * @key: a property key.
 * @value: a property value as a gchar pointer.
 */
void
codeslayer_registry_set_string (CodeSlayerRegistry *registry,
                                gchar              *key, 
                                gchar              *value)
{
  CodeSlayerRegistryPrivate *priv;
  CodeSlayerConfig *config;
  
  priv = CODESLAYER_REGISTRY_GET_PRIVATE (registry);
  config = codeslayer_config_handler_get_config (priv->config_handler);

  codeslayer_config_set_setting (config, key, value);
}
