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
#include <codeslayer/codeslayer-process.h>
#include <codeslayer/codeslayer.h>
#include <codeslayer/codeslayer-marshaller.h>

/**
 * SECTION:codeslayer
 * @short_description: The plugins context.
 * @title: CodeSlayer
 * @include: codeslayer/codeslayer.h
 */

#define PLUGINS "plugins"
#define PROJECT_CONFIG ".codeslayer"
#define CONFIG "config"

static void codeslayer_class_init            (CodeSlayerClass   *klass);
static void codeslayer_init                  (CodeSlayer        *codeslayer);
static void codeslayer_finalize              (CodeSlayer        *codeslayer);

static void editor_saved_action              (CodeSlayer        *codeslayer,
                                              CodeSlayerEditor  *editor);
static void editors_all_saved_action         (CodeSlayer        *codeslayer,
                                              GList             *editors);
static void project_properties_opened_action (CodeSlayer        *codeslayer,
                                              CodeSlayerProject *project);
static void project_properties_closed_action (CodeSlayer        *codeslayer,
                                              CodeSlayerProject *project);
static void editor_added_action              (CodeSlayer        *codeslayer,
                                              GtkWidget         *child,
                                              guint              page_num);
static void editor_removed_action            (CodeSlayer        *codeslayer, 
                                              GtkWidget         *child,
                                              guint              page_num);
static void editor_switched_action           (CodeSlayer        *codeslayer,
                                              GtkWidget         *child,
                                              guint              page_num);
static void projects_changed_action          (CodeSlayer        *codeslayer);
static void verify_project_config_dir_exists (CodeSlayerProject *project);

#define CODESLAYER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_TYPE, CodeSlayerPrivate))

typedef struct _CodeSlayerPrivate CodeSlayerPrivate;

struct _CodeSlayerPrivate
{
  GtkWindow                   *window;
  CodeSlayerConfigHandler     *config_handler;
  CodeSlayerRegistry          *registry;
  CodeSlayerProcesses         *processes;
  CodeSlayerMenuBar           *menu_bar;
  CodeSlayerNotebook          *notebook;
  CodeSlayerProjects          *projects;
  CodeSlayerProjectProperties *project_properties;
  CodeSlayerSidePane          *side_pane;
  CodeSlayerBottomPane        *bottom_pane;
};

