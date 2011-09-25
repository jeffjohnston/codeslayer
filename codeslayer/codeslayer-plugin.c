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

#include <gmodule.h>
#include <glib.h>
#include <codeslayer/codeslayer-plugin.h>

/**
 * SECTION:codeslayer-plugin
 * @short_description: Represents the plugin.
 * @title: CodeSlayerPlugin
 * @include: codeslayer/codeslayer-plugin.h
 */

static void codeslayer_plugin_class_init    (CodeSlayerPluginClass *klass);
static void codeslayer_plugin_init          (CodeSlayerPlugin      *plugin);
static void codeslayer_plugin_finalize      (CodeSlayerPlugin      *plugin);
static void codeslayer_plugin_get_property  (GObject               *object, 
                                             guint                  prop_id,
                                             GValue                *value,
                                             GParamSpec            *pspec);
static void codeslayer_plugin_set_property  (GObject               *object,
                                             guint                  prop_id,
                                             const GValue          *value,
                                             GParamSpec            *pspec);

#define CODESLAYER_PLUGIN_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PLUGIN_TYPE, CodeSlayerPluginPrivate))

typedef struct _CodeSlayerPluginPrivate CodeSlayerPluginPrivate;

typedef gboolean (*CodeSlayerActivateFunc) (gpointer data);
typedef gboolean (*CodeSlayerDeactivateFunc) (gpointer data);
typedef gboolean (*CodeSlayerConfigureFunc) (gpointer data);

struct _CodeSlayerPluginPrivate
{
  GModule  *module;
  GObject  *data;
  gchar    *lib;
  gchar    *version;
  gchar    *name;
  gchar    *description;
  gchar    *authors;
  gchar    *copyright;
  gchar    *website;
  gboolean  enabled;
  gboolean  configurable;
};

G_DEFINE_TYPE (CodeSlayerPlugin, codeslayer_plugin, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_LIB,
  PROP_VERSION,
  PROP_NAME,
  PROP_DESCRIPTION,
  PROP_AUTHORS,
  PROP_COPYRIGHT,
  PROP_WEBSITE,
  PROP_ENABLED
};

