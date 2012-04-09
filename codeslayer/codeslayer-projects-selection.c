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

#include <codeslayer/codeslayer-projects-selection.h>
#include <codeslayer/codeslayer-project.h>

/**
 * SECTION:codeslayer-projects-selection
 * @short_description: Represents the nodes selected in the projects tree.
 * @title: CodeSlayerProjectsSelection
 * @include: codeslayer/codeslayer-projects-selection.h
 */

static void codeslayer_projects_selection_class_init    (CodeSlayerProjectsSelectionClass *klass);
static void codeslayer_projects_selection_init          (CodeSlayerProjectsSelection      *projects_selection);
static void codeslayer_projects_selection_finalize      (CodeSlayerProjectsSelection      *projects_selection);
static void codeslayer_projects_selection_get_property  (GObject                          *object, 
                                                         guint                             prop_id,
                                                         GValue                           *value,
                                                         GParamSpec                       *pspec);
static void codeslayer_projects_selection_set_property  (GObject                          *object,
                                                         guint                             prop_id,
                                                         const GValue                     *value,
                                                         GParamSpec                       *pspec);

#define CODESLAYER_PROJECTS_SELECTION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROJECTS_SELECTION_TYPE, CodeSlayerProjectsSelectionPrivate))

typedef struct _CodeSlayerProjectsSelectionPrivate CodeSlayerProjectsSelectionPrivate;

struct _CodeSlayerProjectsSelectionPrivate
{
  gchar             *file_path;
  CodeSlayerProject *project;
};

G_DEFINE_TYPE (CodeSlayerProjectsSelection, codeslayer_projects_selection, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_FILE_PATH,
  PROP_PROJECT
};

static void 
codeslayer_projects_selection_class_init (CodeSlayerProjectsSelectionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_projects_selection_finalize;

  gobject_class->get_property = codeslayer_projects_selection_get_property;
  gobject_class->set_property = codeslayer_projects_selection_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerProjectsSelectionPrivate));

  /**
   * CodeSlayerProjectsSelection:file-path:
   *
   * The fully qualified file path where the projects_selection is located.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_FILE_PATH,
                                   g_param_spec_string ("file_path",
                                                        "File Path",
                                                        "File Path Object", "",
                                                        G_PARAM_READWRITE));

  /**
   * CodeSlayerProjectsSelection:project:
   *
   * a #CodeSlayerProject.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_PROJECT,
                                   g_param_spec_object ("project",
                                                        "CodeSlayerProject",
                                                        "CodeSlayerProject Object",
                                                        CODESLAYER_PROJECT_TYPE,
                                                        G_PARAM_READWRITE));
}

static void
codeslayer_projects_selection_init (CodeSlayerProjectsSelection *projects_selection)
{
  CodeSlayerProjectsSelectionPrivate *priv;
  priv = CODESLAYER_PROJECTS_SELECTION_GET_PRIVATE (projects_selection);
  priv->file_path = NULL;
}

static void
codeslayer_projects_selection_finalize (CodeSlayerProjectsSelection *projects_selection)
{
  CodeSlayerProjectsSelectionPrivate *priv;
  priv = CODESLAYER_PROJECTS_SELECTION_GET_PRIVATE (projects_selection);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  if (priv->project)
      priv->project = NULL;

  G_OBJECT_CLASS (codeslayer_projects_selection_parent_class)->finalize (G_OBJECT (projects_selection));
}

static void
codeslayer_projects_selection_get_property (GObject    *object, 
                                            guint       prop_id,
                                            GValue     *value, 
                                            GParamSpec *pspec)
{
  CodeSlayerProjectsSelectionPrivate *priv;
  CodeSlayerProjectsSelection *projects_selection;
  
  projects_selection = CODESLAYER_PROJECTS_SELECTION (object);
  priv =CODESLAYER_PROJECTS_SELECTION_GET_PRIVATE (projects_selection);

  switch (prop_id)
    {
    case PROP_FILE_PATH:
      g_value_set_string (value, priv->file_path);
      break;
    case PROP_PROJECT:
      g_value_set_pointer (value, priv->project);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_projects_selection_set_property (GObject      *object, 
                                            guint         prop_id,
                                            const GValue *value, 
                                            GParamSpec   *pspec)
{
  CodeSlayerProjectsSelection *projects_selection;
  projects_selection = CODESLAYER_PROJECTS_SELECTION (object);

  switch (prop_id)
    {
    case PROP_FILE_PATH:
      codeslayer_projects_selection_set_file_path (projects_selection, g_value_get_string (value));
      break;
    case PROP_PROJECT:
      codeslayer_projects_selection_set_project (projects_selection, CODESLAYER_PROJECT (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_projects_selection_new:
 *
 * Creates a new #CodeSlayerProjectsSelection.
 *
 * Returns: a new #CodeSlayerProjectsSelection. 
 */
CodeSlayerProjectsSelection *
codeslayer_projects_selection_new (void)
{
  return CODESLAYER_PROJECTS_SELECTION (g_object_new (codeslayer_projects_selection_get_type (), NULL));
}

/**
 * codeslayer_projects_selection_get_file_path:
 * @projects_selection: a #CodeSlayerProjectsSelection.
 *
 * Returns: the fully qualified path to the projects_selection.
 */
const gchar *
codeslayer_projects_selection_get_file_path (CodeSlayerProjectsSelection *projects_selection)
{
  return CODESLAYER_PROJECTS_SELECTION_GET_PRIVATE (projects_selection)->file_path;
}

/**
 * codeslayer_projects_selection_set_file_path:
 * @projects_selection: a #CodeSlayerProjectsSelection.
 * @file_path: the fully qualified path to the projects_selection.
 *
 * Note: this is not a URI, but rather a normal file path.
 */
void
codeslayer_projects_selection_set_file_path (CodeSlayerProjectsSelection *projects_selection,
                                             const gchar                 *file_path)
{
  CodeSlayerProjectsSelectionPrivate *priv;
  priv = CODESLAYER_PROJECTS_SELECTION_GET_PRIVATE (projects_selection);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}

/**
 * codeslayer_projects_selection_get_project:
 * @projects_selection: a #CodeSlayerProjectsSelection.
 *
 * Returns: the #CodeSlayerProject that this selection is a part of.
 */
CodeSlayerProject *
codeslayer_projects_selection_get_project (CodeSlayerProjectsSelection *projects_selection)
{
  return CODESLAYER_PROJECTS_SELECTION_GET_PRIVATE (projects_selection)->project;
}

/**
 * codeslayer_projects_selection_set_project:
 * @projects_selection: a #CodeSlayerProjectsSelection.
 * @project: the #CodeSlayerProject that this selection is a part of.
 */
void
codeslayer_projects_selection_set_project (CodeSlayerProjectsSelection *projects_selection,
                                           CodeSlayerProject           *project)
{
  CodeSlayerProjectsSelectionPrivate *priv;
  priv = CODESLAYER_PROJECTS_SELECTION_GET_PRIVATE (projects_selection);
  priv->project = project;
}