enum
{
  EDITOR_SAVED,
  EDITORS_ALL_SAVED,
  EDITOR_ADDED,
  EDITOR_REMOVED,
  EDITOR_SWITCHED,
  PATH_NAVIGATED,
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
   * CodeSlayer::editor-saved
   * @codeslayer: the plugin that received the signal
   * @editor: the #CodeSlayerEditor that was saved
   *
   * The ::editor-saved signal is emitted when an editor is saved successfully
   */
  codeslayer_signals[EDITOR_SAVED] =
    g_signal_new ("editor-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, editor_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_EDITOR_TYPE);

  /**
   * CodeSlayer::editors-all-saved
   * @codeslayer: the plugin that received the signal
   * @editors: a #GList of #CodeSlayerEditor objects that were saved
   *
   * The ::editors-all-saved signal is emitted when all the editors have been saved successfully
   */
  codeslayer_signals[EDITORS_ALL_SAVED] =
    g_signal_new ("editors-all-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, editors_all_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayer::editor-added
   * @codeslayer: the plugin that received the signal
   * @editor: the #CodeSlayerEditor that was added
   *
   * The ::editor-added signal is emitted when the editor is added to the notebook
   */
  codeslayer_signals[EDITOR_ADDED] =
    g_signal_new ("editor-added", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, editor_added), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_EDITOR_TYPE);

  /**
   * CodeSlayer::editor-removed
   * @codeslayer: the plugin that received the signal
   * @editor: the #CodeSlayerEditor that was removed
   *
   * The ::editor-removed signal is emitted when the editor is removed from the notebook
   */
  codeslayer_signals[EDITOR_REMOVED] =
    g_signal_new ("editor-removed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, editor_removed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_EDITOR_TYPE);

  /**
   * CodeSlayer::editor-switched
   * @codeslayer: the plugin that received the signal
   * @editor: the #CodeSlayerEditor switched to
   *
   * The ::editor-switched signal is emitted when the active editor is switched in the notebook
   */
  codeslayer_signals[EDITOR_SWITCHED] =
    g_signal_new ("editor-switched", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, editor_switched), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_EDITOR_TYPE);

  /**
   * CodeSlayer::path-navigated
   * @codeslayer: the plugin that received the signal
   * @from_file_path: the file path navigated from
   * @from_line_number: the line number navigated from
   * @to_file_path: the file path navigated to
   * @to_line_number: the line number navigated to
   *
   * The ::path-navigated signal is emitted when the editor is navigated to in the notebook
   */
  codeslayer_signals[PATH_NAVIGATED] =
    g_signal_new ("path-navigated", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerClass, path_navigated), 
                  NULL, NULL,
                  _codeslayer_marshal_VOID__STRING_INT_STRING_INT, G_TYPE_NONE, 4, 
                  G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);

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

CodeSlayer*
codeslayer_new (GtkWindow                   *window,
                CodeSlayerConfigHandler     *config_handler,
                CodeSlayerRegistry          *registry, 
                CodeSlayerProcesses         *processes, 
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
  priv->config_handler = config_handler;
  priv->registry = registry;
  priv->processes = processes;
  priv->menu_bar = menu_bar;
  priv->notebook = notebook;
  priv->projects = projects;
  priv->project_properties = project_properties;
  priv->side_pane = side_pane;
  priv->bottom_pane = bottom_pane;
  
  g_signal_connect_swapped (G_OBJECT (notebook), "page-added",
                            G_CALLBACK (editor_added_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "page-removed",
                            G_CALLBACK (editor_removed_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "switch-page",
                            G_CALLBACK (editor_switched_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "editor-saved",
                            G_CALLBACK (editor_saved_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "editors-all-saved",
                            G_CALLBACK (editors_all_saved_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (projects), "properties-opened",
                            G_CALLBACK (project_properties_opened_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (projects), "properties-saved",
                            G_CALLBACK (project_properties_closed_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (projects), "projects-changed",
                            G_CALLBACK (projects_changed_action), codeslayer);
  
  return codeslayer;
}

/**
 * codeslayer_select_editor:
 * @codeslayer: a #CodeSlayer.
 * @document: a #CodeSlayerDocument.
 *
 * Finds the editor based on the document file path. First it will find the
 * document in the tree and then open up the editor in the notebook. It is Ok
 * to call this method as much as you want because after the editor is initially
 * loaded up then subsequent calls are very efficient.
 *
 * The minimum document attribute that needs to be filled in is the file_path.
 *
 * Returns: is TRUE if the editor is able to be found in the tree. 
 */
gboolean
codeslayer_select_editor (CodeSlayer         *codeslayer, 
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
 * codeslayer_select_editor_by_file_path:
 * @codeslayer: a #CodeSlayer.
 * @file_path: the path to the editor to open.
 * @line_number: the line in the editor to scroll to.
 *
 * Finds the editor based on the file path. First it will find the document in the 
 * tree and then open up the editor in the notebook. It is Ok to call this method 
 * as much as you want because after the editor is initially loaded up then 
 * subsequent calls are very efficient.
 *
 * Returns: is TRUE if the editor is able to be found in the tree. 
 */
gboolean
codeslayer_select_editor_by_file_path (CodeSlayer  *codeslayer, 
                                       const gchar *file_path, 
                                       gint         line_number)
{
  CodeSlayerDocument *document;
  gboolean result;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), FALSE);
  
  document = codeslayer_document_new ();
  codeslayer_document_set_file_path (document, file_path);
  codeslayer_document_set_line_number (document, line_number);
  
  result = codeslayer_select_editor (codeslayer, document);
  
  g_object_unref (document);
  
  return result;  
}                                       

/**
 * codeslayer_get_active_editor:
 * @codeslayer: a #CodeSlayer.
 *
 * Returns: The editor in the notebook that has focus. Will 
 *          return NULL if there is no active editor.
 */
CodeSlayerEditor*
codeslayer_get_active_editor (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  GtkWidget *editor;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 0)
    return NULL;

  editor = codeslayer_notebook_get_active_editor (priv->notebook);
  return CODESLAYER_EDITOR(editor);
}

/**
 * codeslayer_get_active_editor_file_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The file path for the active editor.
 *
 * Returns: a string that is owned by the editor and should not be freed.
 */
const gchar*
codeslayer_get_active_editor_file_path (CodeSlayer *codeslayer)
{
  CodeSlayerEditor *editor;
  CodeSlayerDocument *document;
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  
  editor = codeslayer_get_active_editor (codeslayer);
  
  if (editor == NULL)
    return NULL;
  
  document = codeslayer_editor_get_document (editor);
  return codeslayer_document_get_file_path (document);;
}

/**
 * codeslayer_get_active_editor_document:
 * @codeslayer: a #CodeSlayer.
 *
 * This is a convenience function so that you can easily get at the
 * document associated with the active editor.
 *
 * Returns: The document that is associated with the active editor. Will 
 *          return NULL if there is no active editor.
 */
CodeSlayerDocument*             
codeslayer_get_active_editor_document (CodeSlayer *codeslayer)
{
  CodeSlayerEditor *editor;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);

  editor = codeslayer_get_active_editor (codeslayer);
  if (!editor)
    return NULL;

  return codeslayer_editor_get_document (editor);
}

/**
 * codeslayer_get_active_editor_project:
 * @codeslayer: a #CodeSlayer.
 *
 * This is a convenience function so that you can easily get at the
 * project associated with the active editor.
 *
 * Returns: The project that is associated with the active editor. Will 
            return NULL if there is no active editor.
 */
CodeSlayerProject*             
codeslayer_get_active_editor_project (CodeSlayer *codeslayer)
{
  CodeSlayerEditor *editor;
  CodeSlayerDocument *document;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);

  editor = codeslayer_get_active_editor (codeslayer);
  if (!editor)
    return NULL;
  
  document = codeslayer_editor_get_document (editor);

  return codeslayer_document_get_project (document);
}

/**
 * codeslayer_get_all_editors:
 *  @codeslayer: a #CodeSlayer.
 *
 * Returns: a #GList of #CodeSlayerEditor. Note: you need to call g_list_free
 * when you are done with the list.
 */
GList*
codeslayer_get_all_editors (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  GList *results = NULL;
  gint pages;
  gint i;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));
  
  for (i = 0; i < pages; i++)
    {
      GtkWidget *notebook_page;
      GtkWidget *editor; 
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), i);
      editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      results = g_list_prepend (results, editor);
    }
    
