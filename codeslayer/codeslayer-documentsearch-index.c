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

#include <codeslayer/codeslayer-documentsearch-index.h>

static void codeslayer_documentsearch_index_class_init    (CodeSlayerDocumentSearchIndexClass *klass);
static void codeslayer_documentsearch_index_init          (CodeSlayerDocumentSearchIndex      *index);
static void codeslayer_documentsearch_index_finalize      (CodeSlayerDocumentSearchIndex      *index);
static void codeslayer_documentsearch_index_get_property  (GObject                            *object, 
                                                           guint                               prop_id,
                                                           GValue                             *value,
                                                           GParamSpec                         *pspec);
static void codeslayer_documentsearch_index_set_property  (GObject                            *object, 
                                                           guint                               prop_id,
                                                           const GValue                       *value,
                                                           GParamSpec                         *pspec);

#define CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_DOCUMENTSEARCH_INDEX_TYPE, CodeSlayerDocumentSearchIndexPrivate))

typedef struct _CodeSlayerDocumentSearchIndexPrivate CodeSlayerDocumentSearchIndexPrivate;

struct _CodeSlayerDocumentSearchIndexPrivate
{
  gchar *project_key;
  gchar *file_name;
  gchar *file_path;
};

enum
{
  PROP_0,
  PROP_PROJECT_KEY,
  PROP_FILE_NAME,
  PROP_FILE_PATH
};

G_DEFINE_TYPE (CodeSlayerDocumentSearchIndex, codeslayer_documentsearch_index, G_TYPE_OBJECT)
     
static void 
codeslayer_documentsearch_index_class_init (CodeSlayerDocumentSearchIndexClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_documentsearch_index_finalize;

  gobject_class->get_property = codeslayer_documentsearch_index_get_property;
  gobject_class->set_property = codeslayer_documentsearch_index_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerDocumentSearchIndexPrivate));

  g_object_class_install_property (gobject_class, 
                                   PROP_PROJECT_KEY,
                                   g_param_spec_string ("project_key", 
                                                        "Project Key",
                                                        "Project Key", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_FILE_NAME,
                                   g_param_spec_string ("file_name", 
                                                        "File Name",
                                                        "File Name", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_FILE_PATH,
                                   g_param_spec_string ("file_path",
                                                        "File Path",
                                                        "File Path",
                                                        "",
                                                        G_PARAM_READWRITE));
}

static void
codeslayer_documentsearch_index_init (CodeSlayerDocumentSearchIndex *index)
{
  CodeSlayerDocumentSearchIndexPrivate *priv;
  priv = CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index);
  priv->project_key = NULL;
  priv->file_name = NULL;
  priv->file_path = NULL;
}

static void
codeslayer_documentsearch_index_finalize (CodeSlayerDocumentSearchIndex *index)
{
  CodeSlayerDocumentSearchIndexPrivate *priv;
  priv = CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index);
  if (priv->project_key)
    {
      g_free (priv->project_key);
      priv->project_key = NULL;
    }
  if (priv->file_name)
    {
      g_free (priv->file_name);
      priv->file_name = NULL;
    }
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  G_OBJECT_CLASS (codeslayer_documentsearch_index_parent_class)->finalize (G_OBJECT (index));
}

static void
codeslayer_documentsearch_index_get_property (GObject    *object, 
                                              guint       prop_id,
                                              GValue     *value, 
                                              GParamSpec *pspec)
{
  CodeSlayerDocumentSearchIndex *index;
  CodeSlayerDocumentSearchIndexPrivate *priv;
  
  index = CODESLAYER_DOCUMENTSEARCH_INDEX (object);
  priv = CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index);

  switch (prop_id)
    {
    case PROP_PROJECT_KEY:
      g_value_set_string (value, priv->project_key);
      break;
    case PROP_FILE_NAME:
      g_value_set_string (value, priv->file_name);
      break;
    case PROP_FILE_PATH:
      g_value_set_string (value, priv->file_path);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_documentsearch_index_set_property (GObject      *object, 
                                              guint         prop_id,
                                              const GValue *value, 
                                              GParamSpec   *pspec)
{
  CodeSlayerDocumentSearchIndex *index;
  index = CODESLAYER_DOCUMENTSEARCH_INDEX (object);

  switch (prop_id)
    {
    case PROP_PROJECT_KEY:
      codeslayer_documentsearch_index_set_project_key (index, g_value_get_string (value));
      break;
    case PROP_FILE_NAME:
      codeslayer_documentsearch_index_set_file_name (index, g_value_get_string (value));
      break;
    case PROP_FILE_PATH:
      codeslayer_documentsearch_index_set_file_path (index, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

CodeSlayerDocumentSearchIndex *
codeslayer_documentsearch_index_new (void)
{
  return CODESLAYER_DOCUMENTSEARCH_INDEX (g_object_new (codeslayer_documentsearch_index_get_type (), NULL));
}

const gchar *
codeslayer_documentsearch_index_get_project_key (CodeSlayerDocumentSearchIndex *index)
{
  return CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index)->project_key;
}

void
codeslayer_documentsearch_index_set_project_key (CodeSlayerDocumentSearchIndex *index, 
                                                 const gchar                   *project_key)
{
  CodeSlayerDocumentSearchIndexPrivate *priv;
  priv = CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index);
  if (priv->project_key)
    {
      g_free (priv->project_key);
      priv->project_key = NULL;
    }
  priv->project_key = g_strdup (project_key);
}

const gchar*
codeslayer_documentsearch_index_get_file_name (CodeSlayerDocumentSearchIndex *index)
{
  return CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index)->file_name;
}

void
codeslayer_documentsearch_index_set_file_name (CodeSlayerDocumentSearchIndex *index, 
                                               const gchar                   *file_name)
{
  CodeSlayerDocumentSearchIndexPrivate *priv;
  priv = CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index);
  if (priv->file_name)
    {
      g_free (priv->file_name);
      priv->file_name = NULL;
    }
  priv->file_name = g_strdup (file_name);
}

const gchar *
codeslayer_documentsearch_index_get_file_path (CodeSlayerDocumentSearchIndex *index)
{
  return CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index)->file_path;
}

void
codeslayer_documentsearch_index_set_file_path (CodeSlayerDocumentSearchIndex *index,
                                               const gchar                   *file_path)
{
  CodeSlayerDocumentSearchIndexPrivate *priv;
  priv = CODESLAYER_DOCUMENTSEARCH_INDEX_GET_PRIVATE (index);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}
