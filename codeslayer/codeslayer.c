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

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-abstract-pane.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer.h>
#include <codeslayer/codeslayer-marshaller.h>

/**
 * SECTION:codeslayer
 * @short_description: The plugins context.
 * @title: CodeSlayer
 * @include: codeslayer/codeslayer.h
 */

#define PLUGINS "plugins"
#define PROFILES "profiles"
#define PROJECT_CONFIG ".codeslayer"
#define CONFIG "config"

static void codeslayer_class_init             (CodeSlayerClass      *klass);
static void codeslayer_init                   (CodeSlayer           *codeslayer);
static void codeslayer_finalize               (CodeSlayer           *codeslayer);

static void document_saved_action             (CodeSlayer           *codeslayer,
                                               CodeSlayerSourceView *source_view);
static void documents_all_saved_action        (CodeSlayer           *codeslayer,
                                               GList                *source_views);
static void project_properties_opened_action  (CodeSlayer           *codeslayer,
                                               CodeSlayerProject    *project);
static void project_properties_closed_action  (CodeSlayer           *codeslayer,
                                               CodeSlayerProject    *project);
static void document_added_action             (CodeSlayer           *codeslayer,
                                               GtkWidget            *child,
                                               guint                 page_num);
static void document_removed_action           (CodeSlayer           *codeslayer, 
                                               GtkWidget            *child,
                                               guint                 page_num);
static void document_switched_action          (CodeSlayer           *codeslayer,
                                               GtkWidget            *child,
                                               guint                 page_num);
static void projects_changed_action           (CodeSlayer           *codeslayer);
static void verify_project_config_dir_exists  (CodeSlayerProject    *project);

#define CODESLAYER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_TYPE, CodeSlayerPrivate))

typedef struct _CodeSlayerPrivate CodeSlayerPrivate;

struct _CodeSlayerPrivate
{
  GtkWindow                   *window;
  CodeSlayerProfile           *profile;
  CodeSlayerRegistry          *registry;
  CodeSlayerMenuBar           *menu_bar;
  CodeSlayerNotebook          *notebook;
  CodeSlayerProjects          *projects;
  CodeSlayerProjectProperties *project_properties;
  CodeSlayerSidePane          *side_pane;
  CodeSlayerBottomPane        *bottom_pane;
};

enum
{
  DOCUMENT_SAVED,
  DOCUMENTS_ALL_SAVED,
  DOCUMENT_ADDED,
  DOCUMENT_REMOVED,
  DOCUMENT_SWITCHED,
  PROJECT_PROPERTIES_OPENED,
  PROJECT_PROPERTIES_SAVED,
  PROJECTS_CHANGED,
  LAST_SIGNAL
};

static guint codeslayer_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayer, codeslayer, G_TYPE_OBJECT)
     
