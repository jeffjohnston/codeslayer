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

#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-project.h>

/**
 * SECTION:codeslayer-document
 * @short_description: Represents the source file to create the source view.
 * @title: CodeSlayerDocument
 * @include: codeslayer/codeslayer-document.h
 *
 * The document represents the source file loaded into the source view. This 
 * abstraction is very useful because you can create a document and then 
 * pass it to the codeslayer_projects_select_document() method. 
 * This in turn selects the document in the projects tree and adds a page in 
 * the notebook. It also automatically takes into account features like the 
 * documents line number. This means that if you load a document that has 
 * the line number specified it will load the document and the scroll to the 
 * proper line in the source view.
 */

static void codeslayer_document_class_init    (CodeSlayerDocumentClass *klass);
static void codeslayer_document_init          (CodeSlayerDocument      *document);
static void codeslayer_document_finalize      (CodeSlayerDocument      *document);
static void codeslayer_document_get_property  (GObject                 *object, 
                                               guint                    prop_id,
                                               GValue                  *value,
                                               GParamSpec              *pspec);
static void codeslayer_document_set_property  (GObject                 *object,
                                               guint                    prop_id,
                                               const GValue            *value,
                                               GParamSpec              *pspec);

#define CODESLAYER_DOCUMENT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_DOCUMENT_TYPE, CodeSlayerDocumentPrivate))

typedef struct _CodeSlayerDocumentPrivate CodeSlayerDocumentPrivate;

struct _CodeSlayerDocumentPrivate
{
  gint                 line_number;
  GtkTreeRowReference *tree_row_reference;
  gchar               *name;
  gchar               *file_path;
  gboolean             active;
  CodeSlayerProject   *project;
  GtkWidget           *source_view;
};

G_DEFINE_TYPE (CodeSlayerDocument, codeslayer_document, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_LINE_NUMBER,
  PROP_NAME,
  PROP_FILE_PATH,
  PROP_ACTIVE,
  PROP_PROJECT,
  PROP_PROJECT_KEY,
  PROP_TREE_ROW_REFERENCE
};

