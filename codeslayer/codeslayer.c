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

/**
 * SECTION:codeslayer
 * @short_description: The plugins context.
 * @title: CodeSlayer
 * @include: codeslayer/codeslayer.h
 */

#define GROUPS "groups"
#define PLUGINS "plugins"
#define CONFIGURATION "configuration"

static void codeslayer_class_init            (CodeSlayerClass   *klass);
static void codeslayer_init                  (CodeSlayer        *codeslayer);
static void codeslayer_finalize              (CodeSlayer        *codeslayer);

static void editor_saved_action              (CodeSlayer        *codeslayer,
                                              CodeSlayerEditor  *editor);
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

#define CODESLAYER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_TYPE, CodeSlayerPrivate))

typedef struct _CodeSlayerPrivate CodeSlayerPrivate;

struct _CodeSlayerPrivate
{
  CodeSlayerMenuBar           *menubar;
  CodeSlayerNotebook          *notebook;
  CodeSlayerProjects          *projects;
  CodeSlayerProjectProperties *project_properties;
  CodeSlayerSidePane          *side_pane;
  CodeSlayerBottomPane        *bottom_pane;
  CodeSlayerGroups            *groups;
};

enum
{
  EDITOR_SAVED,
  EDITOR_ADDED,
  EDITOR_REMOVED,
  EDITOR_SWITCHED,
  PROJECT_PROPERTIES_OPENED,
  PROJECT_PROPERTIES_SAVED,
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
codeslayer_new (CodeSlayerMenuBar           *menubar,
                CodeSlayerNotebook          *notebook,
                CodeSlayerProjects      *projects, 
                CodeSlayerProjectProperties *project_properties, 
                CodeSlayerSidePane          *side_pane,
                CodeSlayerBottomPane        *bottom_pane, 
                CodeSlayerGroups            *groups)
{
  CodeSlayerPrivate *priv;
  CodeSlayer *codeslayer;
  codeslayer = CODESLAYER (g_object_new (codeslayer_get_type (), NULL));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  priv->menubar = menubar;
  priv->notebook = notebook;
  priv->projects = projects;
  priv->project_properties = project_properties;
  priv->side_pane = side_pane;
  priv->bottom_pane = bottom_pane;
  priv->groups = groups;
  
  g_signal_connect_swapped (G_OBJECT (notebook), "page-added",
                            G_CALLBACK (editor_added_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "page-removed",
                            G_CALLBACK (editor_removed_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "switch-page",
                            G_CALLBACK (editor_switched_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (notebook), "editor-saved",
                            G_CALLBACK (editor_saved_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (projects), "properties-opened",
                            G_CALLBACK (project_properties_opened_action), codeslayer);
  
  g_signal_connect_swapped (G_OBJECT (projects), "properties-saved",
                            G_CALLBACK (project_properties_closed_action), codeslayer);
  
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
 * Returns: is TRUE if the editor is able to be found in the tree. 
 */
gboolean
codeslayer_select_editor (CodeSlayer         *codeslayer, 
                          CodeSlayerDocument *document)
{
  CodeSlayerPrivate *priv;
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), FALSE);
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  return codeslayer_projects_select_document (priv->projects, document);
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
 * codeslayer_side_pane_add:
 * @side_pane: a #CodeSlayerSidePane.
 * @widget: a #GtkWidget.
 * @title: the title for the tab.
 *
 * Returns: a #GList of #CodeSlayerEditor. Note: you need to call g_list_free
 * when you are done with the list.
 */
GList*
codeslayer_get_all_editors (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  GList *results;
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
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer); 
  g_object_ref_sink (GTK_OBJECT (menuitem));  
  codeslayer_menubar_add_tools_item (priv->menubar, GTK_WIDGET (menuitem));
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
  CodeSlayerPrivate *priv;
  g_return_if_fail (IS_CODESLAYER (codeslayer));
  priv = CODESLAYER_GET_PRIVATE (codeslayer);

  codeslayer_menubar_remove_tools_item (priv->menubar, GTK_WIDGET (menuitem));
  
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
  
  g_object_ref_sink (GTK_OBJECT (menuitem));
      
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
  CodeSlayerPrivate *priv;
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  return codeslayer_menubar_get_accel_group (priv->menubar);
}

/**
 * codeslayer_get_active_group:
 * @codeslayer: a #CodeSlayer.
 *
 * Returns: The #CodeSlayerGroup that is currently opened.
 */
CodeSlayerGroup*
codeslayer_get_active_group (CodeSlayer *codeslayer)
{
  CodeSlayerPrivate *priv;
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  return codeslayer_groups_get_active_group (priv->groups);
}

/**
 * codeslayer_get_active_group_file_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The file path to where you should place project configuration files.
 * 
 * DEPRECATED: you should use the newer codeslayer_get_active_group_folder_path()
 *
 * Returns: a newly-allocated string that must be freed with g_free().
 */
gchar*
codeslayer_get_active_group_file_path (CodeSlayer *codeslayer)
{
  CodeSlayerGroup *group;
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  group = codeslayer_get_active_group (codeslayer);
  return g_build_filename (g_get_home_dir (), CODESLAYER_HOME, GROUPS,
                           codeslayer_group_get_name (group), NULL);
}

/**
 * codeslayer_get_active_group_folder_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The folder path to where you should place project/group configuration files.
 *
 * Returns: a newly-allocated string that must be freed with g_free().
 */
gchar*
codeslayer_get_active_group_folder_path (CodeSlayer *codeslayer)
{
  CodeSlayerGroup *group;
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  group = codeslayer_get_active_group (codeslayer);
  return g_build_filename (g_get_home_dir (), CODESLAYER_HOME, GROUPS,
                           codeslayer_group_get_name (group), NULL);
}

/**
 * codeslayer_get_configuration_file_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The file path to where you should place plugin configuration files.
 * 
 * DEPRECATED: you should use the newer codeslayer_get_configuration_folder_path()
 *
 * Returns: a newly-allocated string that must be freed with g_free().
 */
gchar*
codeslayer_get_configuration_file_path (CodeSlayer *codeslayer)
{
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  return g_build_filename (g_get_home_dir (), CODESLAYER_HOME, 
                           PLUGINS, CONFIGURATION, NULL);
}

/**
 * codeslayer_get_configuration_folder_path:
 * @codeslayer: a #CodeSlayer.
 *
 * The folder path to where you should place plugin configuration files.
 *
 * Returns: a newly-allocated string that must be freed with g_free().
 */
gchar*
codeslayer_get_configuration_folder_path (CodeSlayer *codeslayer)
{
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);
  return g_build_filename (g_get_home_dir (), CODESLAYER_HOME, 
                           PLUGINS, CONFIGURATION, NULL);
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
  CodeSlayerGroup *group;
  GList *projects; 
  
  g_return_val_if_fail (IS_CODESLAYER (codeslayer), NULL);

  group = codeslayer_get_active_group (codeslayer);
  projects = codeslayer_group_get_projects (group);

  while (projects != NULL)
    {
      CodeSlayerProject *project = projects->data;
      const gchar *folder_path = codeslayer_project_get_folder_path (project);
      
      if (g_str_has_prefix (file_path, folder_path))
        return project;
      
      projects = g_list_next (projects);
    }
  
  return NULL;
}

static void
editor_saved_action (CodeSlayer       *codeslayer,
                     CodeSlayerEditor *editor)                     
{
  g_signal_emit_by_name((gpointer)codeslayer, "editor-saved", editor);
}

static void
editor_added_action (CodeSlayer  *codeslayer,
                     GtkWidget   *page,
                     guint        page_num)                     
{
  GtkWidget *editor;
  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name((gpointer)codeslayer, "editor-added", editor);
}

static void      
editor_removed_action (CodeSlayer  *codeslayer, 
                       GtkWidget   *page,
                       guint        page_num)
{
  GtkWidget *editor;
  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name((gpointer)codeslayer, "editor-removed", editor);
}

static void
editor_switched_action (CodeSlayer      *codeslayer, 
                        GtkWidget       *notebook_page,
                        guint            page_num)
{
  CodeSlayerPrivate *priv;
  GtkWidget *editor;
  GtkWidget *page;
  priv = CODESLAYER_GET_PRIVATE (codeslayer);
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page_num);  
  editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (page));
  g_signal_emit_by_name((gpointer)codeslayer, "editor-switched", editor);
}

static void 
project_properties_opened_action (CodeSlayer        *codeslayer,
                                  CodeSlayerProject *project)
{
  g_signal_emit_by_name((gpointer)codeslayer, "project-properties-opened", project);
}                                  

static void 
project_properties_closed_action (CodeSlayer        *codeslayer,
                                  CodeSlayerProject *project)
{
  g_signal_emit_by_name((gpointer)codeslayer, "project-properties-saved", project);
}                                  