static void 
codeslayer_class_init (CodeSlayerClass *klass)
{
  /**
   * CodeSlayer::document-saved
   * @codeslayer: the plugin that received the signal
   * @document: the #CodeSlayerSourceView that was saved
   *
   * The ::document-saved signal is emitted when an document is saved successfully
   */
  codeslayer_signals[DOCUMENT_SAVED] =
    g_signal_new ("document-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, document_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_DOCUMENT_TYPE);

  /**
   * CodeSlayer::documents-all-saved
   * @codeslayer: the plugin that received the signal
   * @documents: a #GList of #CodeSlayerSourceView objects that were saved
   *
   * The ::documents-all-saved signal is emitted when all the documents have been saved successfully
   */
  codeslayer_signals[DOCUMENTS_ALL_SAVED] =
    g_signal_new ("documents-all-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, documents_all_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayer::document-added
   * @codeslayer: the plugin that received the signal
   * @document: the #CodeSlayerSourceView that was added
   *
   * The ::document-added signal is emitted when the document is added to the notebook
   */
  codeslayer_signals[DOCUMENT_ADDED] =
    g_signal_new ("document-added", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, document_added), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_DOCUMENT_TYPE);

  /**
   * CodeSlayer::document-removed
   * @codeslayer: the plugin that received the signal
   * @document: the #CodeSlayerSourceView that was removed
   *
   * The ::document-removed signal is emitted when the document is removed from the notebook
   */
  codeslayer_signals[DOCUMENT_REMOVED] =
    g_signal_new ("document-removed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, document_removed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_DOCUMENT_TYPE);

  /**
   * CodeSlayer::document-switched
   * @codeslayer: the plugin that received the signal
   * @document: the #CodeSlayerSourceView switched to
   *
   * The ::document-switched signal is emitted when the active document is switched in the notebook
   */
  codeslayer_signals[DOCUMENT_SWITCHED] =
    g_signal_new ("document-switched", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, document_switched), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_DOCUMENT_TYPE);

  /**
   * CodeSlayer::project-properties-opened
   * @codeslayer: the plugin that received the signal
   * @project_properties: the #CodeSlayerProjectProperties switched to
   *
   * The ::project-properties-opened signal is emitted when the project properties window is opened
   */
  codeslayer_signals[PROJECT_PROPERTIES_OPENED] =
    g_signal_new ("project-properties-opened", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, project_properties_opened), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_PROJECT_TYPE);

  /**
	* CodeSlayer::project-properties-saved
	* @codeslayer: the plugin that received the signal
	* @project_properties: the #CodeSlayerProjectProperties switched to
	*
	* The ::project-properties-saved signal is emitted when the project properties window is closed
	*/
  codeslayer_signals[PROJECT_PROPERTIES_SAVED] =
    g_signal_new ("project-properties-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, project_properties_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_PROJECT_TYPE);

  /**
   * CodeSlayer::projects-changed
   * @codeslayer: the plugin that received the signal
   *
   * The ::projects-changed signal is invoked when the projects structure changed.
   */
  codeslayer_signals[PROJECTS_CHANGED] =
    g_signal_new ("projects-changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, projects_changed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPrivate));
}

static void
codeslayer_init (CodeSlayer *codeslayer) {}

static void
codeslayer_finalize (CodeSlayer *codeslayer)
{
  G_OBJECT_CLASS (codeslayer_parent_class)-> finalize (G_OBJECT (codeslayer));
}

/**
 * codeslayer_new:
 * @window: a #GtkWindow.
 * @profile: a #CodeSlayerProfile.
 * @menu_bar: a #CodeSlayerMenuBar.
 * @notebook: a #CodeSlayerNotebook.
 * @projects: a #CodeSlayerProjects.
 * @project_properties: a #CodeSlayerProjectProperties.
 * @side_pane: a #CodeSlayerSidePane.
 * @bottom_pane: a #CodeSlayerBottomPane.
 *
 * Returns: a new #CodeSlayer. 
 */
CodeSlayer*
codeslayer_new (GtkWindow                   *window,
                CodeSlayerProfile           *profile,
                CodeSlayerMenuBar           *menu_bar,
                CodeSlayerNotebook          *notebook,
                CodeSlayerProjects          *projects, 
                CodeSlayerProjectProperties *project_properties, 
                CodeSlayerSidePane          *side_pane,
                CodeSlayerBottomPane        *bottom_pane)
{
  CodeSlayerPrivate *priv;
  CodeSlayer *codeslayer;
  codeslayer = CODESLAYER (g_object_new (codeslayer_get_type (), NULL));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  priv->window = window;
  priv->profile = profile;
  priv->menu_bar = menu_bar;
  priv->notebook = notebook;
  priv->projects = projects;
  priv->project_properties = project_properties;
  priv->side_pane = side_pane;
  priv->bottom_pane = bottom_pane;
  
  g_signal_connect_swapped (G_OBJECT (notebook), "page-added",
                            G_CALLBACK (document_added_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "page-removed",
                            G_CALLBACK (document_removed_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "switch-page",
                            G_CALLBACK (document_switched_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "document-saved",
                            G_CALLBACK (document_saved_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "documents-all-saved",
                            G_CALLBACK (documents_all_saved_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (projects), "properties-opened",
                            G_CALLBACK (project_properties_opened_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (projects), "properties-saved",
                            G_CALLBACK (project_properties_closed_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (projects), "projects-changed",
                            G_CALLBACK (projects_changed_action), codeslayer);
  
  return codeslayer;
}

/**
 * codeslayer_select_document:
 * @codeslayer: a #CodeSlayer.
 * @document: a #CodeSlayerDocument.
 *
 * Finds the document based on the file path. First it will find the
 * document in the tree and then open up the document in the notebook. It is Ok
 * to call this method as much as you want because after the document is initially
 * loaded up then subsequent calls are very efficient.
 *
 * The minimum document attribute that needs to be filled in is the file_path.
 *
 * Returns: is TRUE if the document is able to be found in the tree. 
 */
gboolean
codeslayer_select_document (CodeSlayer         *codeslayer, 
                            CodeSlayerDocument *document)
{
  CodeSlayerPrivate *priv;
  CodeSlayerProject *project;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), FALSE);
  
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  
  project = codeslayer_document_get_project (document);
  if (project == NULL)
    {
      const gchar* file_path = codeslayer_document_get_file_path (document);
      project = codeslayer_get_project_by_file_path (codeslayer, file_path);
      codeslayer_document_set_project (document, project);
    }

  return codeslayer_projects_select_document (priv->projects, document);
}

/**
 * codeslayer_select_document_by_file_path:
 * @codeslayer: a #CodeSlayer.
 * @file_path: the path to the document to open.
 * @line_number: the line in the document to scroll to.
 *
 * Finds the document based on the file path. First it will find the document in the 
 * tree and then open up the document in the notebook. It is Ok to call this method 
 * as much as you want because after the document is initially loaded up then 
 * subsequent calls are very efficient.
 *
 * Returns: is TRUE if the document is able to be found in the tree. 
 */
gboolean
codeslayer_select_document_by_file_path (CodeSlayer  *codeslayer, 
                                         const gchar *file_path, 
                                         gint         line_number)
{
  CodeSlayerDocument *document;
  gboolean result;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), FALSE);
  
  document = codeslayer_document_new ();
  codeslayer_document_set_file_path (document, file_path);
  codeslayer_document_set_line_number (document, line_number);
  
  result = codeslayer_select_document (codeslayer, document);
  
  g_object_unref (document);
  
  return result;  
}                                       

/**
 * codeslayer_get_active_document:
 * @codeslayer: a #CodeSlayer.
 *
 * Returns: The document in the notebook that has focus. Will 
 *          return NULL if there is no active document.
 */
CodeSlayerDocument*
codeslayer_get_active_document (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  GtkWidget *source_view;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 0)
    return NULL;

  source_view = codeslayer_notebook_get_active_source_view (priv->notebook);
  return codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW(source_view));
}

/**
 * codeslayer_get_active_document_file_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The file path for the active document.
 *
 * Returns: a string that is owned by the document and should not be freed.
 */
const gchar*
codeslayer_get_active_document_file_path (CodeSlayer *codeslayer)
{
  CodeSlayerDocument *document;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  
  document = codeslayer_get_active_document (codeslayer);  
  if (document == NULL)
    return NULL;
  
  return codeslayer_document_get_file_path (document);;
}

/**
 * codeslayer_get_active_document_project:
 * @codeslayer: a #CodeSlayer.
 *
 * This is a convenience function so that you can easily get at the
 * project associated with the active document.
 *
 * Returns: The project that is associated with the active document. Will 
            return NULL if there is no active document.
 */
CodeSlayerProject*             
codeslayer_get_active_document_project (CodeSlayer *codeslayer)
{
  CodeSlayerDocument *document;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);

  document = codeslayer_get_active_document (codeslayer);
  if (!document)
    return NULL;
  
  return codeslayer_document_get_project (document);
}

/**
 * codeslayer_get_all_documents:
 *  @codeslayer: a #CodeSlayer.
 *
 * Returns: a list of documents. Note: you need to call g_list_free
 * when you are done with the list.
 */
GList*
codeslayer_get_all_documents (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  GList *documents = NULL;
  GList *source_views;
  GList *list;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);

  priv = CODESLAYER_GET_PRIVATE (codeslayer);  

  source_views = codeslayer_notebook_get_all_source_views (priv->notebook);
  list = source_views;
  
  while (list != NULL)
    {
      CodeSlayerSourceView *source_view = list->data;
      CodeSlayerDocument *document;
      document = codeslayer_source_view_get_document (source_view);
      documents = g_list_append (documents, document);
      list = g_list_next (list);
    }
  
  g_list_free (source_views);
  
  return documents;
}

/**
 * codeslayer_add_to_side_pane:
 * @codeslayer: a #CodeSlayer.
 * @widget: a #GtkWidget.
 * @title: the title for the tab.
 *
 * Add the given widget to the side pane. 
 */
void
codeslayer_add_to_side_pane (CodeSlayer  *codeslayer, 
                             GtkWidget   *widget,
                             const gchar *title)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_abstract_pane_add (CODESLAYER_ABSTRACT_PANE (priv->side_pane), widget, title);
}