  return g_list_reverse (results);
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
 * codeslayer_add_to_menubar:
 * @codeslayer: a #CodeSlayer.
 * @menuitem: a #GtkMenuItem.
 *
 * Add the given menu item to the menubar tools menu. 
 */
void
codeslayer_add_to_menubar (CodeSlayer  *codeslayer, 
                           GtkMenuItem *menuitem)
{
  codeslayer_add_to_menu_bar (codeslayer, menuitem);
}                              

/**
 * codeslayer_add_to_menu_bar:
 * @codeslayer: a #CodeSlayer.
 * @menuitem: a #GtkMenuItem.
 *
 * Add the given menu item to the menu bar tools menu. 
 */
void
codeslayer_add_to_menu_bar (CodeSlayer  *codeslayer, 
                           GtkMenuItem *menuitem)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer); 
  g_object_ref_sink (menuitem);  
  codeslayer_menu_bar_add_tools_item (priv->menu_bar, GTK_WIDGET (menuitem));
}                              

/**
 * codeslayer_remove_from_menubar:
 * @codeslayer: a #CodeSlayer.
 * @menuitem: a #GtkMenuItem.
 *
 * Remove the given menu item from the menubar tools menu. 
 */
void
codeslayer_remove_from_menubar (CodeSlayer  *codeslayer, 
                                GtkMenuItem *menuitem)
{
  codeslayer_remove_from_menu_bar (codeslayer, menuitem);
}

/**
 * codeslayer_remove_from_menu_bar:
 * @codeslayer: a #CodeSlayer.
 * @menuitem: a #GtkMenuItem.
 *
 * Remove the given menu item from the menu bar tools menu. 
 */
void
codeslayer_remove_from_menu_bar (CodeSlayer  *codeslayer, 
                                GtkMenuItem *menuitem)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  codeslayer_menu_bar_remove_tools_item (priv->menu_bar, GTK_WIDGET (menuitem));
  
  gtk_widget_destroy (GTK_WIDGET (menuitem));
  g_object_unref (menuitem);   
}

/**
 * codeslayer_add_to_projects_popup:
 * @codeslayer: a #CodeSlayer.
 * @menuitem: a #GtkMenuItem.
 *
 * Add the given menu item to the projects tree tools menu. 
 */
void
codeslayer_add_to_projects_popup (CodeSlayer  *codeslayer, 
                                  GtkMenuItem *menuitem)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  
  g_object_ref_sink (menuitem);
      
  codeslayer_projects_add_popup_item (priv->projects, GTK_WIDGET (menuitem));
}                                      

/**
 * codeslayer_remove_from_projects_popup:
 * @codeslayer: a #CodeSlayer.
 * @menuitem: a #GtkMenuItem.
 *
 * Remove the given menu item from the projects tree tools menu. 
 */
