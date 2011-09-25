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

#include <codeslayer/codeslayer-project.h>

/**
 * SECTION:codeslayer-project
 * @short_description: Contains the documents.
 * @title: CodeSlayerProject
 * @include: codeslayer/codeslayer-project.h
 */

static void codeslayer_project_class_init    (CodeSlayerProjectClass *klass);
static void codeslayer_project_init          (CodeSlayerProject      *project);
static void codeslayer_project_finalize      (CodeSlayerProject      *project);
static void codeslayer_project_get_property  (GObject                *object, 
                                              guint                   prop_id,
                                              GValue                 *value,
                                              GParamSpec             *pspec);
static void codeslayer_project_set_property  (GObject                *object, 
                                              guint                   prop_id,
                                              const GValue           *value,
                                              GParamSpec             *pspec);

#define CODESLAYER_PROJECT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROJECT_TYPE, CodeSlayerProjectPrivate))

typedef struct _CodeSlayerProjectPrivate CodeSlayerProjectPrivate;

struct _CodeSlayerProjectPrivate
{
  gchar *name;
  gchar *key;
  gchar *folder_path;
};

enum
{
  PROP_0,
  PROP_NAME,
  PROP_KEY,
  PROP_FOLDER_PATH
};

G_DEFINE_TYPE (CodeSlayerProject, codeslayer_project, G_TYPE_OBJECT)
     
static void 
codeslayer_project_class_init (CodeSlayerProjectClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_project_finalize;

  gobject_class->get_property = codeslayer_project_get_property;
  gobject_class->set_property = codeslayer_project_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerProjectPrivate));

  /**
	 * CodeSlayerProject:name:
	 *
	 * The displayed name for the project.
	 */
  g_object_class_install_property (gobject_class, 
                                   PROP_NAME,
                                   g_param_spec_string ("name", 
                                                        "Name",
                                                        "Name Object", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerProject:key:
	 *
	 * The key for the project to uniquely identify it.
	 */
  g_object_class_install_property (gobject_class, 
                                   PROP_KEY,
                                   g_param_spec_string ("key", 
                                                        "Key",
                                                        "Key Object", "",
                                                        G_PARAM_READWRITE));

  /**
	 * CodeSlayerProject:folder_path:
	 *
	 * The fully qualified folder path where the project is located.
	 */
  g_object_class_install_property (gobject_class, 
                                   PROP_FOLDER_PATH,
                                   g_param_spec_string ("folder_path",
                                                        "Folder Path",
                                                        "Folder Path Object",
                                                        "",
                                                        G_PARAM_READWRITE));
}

static void
codeslayer_project_init (CodeSlayerProject *project)
{
  CodeSlayerProjectPrivate *priv;
  priv = CODESLAYER_PROJECT_GET_PRIVATE (project);
  priv->name = NULL;
  priv->key = NULL;
}

static void
codeslayer_project_finalize (CodeSlayerProject *project)
{
  CodeSlayerProjectPrivate *priv;
  priv = CODESLAYER_PROJECT_GET_PRIVATE (project);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  if (priv->key)
    {
      g_free (priv->key);
      priv->key = NULL;
    }
  if (priv->folder_path)
    {
      g_free (priv->folder_path);
      priv->folder_path = NULL;
    }
  G_OBJECT_CLASS (codeslayer_project_parent_class)->finalize (G_OBJECT (project));
}

static void
codeslayer_project_get_property (GObject    *object, 
                                 guint       prop_id,
                                 GValue     *value, 
                                 GParamSpec *pspec)
{
  CodeSlayerProject *project;
  CodeSlayerProjectPrivate *priv;
  
  project = CODESLAYER_PROJECT (object);
  priv = CODESLAYER_PROJECT_GET_PRIVATE (project);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_KEY:
      g_value_set_string (value, priv->key);
      break;
    case PROP_FOLDER_PATH:
      g_value_set_string (value, priv->folder_path);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_project_set_property (GObject      *object, 
                                 guint         prop_id,
                                 const GValue *value, 
                                 GParamSpec   *pspec)
{
  CodeSlayerProject *project;
  project = CODESLAYER_PROJECT (object);

  switch (prop_id)
    {
    case PROP_NAME:
      codeslayer_project_set_name (project, g_value_get_string (value));
      break;
    case PROP_KEY:
      codeslayer_project_set_key (project, g_value_get_string (value));
      break;
    case PROP_FOLDER_PATH:
      codeslayer_project_set_folder_path (project, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_project_new:
 *
 * Creates a new #CodeSlayerProject.
 *
 * Returns: a new #CodeSlayerProject. 
 */
CodeSlayerProject *
codeslayer_project_new (void)
{
  return CODESLAYER_PROJECT (g_object_new (codeslayer_project_get_type (), NULL));
}

/**
 * codeslayer_project_get_name:
 * @project: a #CodeSlayerProject.
 *
 * Returns: the text to display for the project.
 */
const gchar *
codeslayer_project_get_name (CodeSlayerProject *project)
{
  return CODESLAYER_PROJECT_GET_PRIVATE (project)->name;
}

/**
 * codeslayer_project_set_name:
 * @project: a #CodeSlayerProject.
 * @name: the text to display for the project.
 */
void
codeslayer_project_set_name (CodeSlayerProject *project, 
                             const gchar       *name)
{
  CodeSlayerProjectPrivate *priv;
  priv = CODESLAYER_PROJECT_GET_PRIVATE (project);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

/**
 * codeslayer_project_get_key:
 * @project: a #CodeSlayerProject.
 *
 * Returns: the unique identifier for the project
 */
const gchar *
codeslayer_project_get_key (CodeSlayerProject *project)
{
  return CODESLAYER_PROJECT_GET_PRIVATE (project)->key;
}

/**
 * codeslayer_project_set_key:
 * @project: a #CodeSlayerProject.
 * @key: the unique identifier for the project
 */
void
codeslayer_project_set_key (CodeSlayerProject *project, 
                             const gchar       *key)
{
  CodeSlayerProjectPrivate *priv;
  priv = CODESLAYER_PROJECT_GET_PRIVATE (project);
  if (priv->key)
    {
      g_free (priv->key);
      priv->key = NULL;
    }
  priv->key = g_strdup (key);
}

/**
 * codeslayer_project_get_folder_path:
 * @project: a #CodeSlayerProject.
 *
 * Returns: the fully qualified path to the project.
 */
const gchar *
codeslayer_project_get_folder_path (CodeSlayerProject *project)
{
  return CODESLAYER_PROJECT_GET_PRIVATE (project)->folder_path;
}

/**
 * codeslayer_project_set_folder_path:
 * @project: a #CodeSlayerDocument.
 * @folder_path: the fully qualified path to the project.
 *
 * Note: this is not a URI, but rather a normal file path.
 */
void
codeslayer_project_set_folder_path (CodeSlayerProject *project,
                                    const gchar       *folder_path)
{
  CodeSlayerProjectPrivate *priv;
  priv = CODESLAYER_PROJECT_GET_PRIVATE (project);
  if (priv->folder_path)
    {
      g_free (priv->folder_path);
      priv->folder_path = NULL;
    }
  priv->folder_path = g_strdup (folder_path);
}