/**
 * codeslayer_show_side_pane:
 * @codeslayer: a #CodeSlayer.
 * @widget: a #GtkWidget.
 *
 * Show the side pane and display the tab with the given widget. This
 * should be the same widget that you added to the side pane.
 */
void
codeslayer_show_side_pane (CodeSlayer *codeslayer,
                           GtkWidget  *widget)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_abstract_pane_open (CODESLAYER_ABSTRACT_PANE (priv->side_pane), widget);
}

/**
 * codeslayer_remove_from_side_pane:
 * @codeslayer: a #CodeSlayer.
 * @widget: a #GtkWidget.
 *
 * Remove the given widget from the side pane. 
 */
void
codeslayer_remove_from_side_pane (CodeSlayer *codeslayer, 
                                  GtkWidget  *widget)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_abstract_pane_remove (CODESLAYER_ABSTRACT_PANE (priv->side_pane), widget);
}                                  

/**
 * codeslayer_add_to_bottom_pane:
 * @codeslayer: a #CodeSlayer.
 * @widget: a #GtkWidget.
 * @title: the title for the tab.
 *
 * Add the given widget to the bottom pane. 
 */
void
codeslayer_add_to_bottom_pane (CodeSlayer  *codeslayer, 
                               GtkWidget   *widget,
                               const gchar *title)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_abstract_pane_add (CODESLAYER_ABSTRACT_PANE (priv->bottom_pane), widget, title);
}                               