static void 
codeslayer_plugin_class_init (CodeSlayerPluginClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_plugin_finalize;

  gobject_class->get_property = codeslayer_plugin_get_property;
  gobject_class->set_property = codeslayer_plugin_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerPluginPrivate));

  /**
	 * CodeSlayerGroup:lib:
	 *
	 * The lib that will be used to locate the plugin.
	 */
  g_object_class_install_property (gobject_class, PROP_LIB,
                                   g_param_spec_string ("lib", 
                                                        "Lib", 
                                                        "Lib", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerGroup:version:
	 *
	 * The version that will be used to identify the plugin.
	 */
  g_object_class_install_property (gobject_class, PROP_VERSION,
                                   g_param_spec_string ("version", 
                                                        "Version", 
                                                        "Version", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerGroup:name:
	 *
	 * The name that will be displayed for the plugin.
	 */
  g_object_class_install_property (gobject_class, PROP_NAME,
                                   g_param_spec_string ("name", 
                                                        "Name", 
                                                        "Name", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerGroup:description:
	 *
	 * The description that will be displayed for the plugin.
	 */
  g_object_class_install_property (gobject_class, PROP_DESCRIPTION,
                                   g_param_spec_string ("description", 
                                                        "Description", 
                                                        "Description", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerGroup:description:
	 *
	 * The authors that will be displayed for the plugin.
	 */
  g_object_class_install_property (gobject_class, PROP_AUTHORS,
                                   g_param_spec_string ("authors", 
                                                        "Authors", 
                                                        "Authors", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerGroup:copyright:
	 *
	 * The copyright that will be displayed for the plugin.
	 */
  g_object_class_install_property (gobject_class, PROP_COPYRIGHT,
                                   g_param_spec_string ("copyright", 
                                                        "Copyright", 
                                                        "Copyright", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerGroup:website:
	 *
	 * The website that will be displayed for the plugin.
	 */
  g_object_class_install_property (gobject_class, PROP_WEBSITE,
                                   g_param_spec_string ("website", 
                                                        "Website", 
                                                        "Website", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerPlugin:enabled:
	 *
	 * Whether or not the plugin is enabled.
	 */
  g_object_class_install_property (gobject_class, 
                                   PROP_ENABLED,
                                   g_param_spec_boolean ("enabled", 
                                                         "Enabled",
                                                         "Enabled", 
                                                         TRUE,
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_plugin_init (CodeSlayerPlugin *plugin)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  priv->module = NULL;
  priv->lib = NULL;
  priv->version = NULL;
  priv->name = NULL;
  priv->description = NULL;
  priv->authors = NULL;
  priv->copyright = NULL;
  priv->website = NULL;
  priv->enabled = FALSE;
}

static void
codeslayer_plugin_finalize (CodeSlayerPlugin *plugin)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  if (priv->lib)
    {
      g_free (priv->lib);
      priv->lib = NULL;
    }
  if (priv->version)
    {
      g_free (priv->version);
      priv->version = NULL;
    }
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  if (priv->description)
    {
      g_free (priv->description);
      priv->description = NULL;
    }
  if (priv->authors)
    {
      g_free (priv->authors);
      priv->authors = NULL;
    }
  if (priv->copyright)
    {
      g_free (priv->copyright);
      priv->copyright = NULL;
    }
  if (priv->website)
    {
      g_free (priv->website);
      priv->website = NULL;
    }

  G_OBJECT_CLASS (codeslayer_plugin_parent_class)->finalize (G_OBJECT (plugin));
}

static void
codeslayer_plugin_get_property (GObject    *object, 
                                guint       prop_id,
                                GValue     *value, 
                                GParamSpec *pspec)
{
  CodeSlayerPluginPrivate *priv;
  CodeSlayerPlugin *plugin;
  
  plugin = CODESLAYER_PLUGIN (object);
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);

  switch (prop_id)
    {
    case PROP_LIB:
      g_value_set_string (value, priv->lib);
      break;
    case PROP_VERSION:
      g_value_set_string (value, priv->version);
      break;
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_DESCRIPTION:
      g_value_set_string (value, priv->description);
      break;
    case PROP_AUTHORS:
      g_value_set_string (value, priv->authors);
      break;
    case PROP_COPYRIGHT:
      g_value_set_string (value, priv->copyright);
      break;
    case PROP_WEBSITE:
      g_value_set_string (value, priv->website);
      break;
    case PROP_ENABLED:
      g_value_set_boolean (value, priv->enabled);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_plugin_set_property (GObject      *object, 
                                guint         prop_id,
                                const GValue *value, 
                                GParamSpec   *pspec)
{
  CodeSlayerPlugin *plugin;
  plugin = CODESLAYER_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_LIB:
      codeslayer_plugin_set_lib (plugin, g_value_get_string (value));
      break;
    case PROP_VERSION:
      codeslayer_plugin_set_version (plugin, g_value_get_string (value));
      break;
    case PROP_NAME:
      codeslayer_plugin_set_name (plugin, g_value_get_string (value));
      break;
    case PROP_DESCRIPTION:
      codeslayer_plugin_set_description (plugin, g_value_get_string (value));
      break;
    case PROP_AUTHORS:
      codeslayer_plugin_set_authors (plugin, g_value_get_string (value));
      break;
    case PROP_COPYRIGHT:
      codeslayer_plugin_set_copyright (plugin, g_value_get_string (value));
      break;
    case PROP_WEBSITE:
      codeslayer_plugin_set_website (plugin, g_value_get_string (value));
      break;
    case PROP_ENABLED:
      codeslayer_plugin_set_enabled (plugin, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_plugin_new:
 *
 * Creates a new #CodeSlayerPlugin.
 *
 * Returns: a new #CodeSlayerPlugin. 
 */
CodeSlayerPlugin*
codeslayer_plugin_new ()
{
  CodeSlayerPlugin *plugin;
  plugin = CODESLAYER_PLUGIN (g_object_new (codeslayer_plugin_get_type (), NULL));
  return plugin;
}

/**
 * codeslayer_plugin_set_data:
 * @plugin: a #CodeSlayerPlugin.
 * @data: a #CodeSlayer object.
 */
void               
codeslayer_plugin_set_data (CodeSlayerPlugin *plugin, 
                            GObject          *data)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  priv->data = data;  
}

/**
 * codeslayer_plugin_activate:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Open the library and resolve the symbols.
 */
void
codeslayer_plugin_activate (CodeSlayerPlugin *plugin)
{
  CodeSlayerPluginPrivate *priv;
  CodeSlayerActivateFunc activate;
  
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  
  if (!priv->enabled)
    return;

  if (priv->module == NULL)
    {
      g_assert (g_module_supported ());
      priv->module = g_module_open (priv->lib, G_MODULE_BIND_LAZY|G_MODULE_BIND_LOCAL);
      g_module_make_resident (priv->module);
    }

  if (!priv->module) 
    {
      g_warning ("%s\n", (gchar*) g_module_error ());
      return;  
    }

  if (!g_module_symbol (priv->module, "activate", (gpointer*) &activate))
    {
      g_warning ("%s\n", (gchar*) g_module_error ());
      return;  
    }

  activate ((gpointer) priv->data);
  
}

/**
 * codeslayer_plugin_deactivate:
 * @plugin: a #CodeSlayerPlugin.
 */
void
codeslayer_plugin_deactivate (CodeSlayerPlugin *plugin)
{
  CodeSlayerPluginPrivate *priv;
  CodeSlayerDeactivateFunc deactivate;

  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);

  if (!priv->module)
    return;

  if (!g_module_symbol (priv->module, "deactivate", (gpointer*) &deactivate))
    {
      g_warning ("%s\n", (gchar*) g_module_error ());
      return;    
    }

  deactivate ((gpointer) priv->data);
}

/**
 * codeslayer_plugin_configure:
 * @plugin: a #CodeSlayerPlugin.
 */
void
codeslayer_plugin_configure (CodeSlayerPlugin *plugin)
{
  CodeSlayerPluginPrivate *priv;
  CodeSlayerConfigureFunc configure;
  
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);

  if (!priv->module)
    return;
  
  if (!g_module_symbol (priv->module, "configure", (gpointer*) &configure))
    {
      g_warning ("%s\n", (gchar*) g_module_error ());
      return;    
    }
  
  configure ((gpointer) priv->data);
}

/**
 * codeslayer_plugin_get_lib:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: the shared library that will be loaded.
 */
const gchar*
codeslayer_plugin_get_lib (CodeSlayerPlugin *plugin)
{
  return CODESLAYER_PLUGIN_GET_PRIVATE (plugin)->lib;
}

/**
 * codeslayer_plugin_set_lib:
 * @plugin: a #CodeSlayerPlugin.
 * @lib: the lib to set.
 */
void
codeslayer_plugin_set_lib (CodeSlayerPlugin *plugin,
                           const gchar      *lib)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  if (priv->lib)
    {
      g_free (priv->lib);
      priv->lib = NULL;
    }
  priv->lib = g_strdup (lib);
}

/**
 * codeslayer_plugin_get_version:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: the version of the shared library.
 */
const gchar*
codeslayer_plugin_get_version (CodeSlayerPlugin *plugin)
{
  return CODESLAYER_PLUGIN_GET_PRIVATE (plugin)->version;
}

/**
 * codeslayer_plugin_set_version:
 * @plugin: a #CodeSlayerPlugin.
 * @version: the version to set.
 */
void
codeslayer_plugin_set_version (CodeSlayerPlugin *plugin,
                               const gchar      *version)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  if (priv->version)
    {
      g_free (priv->version);
      priv->version = NULL;
    }
  priv->version = g_strdup (version);
}

/**
 * codeslayer_plugin_get_name:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: the human readable name of the plugin.
 */
const gchar*
codeslayer_plugin_get_name (CodeSlayerPlugin *plugin)
{
  return CODESLAYER_PLUGIN_GET_PRIVATE (plugin)->name;
}

void
codeslayer_plugin_set_name (CodeSlayerPlugin *plugin,
                            const gchar      *name)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

/**
 * codeslayer_plugin_get_description:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: the plugin description.
 */
const gchar*
codeslayer_plugin_get_description (CodeSlayerPlugin *plugin)
{
  return CODESLAYER_PLUGIN_GET_PRIVATE (plugin)->description;
}

void
codeslayer_plugin_set_description (CodeSlayerPlugin *plugin,
                                   const gchar      *description)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  if (priv->description)
    {
      g_free (priv->description);
      priv->description = NULL;
    }
  priv->description = g_strdup (description);
}

/**
 * codeslayer_plugin_get_authors:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: the plugin authors.
 */
const gchar*
codeslayer_plugin_get_authors (CodeSlayerPlugin *plugin)
{
  return CODESLAYER_PLUGIN_GET_PRIVATE (plugin)->authors;
}

void
codeslayer_plugin_set_authors (CodeSlayerPlugin *plugin,
                               const gchar      *authors)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  if (priv->authors)
    {
      g_free (priv->authors);
      priv->authors = NULL;
    }
  priv->authors = g_strdup (authors);
}

/**
 * codeslayer_plugin_get_copyright:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: the plugin copyright.
 */
const gchar*
codeslayer_plugin_get_copyright (CodeSlayerPlugin *plugin)
{
  return CODESLAYER_PLUGIN_GET_PRIVATE (plugin)->copyright;
}

void
codeslayer_plugin_set_copyright (CodeSlayerPlugin *plugin,
                                 const gchar      *copyright)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  if (priv->copyright)
    {
      g_free (priv->copyright);
      priv->copyright = NULL;
    }
  priv->copyright = g_strdup (copyright);
}

/**
 * codeslayer_plugin_get_website:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: the plugin website.
 */
const gchar*
codeslayer_plugin_get_website (CodeSlayerPlugin *plugin)
{
  return CODESLAYER_PLUGIN_GET_PRIVATE (plugin)->website;
}

void
codeslayer_plugin_set_website (CodeSlayerPlugin *plugin,
                               const gchar      *website)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  if (priv->website)
    {
      g_free (priv->website);
      priv->website = NULL;
    }
  priv->website = g_strdup (website);
}

/**
 * codeslayer_plugin_get_enabled:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: is TRUE if the plugin is active.
 */
gboolean
codeslayer_plugin_get_enabled (CodeSlayerPlugin *plugin)
{
  return CODESLAYER_PLUGIN_GET_PRIVATE (plugin)->enabled;
}

/**
 * codeslayer_plugin_set_enabled:
 * @plugin: a #CodeSlayerPlugin.
 * @enabled: is TRUE if the plugin is active.
 */
void
codeslayer_plugin_set_enabled (CodeSlayerPlugin *plugin,
                               gboolean          enabled)
{
  CodeSlayerPluginPrivate *priv;
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);
  priv->enabled = enabled;
}

/**
 * codeslayer_plugin_is_configurable:
 * @plugin: a #CodeSlayerPlugin.
 *
 * Returns: is TRUE if the plugin can be configured.
 */
gboolean
codeslayer_plugin_is_configurable (CodeSlayerPlugin *plugin)
{
  CodeSlayerPluginPrivate *priv;
  CodeSlayerConfigureFunc configure;
  
  priv = CODESLAYER_PLUGIN_GET_PRIVATE (plugin);

  if (!priv->enabled)
    return FALSE;

  return g_module_symbol (priv->module, "configure", (gpointer*) &configure);
}