static void 
codeslayer_document_class_init (CodeSlayerDocumentClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_document_finalize;

  gobject_class->get_property = codeslayer_document_get_property;
  gobject_class->set_property = codeslayer_document_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerDocumentPrivate));

  /**
   * CodeSlayerDocument:line-number:
   *
   * The line that the source view should scroll to.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_LINE_NUMBER,
                                   g_param_spec_int ("line_number",
                                                     "Line Number",
                                                     "Line Number", 
                                                     0, 100000, 0,
                                                     G_PARAM_READWRITE));

  /**
   * CodeSlayerDocument:name:
   *
   * The name of the document.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "Name",
                                                        "Name", "",
                                                        G_PARAM_READWRITE));

  /**
   * CodeSlayerDocument:file-path:
   *
   * The fully qualified file path where the document is located.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_FILE_PATH,
                                   g_param_spec_string ("file_path",
                                                        "File Path",
                                                        "File Path", "",
                                                        G_PARAM_READWRITE));

  /**
   * CodeSlayerDocument:active:
   *
   * The currently selected document.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_ACTIVE,
                                   g_param_spec_boolean ("active", 
                                                         "Active",
                                                         "Active", FALSE,
                                                         G_PARAM_READWRITE));

  /**
   * CodeSlayerDocument:project:
   *
   * a #CodeSlayerProject.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_PROJECT,
                                   g_param_spec_object ("project",
                                                        "CodeSlayerProject",
                                                        "CodeSlayerProject",
                                                        CODESLAYER_PROJECT_TYPE,
                                                        G_PARAM_READWRITE));

  /**
   * CodeSlayerDocument:tree-row-reference:
   *
   * The reference to the node in the tree.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_TREE_ROW_REFERENCE,
                                   g_param_spec_pointer ("tree_row_reference",
                                                         "GtkTreeRowReference",
                                                         "GtkTreeRowReference",
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_document_init (CodeSlayerDocument *document)
{
  CodeSlayerDocumentPrivate *priv;
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);
  priv->file_path = NULL;
  priv->active = FALSE;
  priv->project = NULL;
  priv->source_view = NULL;
  priv->tree_row_reference = NULL;
}

static void
codeslayer_document_finalize (CodeSlayerDocument *document)
{
  CodeSlayerDocumentPrivate *priv;
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);

  if (priv->file_path != NULL)
    g_free (priv->file_path);

  if (priv->tree_row_reference != NULL)
    gtk_tree_row_reference_free (priv->tree_row_reference);      
      
  G_OBJECT_CLASS (codeslayer_document_parent_class)->finalize (G_OBJECT (document));
  
  g_print ("codeslayer_document_finalize\n");
}

static void
codeslayer_document_get_property (GObject    *object, 
                                  guint       prop_id,
                                  GValue     *value, 
                                  GParamSpec *pspec)
{
  CodeSlayerDocumentPrivate *priv;
  CodeSlayerDocument *document;
  
  document = CODESLAYER_DOCUMENT (object);
  priv =CODESLAYER_DOCUMENT_GET_PRIVATE (document);

  switch (prop_id)
    {
    case PROP_LINE_NUMBER:
      g_value_set_int (value, priv->line_number);
      break;
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_FILE_PATH:
      g_value_set_string (value, priv->file_path);
      break;
    case PROP_ACTIVE:
      g_value_set_boolean (value, priv->active);
      break;
    case PROP_PROJECT:
      g_value_set_pointer (value, priv->project);
      break;
    case PROP_TREE_ROW_REFERENCE:
      g_value_set_pointer (value, priv->tree_row_reference);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_document_set_property (GObject      *object, 
                                  guint         prop_id,
                                  const GValue *value, 
                                  GParamSpec   *pspec)
{
  CodeSlayerDocument *document;
  document = CODESLAYER_DOCUMENT (object);

  switch (prop_id)
    {
    case PROP_LINE_NUMBER:
      codeslayer_document_set_line_number (document, g_value_get_int (value));
      break;
    case PROP_NAME:
      codeslayer_document_set_name (document, g_value_get_string (value));
      break;
    case PROP_FILE_PATH:
      codeslayer_document_set_file_path (document, g_value_get_string (value));
      break;
    case PROP_ACTIVE:
      codeslayer_document_set_active (document, g_value_get_boolean (value));
      break;
    case PROP_PROJECT:
      codeslayer_document_set_project (document, CODESLAYER_PROJECT (value));
      break;
    case PROP_TREE_ROW_REFERENCE:
      codeslayer_document_set_tree_row_reference (document, g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_document_new:
 *
 * Creates a new #CodeSlayerDocument.
 *
 * Returns: a new #CodeSlayerDocument. 
 */
CodeSlayerDocument *
codeslayer_document_new (void)
{
  return CODESLAYER_DOCUMENT (g_object_new (codeslayer_document_get_type (), NULL));
}

/**
 * codeslayer_document_get_line_number:
 * @document: a #CodeSlayerDocument.
 *
 * Returns: the line number to scroll to when loaded into the source view.
 */
const gint
codeslayer_document_get_line_number (CodeSlayerDocument *document)
{
  return CODESLAYER_DOCUMENT_GET_PRIVATE (document)->line_number;
}

/**
 * codeslayer_document_set_line_number:
 * @document: a #CodeSlayerDocument.
 * @line_number: the line number to scroll to when loaded into the source view.
 */
void
codeslayer_document_set_line_number (CodeSlayerDocument *document,
                                     const gint          line_number)
{
  CodeSlayerDocumentPrivate *priv;
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);
  priv->line_number = line_number;
}

/**
 * codeslayer_document_get_tree_row_reference:
 * @document: a #CodeSlayerDocument.
 *
 * Returns: the #GtkTreeRowReference within the projects tree.
 */
GtkTreeRowReference *
codeslayer_document_get_tree_row_reference (CodeSlayerDocument *document)
{
  return CODESLAYER_DOCUMENT_GET_PRIVATE (document)->tree_row_reference;
}

/**
 * codeslayer_document_set_tree_row_reference:
 * @document: a #CodeSlayerDocument.
 * @tree_row_reference: a #GtkTreeRowReference so the document can 
 *                      keep its position within the projects tree.
 */
void
codeslayer_document_set_tree_row_reference (CodeSlayerDocument  *document,
                                            GtkTreeRowReference *tree_row_reference)
{
  CodeSlayerDocumentPrivate *priv;
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);

  if (priv->tree_row_reference != NULL)
    gtk_tree_row_reference_free (priv->tree_row_reference);      

  priv->tree_row_reference = tree_row_reference;
}