/**
 * codeslayer_remove_from_bottom_pane:
 * @codeslayer: a #CodeSlayer.
 * @widget: a #GtkWidget.
 *
 * Remove the given widget from the bottom pane. 
 */
void
codeslayer_remove_from_bottom_pane (CodeSlayer *codeslayer, 
                                    GtkWidget  *widget)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_abstract_pane_remove (CODESLAYER_ABSTRACT_PANE (priv->bottom_pane), widget);
}

/**
 * codeslayer_show_bottom_pane:
 * @codeslayer: a #CodeSlayer.
 * @widget: a #GtkWidget.
 *
 * Show the bottom pane and display the tab with the given widget. This
 * should be the same widget that you added to the bottom pane.
 */
void
codeslayer_show_bottom_pane (CodeSlayer *codeslayer,
                             GtkWidget  *widget)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_abstract_pane_open (CODESLAYER_ABSTRACT_PANE (priv->bottom_pane), widget);
}

/**
 * codeslayer_add_to_menu_bar:
 * @codeslayer: a #CodeSlayer.
 * @menu_item: a #GtkMenuItem.
 *
 * Add the given menu item to the menu bar tools menu. 
 */
void
codeslayer_add_to_menu_bar (CodeSlayer  *codeslayer, 
                            GtkMenuItem *menu_item)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer); 
  g_object_ref_sink (menu_item);  
  codeslayer_menu_bar_add_tools_item (priv->menu_bar, GTK_WIDGET (menu_item));
}                              

/**
 * codeslayer_remove_from_menu_bar:
 * @codeslayer: a #CodeSlayer.
 * @menu_item: a #GtkMenuItem.
 *
 * Remove the given menu item from the menu bar tools menu. 
 */
void
codeslayer_remove_from_menu_bar (CodeSlayer  *codeslayer, 
                                 GtkMenuItem *menu_item)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  codeslayer_menu_bar_remove_tools_item (priv->menu_bar, GTK_WIDGET (menu_item));
  
  gtk_widget_destroy (GTK_WIDGET (menu_item));
  g_object_unref (menu_item);   
}

/**
 * codeslayer_add_to_projects_popup:
 * @codeslayer: a #CodeSlayer.
 * @menu_item: a #GtkMenuItem.
 *
 * Add the given menu item to the projects tree tools menu. 
 */
void
codeslayer_add_to_projects_popup (CodeSlayer  *codeslayer, 
                                  GtkMenuItem *menu_item)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  
  g_object_ref_sink (menu_item);
      
  codeslayer_projects_add_popup_item (priv->projects, GTK_WIDGET (menu_item));
}                                      

/**
 * codeslayer_remove_from_projects_popup:
 * @codeslayer: a #CodeSlayer.
 * @menu_item: a #GtkMenuItem.
 *
 * Remove the given menu item from the projects tree tools menu. 
 */