void
codeslayer_remove_from_projects_popup (CodeSlayer  *codeslayer, 
                                       GtkMenuItem *menuitem)
{
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  codeslayer_projects_remove_popup_item (priv->projects, GTK_WIDGET (menuitem));
  
  gtk_widget_destroy (GTK_WIDGET (menuitem));
  g_object_unref (menuitem);   
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
 * codeslayer_get_menubar_accel_group:
 * @codeslayer: a #CodeSlayer.
 *
 * Returns: The #GtkAccelGroup associated with the menu.
 */
GtkAccelGroup*
codeslayer_get_menubar_accel_group (CodeSlayer *codeslayer)
{
  return codeslayer_get_menu_bar_accel_group (codeslayer);
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
 * codeslayer_get_configuration_folder_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The folder path to where you should place plugin configuration files.
 *
 * Returns: a newly-allocated string that must be freed with g_free().
 *
 * Deprecated: 3.0: use codeslayer_get_plugins_config_folder_path now.
 */
gchar*
codeslayer_get_configuration_folder_path (CodeSlayer *codeslayer)
{
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  return g_build_filename (g_get_home_dir (), CODESLAYER_HOME, 
                           PLUGINS, CONFIG, NULL);
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
  CodeSlayerConfig *config;
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  config = codeslayer_config_handler_get_config (priv->config_handler);
  
  if (!codeslayer_config_get_projects_mode (config))
    return NULL;

  return codeslayer_config_get_project_by_file_path (config, file_path);
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
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  return priv->registry;
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
 * codeslayer_add_to_process_bar:
 * @codeslayer: a #CodeSlayer.
 * @name: the name of the process.
 * @func: a #StopProcessFunc is a callback to request that the process be stopped
 * @data: user data supplied to the #StopProcessFunc
 *
 * Returns: The identifier for the process.
 */
gint
codeslayer_add_to_process_bar (CodeSlayer      *codeslayer,
                               const gchar     *name,
                               StopProcessFunc  func,
                               gpointer         data)
{
  CodeSlayerPrivate *priv;
  CodeSlayerProcess *process;
  gint id;

  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  
  id = g_random_int ();
  
  process = codeslayer_process_new (id);
  codeslayer_process_set_name (process, name);
  codeslayer_process_set_func (process, func);
  codeslayer_process_set_data (process, data);
  
  g_object_force_floating (G_OBJECT (process));

  codeslayer_processes_add (priv->processes, process);
  
  return id;
}

/**
 * codeslayer_remove_from_processes:
 * @codeslayer: a #CodeSlayer.
 * @id: the identifier for the process.
 */
void 
codeslayer_remove_from_process_bar (CodeSlayer *codeslayer,
                                    gint        id)
{
  CodeSlayerPrivate *priv;
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  codeslayer_processes_remove (priv->processes, id);
}

/**
 * codeslayer_get_editor_linker:
 * @codeslayer: a #CodeSlayer.
 * @text_view: the text_view to create the links in.
 *
 * Returns: Creates a new @CodeSlayerEditorLinker. You must free this with 
 *          g_object_unref () when done with it.
 *
 * Deprecated: 3.0: use codeslayer_create_editor_linker now.
 */
CodeSlayerEditorLinker*   
codeslayer_get_editor_linker (CodeSlayer  *codeslayer,
                              GtkTextView *text_view)
{
  return codeslayer_editor_linker_new (G_OBJECT (codeslayer), text_view);
}

/**
 * codeslayer_create_editor_linker:
 * @codeslayer: a #CodeSlayer.
 * @text_view: the text_view to create the links in.
 *
 * Returns: Creates a new @CodeSlayerEditorLinker. You must free this with 
            g_object_unref () when done with it.
 */
CodeSlayerEditorLinker*   
codeslayer_create_editor_linker (CodeSlayer  *codeslayer,
                                 GtkTextView *text_view)
{
  return codeslayer_editor_linker_new (G_OBJECT (codeslayer), text_view);
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
editor_saved_action (CodeSlayer       *codeslayer,
                     CodeSlayerEditor *editor)                     
{
  g_signal_emit_by_name ((gpointer) codeslayer, "editor-saved", editor);
}

static void
editors_all_saved_action (CodeSlayer *codeslayer,
                          GList      *editors)
{
  g_signal_emit_by_name ((gpointer) codeslayer, "editors-all-saved", editors);
}

static void
editor_added_action (CodeSlayer *codeslayer,
                     GtkWidget  *page,
                     guint       page_num)                     
{
  GtkWidget *editor;
  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name ((gpointer) codeslayer, "editor-added", editor);
}

static void      
editor_removed_action (CodeSlayer *codeslayer, 
                       GtkWidget  *page,
                       guint       page_num)
{
  GtkWidget *editor;
  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name ((gpointer) codeslayer, "editor-removed", editor);
}

static void
editor_switched_action (CodeSlayer *codeslayer, 
                        GtkWidget  *notebook_page,
                        guint       page_num)
{
  CodeSlayerPrivate *priv;
  GtkWidget *editor;
  GtkWidget *page;
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page_num);  
  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name ((gpointer) codeslayer, "editor-switched", editor);
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