/**
 * codeslayer_document_get_name:
 * @document: a #CodeSlayerDocument.
 *
 * Returns: the name of the document.
 */
const gchar *
codeslayer_document_get_name (CodeSlayerDocument *document)
{
  return CODESLAYER_DOCUMENT_GET_PRIVATE (document)->name;
}

/**
 * codeslayer_document_set_name:
 * @document: a #CodeSlayerDocument.
 * @name: the name of the document.
 *
 * This call is used internally to give a name to a document that does not 
 * have a file path specified (Untitled 1, Untitled 1, etc...). If there is 
 * a file path specified then the name is overridden to be the base name of 
 * the file path.
 */
void
codeslayer_document_set_name (CodeSlayerDocument *document,
                              const gchar        *name)
{
  CodeSlayerDocumentPrivate *priv;
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

/**
 * codeslayer_document_get_file_path:
 * @document: a #CodeSlayerDocument.
 *
 * Returns: the fully qualified path to the document.
 */
const gchar *
codeslayer_document_get_file_path (CodeSlayerDocument *document)
{
  return CODESLAYER_DOCUMENT_GET_PRIVATE (document)->file_path;
}

/**
 * codeslayer_document_set_file_path:
 * @document: a #CodeSlayerDocument.
 * @file_path: the fully qualified path to the document.
 *
 * Note: this is not a URI, but rather a normal file path.
 */
void
codeslayer_document_set_file_path (CodeSlayerDocument *document,
                                   const gchar        *file_path)
{
  CodeSlayerDocumentPrivate *priv;
  gchar *name;
  
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);
  
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
  
  name = g_path_get_basename (file_path);
  codeslayer_document_set_name (document, name);
  g_free (name);
}

/**
 * codeslayer_document_get_active:
 * @document: a #CodeSlayerDocument.
 *
 * Note: this is only used when the active group is first loaded up so that 
 *       the corresponding source view is selected. After 
 *       that it is not updated until the active group is saved.
 *
 * Returns: is TRUE if this is the active document in the project. 
 */
const gboolean
codeslayer_document_get_active (CodeSlayerDocument *document)
{
  return CODESLAYER_DOCUMENT_GET_PRIVATE (document)->active;
}

/**
 * codeslayer_document_set_active:
 * @document: a #CodeSlayerDocument.
 * @active: TRUE if this is the active document in the project.
 */
void
codeslayer_document_set_active (CodeSlayerDocument *document,
                                const gboolean active)
{
  CodeSlayerDocumentPrivate *priv;
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);
  priv->active = active;
}

/**
 * codeslayer_document_get_project:
 * @document: a #CodeSlayerDocument.
 *
 * Returns: the #CodeSlayerProject that this document is a part of.
 */
CodeSlayerProject *
codeslayer_document_get_project (CodeSlayerDocument *document)
{
  return CODESLAYER_DOCUMENT_GET_PRIVATE (document)->project;
}

/**
 * codeslayer_document_set_project:
 * @document: a #CodeSlayerDocument.
 * @project: the #CodeSlayerProject that this document is a part of.
 */
void
codeslayer_document_set_project (CodeSlayerDocument *document,
                                 CodeSlayerProject  *project)
{
  CodeSlayerDocumentPrivate *priv;
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);
  priv->project = project;
}

/**
 * codeslayer_document_get_source_view:
 * @document: a #CodeSlayerDocument.
 *
 * Returns: the #CodeSlayerSourceView that this document is attached to.
 */
GtkWidget*
codeslayer_document_get_source_view (CodeSlayerDocument *document)
{
  return CODESLAYER_DOCUMENT_GET_PRIVATE (document)->source_view;
}

/**
 * codeslayer_document_set_source_view:
 * @document: a #CodeSlayerDocument.
 * @source_view: the #CodeSlayerSourceView that this document is attached to.
 */
void
codeslayer_document_set_source_view (CodeSlayerDocument *document, 
                                     GtkWidget          *source_view)
{
  CodeSlayerDocumentPrivate *priv;
  priv = CODESLAYER_DOCUMENT_GET_PRIVATE (document);
  priv->source_view = source_view;
}                                     