void
codeslayer_remove_from_projects_popup (CodeSlayer  *codeslayer, 
                                       GtkMenuItem *menu_item)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  codeslayer_projects_remove_popup_item (priv->projects, GTK_WIDGET (menu_item));
  
  gtk_widget_destroy (GTK_WIDGET (menu_item));
  g_object_unref (menu_item);   
}                                           

/**
 * codeslayer_add_to_project_properties:
 * @codeslayer: a #CodeSlayer.
 * @widget: a #GtkWidget.
 * @title: the title for the tab.
 *
 * Add the given widget to the project properties. 
 */
void
codeslayer_add_to_project_properties (CodeSlayer  *codeslayer, 
                                      GtkWidget   *widget,
                                      const gchar *title)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_project_properties_add (priv->project_properties, widget, title);
}                                      

/**
 * codeslayer_remove_from_project_properties:
 * @codeslayer: a #CodeSlayer.
 * @widget: a #GtkWidget.
 *
 * Remove the given widget to the project properties. 
 */
void
codeslayer_remove_from_project_properties (CodeSlayer *codeslayer, 
                                           GtkWidget  *widget)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_project_properties_remove (priv->project_properties, widget);
}                                           

/**
 * codeslayer_get_menu_bar_accel_group:
 * @codeslayer: a #CodeSlayer.
 *
 * Returns: The #GtkAccelGroup associated with the menu.
 */
GtkAccelGroup*
codeslayer_get_menu_bar_accel_group (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  return codeslayer_menu_bar_get_accel_group (priv->menu_bar);
}

/**
 * codeslayer_get_project_config_folder_path:
 * @codeslayer: a #CodeSlayer.
 * @project: a #CodeSlayerProject.
 *
 * The folder path to where you should place project configuration files.
 *
 * Returns: a newly-allocated string that must be freed with g_free().
 */
gchar*                    
codeslayer_get_project_config_folder_path  (CodeSlayer        *codeslayer, 
                                            CodeSlayerProject *project)
{
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  verify_project_config_dir_exists (project);
  return g_build_filename (codeslayer_project_get_folder_path (project), 
                           PROJECT_CONFIG, NULL);
}

/**
 * codeslayer_get_plugins_config_folder_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The folder path to where you should place plugin configuration files.
 *
 * Returns: a newly-allocated string that must be freed with g_free().
 */
gchar*                    
codeslayer_get_plugins_config_folder_path (CodeSlayer *codeslayer)
{
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  return g_build_filename (g_get_home_dir (), CODESLAYER_HOME, 
                           PLUGINS, CONFIG, NULL);
}

/**
 * codeslayer_get_profile_config_folder_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The folder path to where you should place profile configuration files.
 *
 * Returns: a newly-allocated string that must be freed with g_free().
 */
gchar*                    
codeslayer_get_profile_config_folder_path (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  const gchar *file_path;
  GFile *file;
  GFile *parent;
  gchar *result;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  file_path = codeslayer_profile_get_file_path (priv->profile);
  file = g_file_new_for_path (file_path);
  parent = g_file_get_parent (file);
  
  result = g_file_get_path (parent);
  
  g_object_unref (file);
  g_object_unref (parent);
  
  return result;                           
}

/**
 * codeslayer_get_project_by_file_path:
 * @codeslayer: a #CodeSlayer.
 * @file_path: the file path to match against the projects folder path.
 *
 * Returns: The #CodeSlayerProject if found, otherwise NULL.
 */
CodeSlayerProject*
codeslayer_get_project_by_file_path (CodeSlayer  *codeslayer, 
                                     const gchar *file_path)
{
  CodeSlayerPrivate *priv;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  if (!codeslayer_profile_get_enable_projects (priv->profile))
    return NULL;

  return codeslayer_profile_get_project_by_file_path (priv->profile, file_path);
}

/**
 * codeslayer_get_registry:
 * @codeslayer: a #CodeSlayer.
 *
 * Returns: The #CodeSlayerRegistry.
 */
CodeSlayerRegistry*
codeslayer_get_registry (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  CodeSlayerRegistry *registry; 
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  registry = codeslayer_profile_get_registry (priv->profile);

  return registry;
}

/**
 * codeslayer_get_projects:
 * @codeslayer: a #CodeSlayer.
 *
 * The list of #CodeSlayerProject objects within the profile.
 *
 * Returns: a newly-allocated list that must be freed with g_list_free().
 */
GList*
codeslayer_get_projects (CodeSlayer *codeslayer)
{

  CodeSlayerPrivate *priv;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  return codeslayer_profile_get_projects (priv->profile);
}

/**
 * codeslayer_get_toplevel_window:
 * @codeslayer: a #CodeSlayer.
 *
 * Returns: The outer most #GtkWindow that codeslayer uses.
 */
GtkWindow*
codeslayer_get_toplevel_window (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  return priv->window;
}

/**
 * codeslayer_create_document_linker:
 * @codeslayer: a #CodeSlayer.
 * @text_view: the text_view to create the links in.
 *
 * Returns: Creates a new @CodeSlayerDocumentLinker. You must free this with 
            g_object_unref () when done with it.
 */
CodeSlayerDocumentLinker*   
codeslayer_create_document_linker (CodeSlayer  *codeslayer,
                                   GtkTextView *text_view)
{
  return codeslayer_document_linker_new (G_OBJECT (codeslayer), text_view);
}

/**
 * codeslayer_send_plugin_message:
 * @codeslayer: a #CodeSlayer.
 * @message: the message that was sent.
 */
void 
codeslayer_send_plugin_message (CodeSlayer  *codeslayer,
                                const gchar *message, 
                                const gchar *program)
{
  g_signal_emit_by_name ((gpointer) codeslayer, "plugin-message", message, program);
}

static void
document_saved_action (CodeSlayer       *codeslayer,
                       CodeSlayerSourceView *source_view)                     
{
  g_signal_emit_by_name ((gpointer) codeslayer, "document-saved", 
                         codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view)));
}

static void
documents_all_saved_action (CodeSlayer *codeslayer,
                            GList      *source_views)
{
  GList *documents = NULL;

  while (source_views != NULL)
    {
      CodeSlayerSourceView *source_view = source_views->data;
      CodeSlayerDocument *document;
      document = codeslayer_source_view_get_document (source_view);
      documents = g_list_append (documents, document);
      source_views = g_list_next (source_views);
    }

  g_signal_emit_by_name ((gpointer) codeslayer, "documents-all-saved", documents);
  
  g_list_free (documents);
}

static void
document_added_action (CodeSlayer *codeslayer,
                       GtkWidget  *page,
                       guint       page_num)                     
{
  GtkWidget *source_view;
  source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name ((gpointer) codeslayer, "document-added", 
                         codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view)));
}

static void      
document_removed_action (CodeSlayer *codeslayer, 
                         GtkWidget  *page,
                         guint       page_num)
{
  GtkWidget *source_view;
  source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name ((gpointer) codeslayer, "document-removed", 
                         codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view)));
}

static void
document_switched_action (CodeSlayer *codeslayer, 
                          GtkWidget  *notebook_page,
                          guint       page_num)
{
  CodeSlayerPrivate *priv;
  GtkWidget *source_view;
  GtkWidget *page;
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page_num);  
  source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name ((gpointer) codeslayer, "document-switched", 
                         codeslayer_source_view_get_document (CODESLAYER_SOURCE_VIEW (source_view)));
}

static void 
project_properties_opened_action (CodeSlayer        *codeslayer,
                                  CodeSlayerProject *project)
{
  g_signal_emit_by_name ((gpointer) codeslayer, "project-properties-opened", project);
}                                  

static void 
project_properties_closed_action (CodeSlayer        *codeslayer,
                                  CodeSlayerProject *project)
{
  g_signal_emit_by_name((gpointer) codeslayer, "project-properties-saved", project);
}                                  

static void 
projects_changed_action (CodeSlayer *codeslayer)
{
  g_signal_emit_by_name((gpointer) codeslayer, "projects-changed");
}

static void
verify_project_config_dir_exists (CodeSlayerProject *project)
{
  gchar *config_dir;
  GFile *file;
  
  config_dir = g_build_filename (codeslayer_project_get_folder_path (project),
                                 PROJECT_CONFIG, NULL);
                           
  file = g_file_new_for_path (config_dir);

  if (!g_file_query_exists (file, NULL)) 
    g_file_make_directory (file, NULL, NULL);

  g_free (config_dir);
  g_object_unref (file);
}
