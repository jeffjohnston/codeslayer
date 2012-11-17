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

#include <gdk/gdkkeysyms.h>
#include <codeslayer/codeslayer-projects-selection.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-project-properties.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-group.h>
#include <codeslayer/codeslayer-project.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-menuitem.h>
#include <codeslayer/codeslayer-marshaller.h>

/**
 * SECTION:codeslayer-projects
 * @short_description: The tree view that contains the projects.
 * @title: CodeSlayerProjects
 * @include: codeslayer/codeslayer-projects.h
 */

static void codeslayer_projects_class_init      (CodeSlayerProjectsClass *klass);
static void codeslayer_projects_init            (CodeSlayerProjects      *projects);
static void codeslayer_projects_finalize        (CodeSlayerProjects      *projects);

static gboolean treeview_row_expanded           (CodeSlayerProjects      *projects, 
                                                 GtkTreeIter             *iter, 
                                                 GtkTreePath             *path);
static gint sort_iter_compare_func              (GtkTreeModel            *model, 
                                                 GtkTreeIter             *a, 
                                                 GtkTreeIter             *b, 
                                                 gpointer                 userdata);
static void add_project                         (CodeSlayerProject       *project, 
                                                 CodeSlayerProjects      *projects);
static gboolean select_document                 (CodeSlayerDocument      *document, 
                                                 CodeSlayerProjects      *projects);

static void append_treestore_children           (CodeSlayerProjects      *projects, 
                                                 CodeSlayerProject       *project, 
                                                 GtkTreeIter              iter, 
                                                 const gchar             *folder_path);
static gboolean is_file_shown                   (CodeSlayerPreferences   *preferences, 
                                                 const char              *file_name, 
                                                 GFileType                file_type);
static gboolean open_document                   (CodeSlayerProjects      *projects, 
                                                 GtkTreeIter             *path, 
                                                 GtkTreeViewColumn       *column);
static void edited_action                       (CodeSlayerProjects      *projects, 
                                                 gchar                   *path, 
                                                 gchar                   *new_text);
static void editing_canceled_action             (CodeSlayerProjects      *projects);
static GFile *create_destination                (GFile                   *source, 
                                                 const gchar             *file_path);
static gchar *get_file_path_from_iter           (GtkTreeModel            *model, 
                                                 GtkTreeIter             *child, 
                                                 CodeSlayerProject       *project);
static gboolean show_popup_menu                 (CodeSlayerProjects      *projects, 
                                                 GdkEventButton          *event);
static void create_project_properties_dialog    (CodeSlayerProjects      *projects);
static void add_to_project_properties           (CodeSlayerProjects      *projects);
static void remove_project_action               (CodeSlayerProjects      *projects);
static void new_folder_action                   (CodeSlayerProjects      *projects);
static void new_file_action                     (CodeSlayerProjects      *projects);
static void cut_action                          (CodeSlayerProjects      *projects);
static void copy_action                         (CodeSlayerProjects      *projects);
static void paste_action                        (CodeSlayerProjects      *projects);
static void rename_action                       (CodeSlayerProjects      *projects);
static void move_to_trash_action                (CodeSlayerProjects      *projects);
static void refresh_action                      (CodeSlayerProjects      *projects);
static void project_properties_action           (CodeSlayerProjects      *projects);
static void tools_action                        (GtkMenuItem             *menuitem, 
                                                 CodeSlayerProjects      *projects);
static void activate_tools_item                 (CodeSlayerProjects      *projects,
                                                 GtkWidget               *menuitem);
static GList* get_showable_popup_items          (CodeSlayerProjects      *projects);
static gboolean is_popup_item_showable          (CodeSlayerProjects      *projects, 
                                                 GtkWidget               *popup_menu_item);
static GList* get_selections                    (CodeSlayerProjects      *projects);
static void reorder_plugins                     (CodeSlayerProjects      *projects);

#define CODESLAYER_PROJECTS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROJECTS_TYPE, CodeSlayerProjectsPrivate))

typedef struct _CodeSlayerProjectsPrivate CodeSlayerProjectsPrivate;

typedef enum
{
  FILE_CUT,
  FILE_COPY
} FileAction;

static void cut_or_copy_file_folder (CodeSlayerProjects *projects,
                                     FileAction          file_action);

typedef struct
{
  GList      *sources;
  GList      *tree_row_references;
  FileAction  file_action;

} CutCopyPaste;

struct _CodeSlayerProjectsPrivate
{
  GtkWidget             *window;
  CodeSlayerPreferences *preferences;
  CodeSlayerGroups      *groups;
  GtkWidget             *project_properties;
  GtkWidget             *properties_dialog;
  GtkWidget             *name_entry;
  GtkWidget             *folder_entry;
  GtkBindingSet         *binding_set;
  GtkWidget             *scrolled_window;
  GtkWidget             *treeview;
  GtkTreeStore          *treestore;
  GtkTreeSortable       *sortable;
  GdkPixbuf             *project_pixbuf;
  GdkPixbuf             *folder_pixbuf;
  GdkPixbuf             *text_pixbuf;
  GtkCellRenderer       *cell_text;
  CutCopyPaste          *ccp;
  
  GList                 *plugins;

  /* popup menu items */
  GtkWidget             *menu;
  GtkWidget             *remove_project_item;
  GtkWidget             *project_separator;
  GtkWidget             *new_folder_item;
  GtkWidget             *new_file_item;
  GtkWidget             *new_separator;
  GtkWidget             *cut_item;
  GtkWidget             *copy_item;
  GtkWidget             *paste_item;
  GtkWidget             *ccp_separator;
  GtkWidget             *rename_item;
  GtkWidget             *move_to_trash_item;
  GtkWidget             *refresh_item;
  GtkWidget             *properties_separator;
  GtkWidget             *properties_item;
  GtkWidget             *find_item;
  GtkWidget             *plugins_separator;
};

enum
{
  IMAGE = 0,
  FILE_TYPE,
  FILE_NAME,
  PROJECT,
  COLUMNS
};

enum
{
  REMOVE_PROJECT,
  PROJECT_MODIFIED,
  OPEN_DOCUMENT,
  FILE_PATH_RENAMED,
  RENAME_FILE_FOLDER,
  DELETE_FILE_FOLDER,
  SEARCH_FIND,
  FIND_PROJECTS,
  CUT_FILE_FOLDER,
  COPY_FILE_FOLDER,
  PASTE_FILE_FOLDER,
  PROPERTIES_OPENED,
  PROPERTIES_SAVED,
  LAST_SIGNAL
};

static guint codeslayer_tree_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerProjects, codeslayer_projects, GTK_TYPE_VBOX)

static void
codeslayer_projects_class_init (CodeSlayerProjectsClass *klass)
{
  GtkBindingSet *binding_set;

  klass->cut_file_folder = cut_action;
  klass->copy_file_folder = copy_action;
  klass->paste_file_folder = paste_action;
  klass->rename_file_folder = rename_action;
  klass->delete_file_folder = move_to_trash_action;
  klass->search_find = codeslayer_projects_search_find;

  /**
   * CodeSlayerProjects::remove-project
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::remove-project signal is a request for the project to be 
   * removed from the group.
   */
  codeslayer_tree_signals[REMOVE_PROJECT] =
    g_signal_new ("remove-project", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, remove_project), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayerProjects::project_modified
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::project_modified signal is invoked when the project was updated.
   */
  codeslayer_tree_signals[PROJECT_MODIFIED] =
    g_signal_new ("project-modified", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, project_modified), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayerProjects::open-document
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::open-document signal is a request to open the document in the notebook.
   */
  codeslayer_tree_signals[OPEN_DOCUMENT] =
    g_signal_new ("open-document", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, open_document), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * CodeSlayerProjects::file-path-renamed
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::file-path-renamed signal is invoked after a file/folder is renamed.
   */
  codeslayer_tree_signals[FILE_PATH_RENAMED] =
    g_signal_new ("file-path-renamed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, file_path_renamed), 
                  NULL, NULL,
                  _codeslayer_marshal_VOID__STRING_STRING, G_TYPE_NONE, 
                  2, G_TYPE_STRING, G_TYPE_STRING);

  /**
   * CodeSlayerProjects::rename-file-folder
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::rename-file-folder signal is a request to rename the file/folder.
   */
  codeslayer_tree_signals[RENAME_FILE_FOLDER] =
    g_signal_new ("rename-file-folder", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, rename_file_folder), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
                  
  /**
   * CodeSlayerProjects::delete-file-folder
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::delete-file-folder signal is a request to rename the file/folder.
   */
  codeslayer_tree_signals[DELETE_FILE_FOLDER] =
    g_signal_new ("delete-file-folder", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, delete_file_folder), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
                  
  /**
   * CodeSlayerMenu::find-projects
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::find-projects signal is a request to open up the search dialog.
   */
  codeslayer_tree_signals[FIND_PROJECTS] =
    g_signal_new ("find-projects", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, find_projects),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
                  
 /**
   * CodeSlayerProjects::search-find
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::search-find signal is a request to open up the search dialog.
   */
  codeslayer_tree_signals[SEARCH_FIND] =
    g_signal_new ("search-find", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, search_find), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

 /**
   * CodeSlayerProjects::cut-file-folder
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::cut-file-folder signal is a request to cut the file/folder from the tree.
   */
  codeslayer_tree_signals[CUT_FILE_FOLDER] =
    g_signal_new ("cut-file-folder", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, cut_file_folder), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerProjects::copy-file-folder
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::copy-file-folder signal is a request to copy the file/folder in the tree.
   */
  codeslayer_tree_signals[COPY_FILE_FOLDER] =
    g_signal_new ("copy-file-folder", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, copy_file_folder), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerProjects::paste-file-folder
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::paste-file-folder signal is a request to paste the file/folder that was 
   * previously cut or copied.
   */
  codeslayer_tree_signals[PASTE_FILE_FOLDER] =
    g_signal_new ("paste-file-folder", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, paste_file_folder), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);                  

  /**
   * CodeSlayerProjects::properties-opened
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::properties-opened signal is a activated when the properties is opened.
   */
  codeslayer_tree_signals[PROPERTIES_OPENED] =
    g_signal_new ("properties-opened", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, properties_opened), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_PROJECT_TYPE);

  /**
   * CodeSlayerProjects::properties_saved
   * @codeslayerprojectstree: the tree that received the signal
   *
   * Note: for internal use only.
   *
   * The ::properties-saved signal is a activated when the properties is closed.
   */
  codeslayer_tree_signals[PROPERTIES_SAVED] =
    g_signal_new ("properties-saved", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProjectsClass, properties_saved), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CODESLAYER_PROJECT_TYPE);
                  
  G_OBJECT_CLASS (klass)->finalize =
    (GObjectFinalizeFunc) codeslayer_projects_finalize;

  binding_set = gtk_binding_set_by_class (klass);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_X, GDK_CONTROL_MASK,
                                "cut-file-folder", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_C, GDK_CONTROL_MASK,
                                "copy-file-folder", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_V, GDK_CONTROL_MASK,
                                "paste-file-folder", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_R, GDK_CONTROL_MASK,
                                "rename-file-folder", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_F, GDK_CONTROL_MASK,
                                "search-find", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Delete, 0,
                                "delete-file-folder", 0);

  g_type_class_add_private (klass, sizeof (CodeSlayerProjectsPrivate));
}

static void
codeslayer_projects_init (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeSelection *tree_selection;
  GtkTreeSortable *sortable;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell_pixbuf;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  priv->ccp = g_malloc (sizeof (CutCopyPaste));
  priv->ccp->sources = NULL;
  priv->ccp->tree_row_references = NULL;

  /* create the tree and tree store */
  
  priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  
  priv->treeview = gtk_tree_view_new ();

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  gtk_tree_selection_set_mode (tree_selection, GTK_SELECTION_MULTIPLE);

  priv->treestore = gtk_tree_store_new (COLUMNS, 
                                        G_TYPE_OBJECT, 
                                        G_TYPE_STRING, 
                                        G_TYPE_STRING, 
                                        G_TYPE_POINTER);

  priv->project_pixbuf = gtk_widget_render_icon_pixbuf (GTK_WIDGET (projects), 
                                                        GTK_STOCK_HARDDISK, 
                                                        GTK_ICON_SIZE_BUTTON);
  priv->folder_pixbuf = gtk_widget_render_icon_pixbuf (GTK_WIDGET (projects), 
                                                       GTK_STOCK_DIRECTORY,
                                                       GTK_ICON_SIZE_BUTTON);
  priv->text_pixbuf = gtk_widget_render_icon_pixbuf (GTK_WIDGET (projects), 
                                                     GTK_STOCK_FILE,
                                                     GTK_ICON_SIZE_BUTTON);

  sortable = GTK_TREE_SORTABLE (priv->treestore);
  gtk_tree_sortable_set_sort_func (sortable, FILE_NAME, sort_iter_compare_func, 
                                   GINT_TO_POINTER (FILE_NAME), NULL);
  gtk_tree_sortable_set_sort_column_id (sortable, FILE_NAME, GTK_SORT_ASCENDING);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->treeview), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview),
                           GTK_TREE_MODEL (priv->treestore));

  column = gtk_tree_view_column_new ();

  cell_pixbuf = gtk_cell_renderer_pixbuf_new ();
  priv->cell_text = gtk_cell_renderer_text_new ();

  gtk_tree_view_column_pack_start (column, cell_pixbuf, FALSE);
  gtk_tree_view_column_set_attributes (column, cell_pixbuf, "pixbuf", IMAGE, NULL);
  gtk_tree_view_column_pack_start (column, priv->cell_text, FALSE);
  gtk_tree_view_column_set_attributes (column, priv->cell_text, "text",
                                       FILE_NAME, NULL);
                                       
  g_signal_connect_swapped (G_OBJECT (priv->treeview), "test-expand-row",
                            G_CALLBACK (treeview_row_expanded), projects);
  
  g_signal_connect_swapped (G_OBJECT (priv->treeview), "button_press_event",
                            G_CALLBACK (show_popup_menu), projects);
  
  g_signal_connect_swapped (G_OBJECT (priv->treeview), "row_activated",
                            G_CALLBACK (open_document), projects);
  
  g_signal_connect_swapped (G_OBJECT (priv->cell_text), "edited",
                            G_CALLBACK (edited_action), projects);
  
  g_signal_connect_swapped (G_OBJECT (priv->cell_text), "editing-canceled",
                            G_CALLBACK (editing_canceled_action),
                            projects);

  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), column);
  
  /* create the popup menu */

  priv->menu = gtk_menu_new ();

  priv->remove_project_item = gtk_menu_item_new_with_label (_("Remove Project"));
  g_signal_connect_swapped (G_OBJECT (priv->remove_project_item), "activate",
                            G_CALLBACK (remove_project_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->remove_project_item);

  priv->project_separator = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->project_separator);

  priv->new_folder_item = gtk_image_menu_item_new_from_stock 
    (GTK_STOCK_ADD, NULL);
  gtk_menu_item_set_label (GTK_MENU_ITEM (priv->new_folder_item), _("Create Folder"));
  g_signal_connect_swapped (G_OBJECT (priv->new_folder_item), "activate",
                            G_CALLBACK (new_folder_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->new_folder_item);

  priv->new_file_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW, NULL);
  gtk_menu_item_set_label (GTK_MENU_ITEM (priv->new_file_item), _("Create File"));
  g_signal_connect_swapped (G_OBJECT (priv->new_file_item), "activate",
                            G_CALLBACK (new_file_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->new_file_item);
  
  priv->refresh_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_REFRESH, NULL);
  g_signal_connect_swapped (G_OBJECT (priv->refresh_item), "activate",
                            G_CALLBACK (refresh_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->refresh_item);

  priv->new_separator = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->new_separator);
  
  priv->find_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_FIND, NULL);
  g_signal_connect_swapped (G_OBJECT (priv->find_item), "activate",
                            G_CALLBACK (codeslayer_projects_search_find), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->find_item);

  priv->cut_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CUT, NULL);
  g_signal_connect_swapped (G_OBJECT (priv->cut_item), "activate",
                            G_CALLBACK (cut_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->cut_item);

  priv->copy_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_COPY, NULL);
  g_signal_connect_swapped (G_OBJECT (priv->copy_item), "activate",
                            G_CALLBACK (copy_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->copy_item);

  priv->paste_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PASTE, NULL);
  g_signal_connect_swapped (G_OBJECT (priv->paste_item), "activate",
                            G_CALLBACK (paste_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->paste_item);

  priv->ccp_separator = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->ccp_separator);

  priv->rename_item = gtk_menu_item_new_with_label (_("Rename"));
  g_signal_connect_swapped (G_OBJECT (priv->rename_item), "activate",
                            G_CALLBACK (rename_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->rename_item);

  priv->move_to_trash_item = gtk_menu_item_new_with_label (_("Move To Trash"));
  g_signal_connect_swapped (G_OBJECT (priv->move_to_trash_item), "activate",
                            G_CALLBACK (move_to_trash_action),
                            projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->move_to_trash_item);

  priv->plugins_separator = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->plugins_separator);

  priv->properties_separator = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->properties_separator);

  priv->properties_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PROPERTIES, 
                                                              NULL);
  g_signal_connect_swapped (G_OBJECT (priv->properties_item), "activate",
                            G_CALLBACK (project_properties_action), projects);
  gtk_menu_shell_append (GTK_MENU_SHELL (priv->menu), priv->properties_item);
}

static void
codeslayer_projects_finalize (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  if (priv->ccp->tree_row_references)
    g_list_free (priv->ccp->tree_row_references);
  if (priv->ccp->sources)
    {
      g_list_foreach (priv->ccp->sources, (GFunc) g_object_unref, NULL);
      g_list_free (priv->ccp->sources);
    }
  g_free (priv->ccp);
  g_object_unref (priv->project_pixbuf);
  g_object_unref (priv->folder_pixbuf);
  g_object_unref (priv->text_pixbuf);
  g_list_free (priv->plugins);
  
  G_OBJECT_CLASS (codeslayer_projects_parent_class)->finalize (G_OBJECT (projects));
}

/**
 * codeslayer_projects_new:
 * @window: a #GtkWindow.
 * @preferences: a #CodeSlayerPreferences.
 * @groups: a #CodeSlayerGroups.
 * @project_properties: a #CodeSlayerProjectProperties.
 *
 * Creates a new #CodeSlayerProjects.
 *
 * Returns: a new #CodeSlayerProjects. 
 */
GtkWidget*
codeslayer_projects_new (GtkWidget             *window, 
                         CodeSlayerPreferences *preferences, 
                         CodeSlayerGroups      *groups, 
                         GtkWidget             *project_properties)
{
  CodeSlayerProjectsPrivate *priv;
  GtkWidget *projects;  
  
  projects = g_object_new (codeslayer_projects_get_type (), NULL);
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  
  priv->window = window;
  priv->groups = groups;
  priv->preferences = preferences;
  priv->plugins = NULL;
  priv->project_properties = project_properties;
  
  create_project_properties_dialog (CODESLAYER_PROJECTS (projects));
  add_to_project_properties (CODESLAYER_PROJECTS (projects));
  
  gtk_container_add (GTK_CONTAINER (priv->scrolled_window), priv->treeview); 
  gtk_container_add (GTK_CONTAINER (projects), priv->scrolled_window);
  
  return projects;
}

static gint
sort_iter_compare_func (GtkTreeModel *model, 
                        GtkTreeIter  *a,
                        GtkTreeIter  *b, 
                        gpointer      userdata)
{
  gint ret = 0;
  gint sortcol;

  sortcol = GPOINTER_TO_INT (userdata);

  switch (sortcol)
    {
    case FILE_NAME:
      {
        gchar *filetype1, *filetype2;
        gchar *filename1, *filename2;
        gint filetype_compare;

        gtk_tree_model_get (model, a, 
                            FILE_TYPE, &filetype1, 
                            FILE_NAME, &filename1, -1);
        gtk_tree_model_get (model, b, 
                            FILE_TYPE, &filetype2, 
                            FILE_NAME, &filename2, -1);

        filetype_compare = g_strcmp0 (filetype1, filetype2);
        if (filetype_compare == 0)
          ret = g_strcmp0 (filename1, filename2);
        else
          ret = filetype_compare;

        g_free (filetype1);
        g_free (filetype2);
        g_free (filename1);
        g_free (filename2);
      }
      break;
    }

  return ret;
}

/**
 * codeslayer_projects_load_group:
 * @projects: a #CodeSlayerProjects.
 * @group: a #CodeSlayerGroup to load into the tree.
 *
 * Load the group projects into the tree.
 */
void
codeslayer_projects_load_group (CodeSlayerProjects *projects,
                                CodeSlayerGroup    *group)
{
  CodeSlayerProjectsPrivate *priv;
  GList *list;  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  gtk_tree_store_clear (priv->treestore);
  list = codeslayer_group_get_projects (group);
  g_list_foreach (list, (GFunc) add_project, projects);
}

static void
add_project (CodeSlayerProject      *project, 
             CodeSlayerProjects *projects)
{
  codeslayer_projects_add_project (projects, project);
}

/**
 * codeslayer_projects_select_document:
 * @projects: a #CodeSlayerProjects.
 * @document: a #CodeSlayerDocument.
 *
 * Returns: is TRUE if the file can be selected in the tree.
 */
gboolean
codeslayer_projects_select_document (CodeSlayerProjects *projects,
                                     CodeSlayerDocument *document)
{
  CodeSlayerProjectsPrivate *priv;
  CodeSlayerGroup *group;
  CodeSlayerProject *project;
  const gchar *project_folder_path;
  const gchar *document_file_path;
  gchar **project_path_dirs;
  gchar **document_path_dirs;
  gint start;
  gint end;
  GtkTreeIter iter;
  GtkTreeIter parent;
  gchar *current_path;
  gchar *destination_path;

  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  if (select_document (document, projects))
    return TRUE;
    
  project = codeslayer_document_get_project (document);
  
  group = codeslayer_groups_get_active_group (priv->groups);
  if (project == NULL || !codeslayer_group_contains_project (group, project))
    {
      g_warning ("Cannot select document from the tree because the project is invalid.");  
      return FALSE;
    }
  
  project_folder_path = codeslayer_project_get_folder_path (project);
  document_file_path = codeslayer_document_get_file_path (document);
  
  if (!codeslayer_utils_file_exists (project_folder_path))
    return FALSE;

  if (!codeslayer_utils_file_exists (document_file_path))
    return FALSE;

  project_path_dirs = g_strsplit (project_folder_path, G_DIR_SEPARATOR_S, -1);
  document_path_dirs = g_strsplit (document_file_path, G_DIR_SEPARATOR_S, -1);

  start = codeslayer_utils_array_length (project_path_dirs);
  end = codeslayer_utils_array_length (document_path_dirs);

  /* the file path starts out as the first projects path */
  gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->treestore), &iter);
  gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                                      PROJECT, &project, -1);
  current_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                          &iter, project);
  
  /* the destination path where we are going  */
  destination_path = g_strdup (project_folder_path);

  for (; start <= end; start++)
    {
      GtkTreePath *tree_path;

      /* find folder/file at the current level */
      while (g_strcmp0 (current_path, destination_path) != 0)
        {
          if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->treestore), &iter))
            {
              g_free (current_path);
              g_strfreev (project_path_dirs);
              g_strfreev (document_path_dirs);
              g_free (destination_path);
              return FALSE;
            }
          
          g_free (current_path);
          gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                                              PROJECT, &project, -1);
          current_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                                  &iter, project);
          if (current_path == NULL)
            {
              g_strfreev (project_path_dirs);
              g_strfreev (document_path_dirs);
              g_free (destination_path);
              return FALSE;
            }
        }

      tree_path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->treestore), &iter);
      gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview), tree_path, FALSE);

      parent = iter;
      if (gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->treestore), 
                                        &iter, &parent))
        {
          /* going down a node level */
          g_free (destination_path);
          destination_path = g_strconcat (current_path, G_DIR_SEPARATOR_S, 
                                          document_path_dirs[start], NULL);
          g_free (current_path);
          current_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                                  &iter, project);
        }
      else
        {
          /* we found the document and can now select it */
          GtkTreeRowReference *tree_row_reference;
          tree_row_reference = gtk_tree_row_reference_new (GTK_TREE_MODEL (priv->treestore), 
                                                           tree_path);
          codeslayer_document_set_tree_row_reference (document, tree_row_reference);
          g_signal_emit_by_name ((gpointer) projects, "open-document", document);
        }

      gtk_tree_path_free (tree_path);
    }

  g_strfreev (project_path_dirs);
  g_strfreev (document_path_dirs);
  g_free (destination_path);
  g_free (current_path);
  return TRUE;
}

/**
 * codeslayer_projects_add_popup_item:
 * @projects: a #CodeSlayerProjects.
 * @item: a #GtkWidget.
 */
void
codeslayer_projects_add_popup_item (CodeSlayerProjects *projects,
                                    GtkWidget          *menuitem)
{
  CodeSlayerProjectsPrivate *priv;
  GList *children;
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  children = gtk_container_get_children (GTK_CONTAINER (priv->menu));
  gtk_menu_shell_insert (GTK_MENU_SHELL (priv->menu), menuitem, g_list_length (children) - 2);  
  priv->plugins = g_list_append (priv->plugins, menuitem);
  reorder_plugins (projects);
  activate_tools_item (projects, menuitem);
  g_list_free (children);
  gtk_widget_show_all (menuitem);                    
}

static void
activate_tools_item (CodeSlayerProjects  *projects,
                     GtkWidget           *menuitem)
{
  GtkWidget *submenu;

  if (IS_CODESLAYER_MENU_ITEM(menuitem))
    {
      g_signal_connect (G_OBJECT (menuitem), "activate",
                        G_CALLBACK (tools_action), projects);
    }

  submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menuitem));
  if (submenu)
    {
      GList *list;
      GList *tmp;
      list = gtk_container_get_children (GTK_CONTAINER (submenu));
      tmp = list;
      
      while (tmp != NULL)
        {
          GtkWidget *widget = tmp->data;
          activate_tools_item (projects, widget);
          tmp = g_list_next (tmp);
        }
      
      g_list_free (list);
    }
}

/**
 * codeslayer_projects_remove_popup_item:
 * @projects: a #CodeSlayerProjects.
 * @item: a #GtkWidget.
 */
void
codeslayer_projects_remove_popup_item (CodeSlayerProjects *projects,
                                       GtkWidget          *menuitem)
{
  CodeSlayerProjectsPrivate *priv;
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  gtk_container_remove (GTK_CONTAINER (priv->menu), menuitem);
  priv->plugins = g_list_remove (priv->plugins, menuitem);
  reorder_plugins (projects);
}

static gint
compare_plugins (GtkMenuItem *item1, 
                 GtkMenuItem *item2)
{
  return g_strcmp0 (gtk_menu_item_get_label (item1),
                    gtk_menu_item_get_label (item2));
}

static void
reorder_plugins (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GList *children;
  GList *tmp;
  int pos = 0;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  
  priv->plugins = g_list_sort (priv->plugins, (GCompareFunc) compare_plugins);
  tmp = priv->plugins;

  children = gtk_container_get_children (GTK_CONTAINER (priv->menu));
  pos = g_list_length (children) - 2 - g_list_length (tmp);

  while (tmp != NULL)
    {
      GtkWidget *item = tmp->data;
      gtk_menu_reorder_child (GTK_MENU (priv->menu), item, pos);
      pos++;
      tmp = g_list_next (tmp);
    }
    
  g_list_free (children);
}

static void
tools_action (GtkMenuItem        *menuitem, 
              CodeSlayerProjects *projects)
{
  GList *selections;
  selections = get_selections (projects);
  
  g_signal_emit_by_name ((gpointer) menuitem, "projects-menu-selected", selections);
  
  if (selections != NULL)
    {
      g_list_foreach (selections, (GFunc) g_object_unref, NULL);
      g_list_free (selections);
    }
}

static GList*
get_selections (CodeSlayerProjects  *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  GList *selections = NULL;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, 
                                                        &tree_model);                                                        
  tmp = selected_rows;

  while (tmp != NULL)
    {
      GtkTreeIter iter;
      CodeSlayerProject *project;
      CodeSlayerProjectsSelection *selection;
      gchar *file_path;

      GtkTreePath *tree_path = tmp->data;

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                          PROJECT, &project, -1);

      file_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                           &iter, project);
      selection = codeslayer_projects_selection_new ();
      codeslayer_projects_selection_set_project (selection, project);
      codeslayer_projects_selection_set_file_path (selection, file_path);
      selections = g_list_prepend (selections, selection);
      
      g_free (file_path);
      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
    
  selections = g_list_reverse (selections);
  
  return selections;
}

static gboolean
select_document (CodeSlayerDocument *document, 
                 CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeRowReference *tree_row_reference;
  GtkTreePath *tree_path;
  GtkTreeViewColumn *column;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  tree_row_reference = codeslayer_document_get_tree_row_reference (document);
  if (tree_row_reference == NULL)
    return FALSE;

  tree_path = gtk_tree_row_reference_get_path (tree_row_reference);
  if (tree_path == NULL)
    return FALSE;

  gtk_tree_view_expand_to_path (GTK_TREE_VIEW (priv->treeview), tree_path);

  column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview), 0);
  gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->treeview), tree_path, column, FALSE);

  gtk_tree_path_free (tree_path);

  return TRUE;
}

/**
 * codeslayer_projects_add_project:
 * @projects: a #CodeSlayerProjects.
 * @project: a #CodeSlayerProject to add to the tree.
 *
 * Add the project to the tree.
 */
void
codeslayer_projects_add_project (CodeSlayerProjects *projects,
                                 CodeSlayerProject  *project)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeIter iter;
  const gchar *project_name;
  const gchar *project_folder_path;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  project_name = codeslayer_project_get_name (project);
  project_folder_path = codeslayer_project_get_folder_path (project);

  if (!codeslayer_utils_file_exists (project_folder_path))
    return;

  gtk_tree_store_append (priv->treestore, &iter, NULL);

  gtk_tree_store_set (priv->treestore, &iter,
                      IMAGE, priv->project_pixbuf,
                      FILE_TYPE, NULL,
                      FILE_NAME, project_name, 
                      PROJECT, project, -1);

  append_treestore_children (projects, project, iter, project_folder_path);
}

static void
remove_project_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkWidget *dialog;
  gint response;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  dialog = gtk_message_dialog_new (GTK_WINDOW (priv->window), 
                                   GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_WARNING,
                                   GTK_BUTTONS_OK_CANCEL,
                                   _("Are you sure you want to remove the project?"));
  gtk_window_set_title (GTK_WINDOW (dialog), _("Remove Project"));
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_CANCEL)
    {
      gtk_widget_destroy (dialog);
      return;
    }
  gtk_widget_destroy (dialog);

  /* confirmed that will remove the project */

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      GtkTreePath *tree_path = tmp->data;
      GtkTreeIter iter;
      gchar *filename;
      CodeSlayerProject *project;

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                          FILE_NAME, &filename, 
                          PROJECT, &project, -1);
      gtk_tree_store_remove (priv->treestore, &iter);

      g_signal_emit_by_name ((gpointer) projects, "remove-project", project);

      g_free (filename);

      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
}

static void
create_project_properties_dialog (CodeSlayerProjects *projects) 
{
  CodeSlayerProjectsPrivate *priv;
  GtkWidget *properties_dialog;
  GtkWidget *content_area;

  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  properties_dialog = gtk_dialog_new_with_buttons (_("Project Properties"), 
                                                  GTK_WINDOW (priv->window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                  NULL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (properties_dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (properties_dialog), TRUE);
                                        
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (properties_dialog));
  gtk_dialog_set_default_response (GTK_DIALOG (properties_dialog), GTK_RESPONSE_OK);
  priv->properties_dialog = properties_dialog;
  
  gtk_container_add (GTK_CONTAINER (content_area), priv->project_properties);
}

static void
add_to_project_properties (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkWidget *vbox;
  GtkWidget *grid;
  GtkWidget *name_label;
  GtkWidget *name_entry;
  GtkWidget *folder_label;
  GtkWidget *folder_entry;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 2);

  name_label = gtk_label_new (_("Name:"));
  gtk_misc_set_alignment (GTK_MISC (name_label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (name_label), 4, 0);
  gtk_grid_attach (GTK_GRID (grid), name_label, 0, 0, 1, 1);

  name_entry = gtk_entry_new ();
  priv->name_entry = name_entry;
  gtk_entry_set_activates_default (GTK_ENTRY (name_entry), TRUE);
  gtk_entry_set_width_chars (GTK_ENTRY (name_entry), 50);  
  gtk_grid_attach_next_to (GTK_GRID (grid), name_entry, name_label, 
                           GTK_POS_RIGHT, 1, 1);

  folder_label = gtk_label_new (_("Folder:"));
  gtk_label_set_width_chars (GTK_LABEL (folder_label), 10);
  gtk_misc_set_alignment (GTK_MISC (folder_label), 1, .50);
  gtk_misc_set_padding (GTK_MISC (folder_label), 4, 0);
  gtk_grid_attach (GTK_GRID (grid), folder_label, 0, 1, 1, 1);

  folder_entry = gtk_entry_new ();
  priv->folder_entry = folder_entry;
  gtk_entry_set_width_chars (GTK_ENTRY (folder_entry), 50);
  gtk_widget_set_sensitive (folder_entry, FALSE);
  gtk_grid_attach_next_to (GTK_GRID (grid), folder_entry, folder_label, 
                           GTK_POS_RIGHT, 1, 1);
                           
  gtk_box_pack_start (GTK_BOX (vbox), grid, FALSE, FALSE, 3);                           
                                        
  codeslayer_project_properties_add (CODESLAYER_PROJECT_PROPERTIES (priv->project_properties), 
                                      vbox, _("Project"));
}

static void
project_properties_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  while (tmp != NULL)
    {
      CodeSlayerProject *project;
      GtkTreeIter iter;
      gint response;
      
      GtkTreePath *tree_path = tmp->data;

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (tree_model, &iter, PROJECT, &project, -1);

      gtk_entry_set_text (GTK_ENTRY (priv->name_entry),
                          codeslayer_project_get_name (project));
                          
      gtk_entry_set_text (GTK_ENTRY (priv->folder_entry),
                          codeslayer_project_get_folder_path (project));

      g_signal_emit_by_name((gpointer)projects, "properties-opened", project);

      gtk_widget_show_all (priv->properties_dialog);

      response = gtk_dialog_run (GTK_DIALOG (priv->properties_dialog));
      if (response == GTK_RESPONSE_OK)
        {
          const gchar *name_text = gtk_entry_get_text (GTK_ENTRY (priv->name_entry));
          if (g_strcmp0 (name_text, codeslayer_project_get_name (project)) != 0)
            {
              gchar *name_strip = g_strdup (name_text);
              name_strip = g_strstrip (name_strip);

              codeslayer_project_set_name (project, name_strip);

              g_signal_emit_by_name ((gpointer) projects, "project-modified", project);

              gtk_tree_store_set (GTK_TREE_STORE (tree_model), &iter, FILE_NAME, name_strip, -1);

              g_free (name_strip);
            }
            
          g_signal_emit_by_name((gpointer)projects, "properties-saved", project);
        }

      gtk_widget_hide (priv->properties_dialog);

      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
}

static void
new_folder_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;  
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      CodeSlayerProject *project;
      gchar *file_path;
      gchar *full_path;
      GFile *file;
      
      GtkTreePath *tree_path = tmp->data;

      GtkTreeIter iter;
      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (tree_model, &iter, PROJECT, &project, -1);

      file_path =  get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                                             &iter, project);
      full_path = g_strconcat (file_path, G_DIR_SEPARATOR_S, _("untitled folder"), NULL);
      file = g_file_new_for_path (full_path);

      if (g_file_make_directory (file, NULL, NULL))
        {
          GtkTreeIter child;
          char *file_name;
          GtkTreePath *parent_path;
          GtkTreeViewColumn *column;
          GtkTreePath *child_path;
          
          gtk_tree_store_append (priv->treestore, &child, &iter);

          file_name = g_file_get_basename (file);

          gtk_tree_store_set (priv->treestore, &child,
                              IMAGE, priv->folder_pixbuf,
                              FILE_TYPE, "G_FILE_TYPE_DIRECTORY",
                              FILE_NAME, file_name, PROJECT, project, -1);

          parent_path = gtk_tree_model_get_path (tree_model, &iter);
          gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview), parent_path, FALSE);

          g_object_set (G_OBJECT (priv->cell_text), "editable", TRUE, NULL);

          column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview), 0);
          child_path = gtk_tree_model_get_path (tree_model, &child);
          gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->treeview), child_path, 
                                                  column, TRUE);

          g_free (file_name);
          gtk_tree_path_free (parent_path);
          gtk_tree_path_free (child_path);
        }

      g_free (file_path);
      g_free (full_path);
      g_object_unref (file);

      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
}

static void
new_file_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      CodeSlayerProject *project;
      gchar *file_path;
      gchar *full_path;
      GFile *file;
      GFileIOStream *stream;
      GError *error = NULL;
      
      GtkTreePath *tree_path = tmp->data;

      GtkTreeIter iter;
      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (tree_model, &iter, PROJECT, &project, -1);

      file_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                           &iter, project);

      full_path = g_strconcat (file_path, G_DIR_SEPARATOR_S, _("new file"), NULL);
      g_free (file_path);

      file = g_file_new_for_path (full_path);

      stream = g_file_create_readwrite (file, G_FILE_CREATE_NONE, NULL, &error);
      if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_EXISTS))
        g_error_free (error);
      else
        {
          if (g_io_stream_close (G_IO_STREAM (stream), NULL, NULL))
            {
              gchar *file_name;
              GtkTreeIter child;
              GtkTreePath *parent_path;
              GtkTreeViewColumn *column;
              GtkTreePath *child_path;
              
              gtk_tree_store_append (priv->treestore, &child, &iter);
              file_name = g_file_get_basename (file);
              gtk_tree_store_set (priv->treestore, &child,
                                  IMAGE, priv->text_pixbuf,
                                  FILE_TYPE, "G_FILE_TYPE_REGULAR",
                                  FILE_NAME, file_name, PROJECT, project, -1);

              parent_path = gtk_tree_model_get_path (tree_model, &iter);
              gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview), 
                                        parent_path, FALSE);
              g_object_set (G_OBJECT (priv->cell_text), "editable", TRUE, NULL);
              column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview), 0);
              child_path = gtk_tree_model_get_path (tree_model, &child);
              gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->treeview), child_path,
                                        column, TRUE);

              g_free (file_name);
              gtk_tree_path_free (parent_path);
              gtk_tree_path_free (child_path);
            }
        }

      g_free (full_path);
      g_object_unref (file);
      if (stream)
        g_object_unref (stream);

      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
}

void
codeslayer_projects_search_find (CodeSlayerProjects *projects)
{

  CodeSlayerProjectsPrivate *priv;
  GString *file_paths;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  gchar *results;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  
  if (!is_popup_item_showable (projects, priv->find_item))
    return;

  file_paths = g_string_new (NULL);
  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;

  while (tmp != NULL)
    {
      GtkTreeIter iter;
      CodeSlayerProject *project;
      gchar *file_path;

      GtkTreePath *tree_path = tmp->data;

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                          PROJECT, &project, -1);

      file_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                           &iter, project);
      
      if (file_paths->len > 0)
        file_paths = g_string_append (file_paths, ";"); 
        
      file_paths = g_string_append (file_paths, file_path);
                                           
      g_free (file_path);
      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);

  results = g_string_free (file_paths, FALSE);
  g_signal_emit_by_name ((gpointer) projects, "find-projects", results);
  g_free (results);
}

static void
cut_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  if (is_popup_item_showable (projects, priv->cut_item))
    cut_or_copy_file_folder (projects, FILE_CUT);
}

static void
copy_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  if (is_popup_item_showable (projects, priv->copy_item))
    cut_or_copy_file_folder (projects, FILE_COPY);
}

static void
cut_or_copy_file_folder (CodeSlayerProjects *projects,
                         FileAction          file_action)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  
   priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;

  priv->ccp->file_action = file_action;

  if (tmp != NULL)
    {
      if (priv->ccp->sources != NULL)
        {
          g_list_foreach (priv->ccp->sources, (GFunc) g_object_unref, NULL);
          g_list_free (priv->ccp->sources);
          priv->ccp->sources = NULL;
        }
      g_list_free (priv->ccp->tree_row_references);
      priv->ccp->tree_row_references = NULL;
    }

  while (tmp != NULL)
    {
      GtkTreeIter iter;
      CodeSlayerProject *project;
      gchar *file_path;
      GFile *file;

      GtkTreePath *tree_path = tmp->data;
      GtkTreeRowReference *tree_row_reference =
        gtk_tree_row_reference_new (tree_model, tree_path);

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                          PROJECT, &project, -1);

      file_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                           &iter, project);
      file = g_file_new_for_path (file_path);

      if (!codeslayer_utils_file_has_parent (priv->ccp->sources, file))
        {
          priv->ccp->sources = g_list_append (priv->ccp->sources, file);
          priv->ccp->tree_row_references = g_list_append (priv->ccp->tree_row_references, 
                                                          tree_row_reference);
        }
      else
        {
          g_object_unref (file);
        }

      g_free (file_path);
      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
}

static void
paste_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  
  if (!is_popup_item_showable (projects, priv->paste_item))
    return;

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      CodeSlayerProject *project;
      GtkTreeIter iter;
      GList *sources;

      GtkTreePath *tree_path = tmp->data;

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                          PROJECT, &project, -1);

      sources = priv->ccp->sources;
      
      while (sources != NULL)
        {
          GError *error = NULL;
          GFile *destination;

          GFile *source = sources->data;
          gchar *file_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                                      &iter, project);

          destination = create_destination (source, file_path);

          if (destination == NULL)
            {
              g_free (file_path);
              sources = g_list_next (sources);
              continue;
            }

          switch (priv->ccp->file_action)
            {
            case FILE_CUT:
              g_file_move (source, destination, G_FILE_COPY_NONE, 
                                  NULL, NULL, NULL, &error);
              if (error == NULL)
                {
                  gint index = g_list_index (priv->ccp->sources, source);
                  GtkTreeRowReference *tree_row_reference = g_list_nth_data (priv->ccp->tree_row_references, 
                                                                             index);
                  GtkTreeIter iter;
                  GtkTreePath *tree_path = gtk_tree_row_reference_get_path (tree_row_reference);
                  gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_model), &iter,
                                           tree_path);
                  gtk_tree_store_remove (GTK_TREE_STORE (tree_model), &iter);
                  gtk_tree_path_free (tree_path);
                }
              break;
            case FILE_COPY:
              codeslayer_utils_file_copy (source, destination, &error);
              break;
            }

          if (error != NULL)
            {
              GtkWidget *dialog;
              dialog =  gtk_message_dialog_new (NULL, 
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                error->message, 
                                                NULL);
              gtk_dialog_run (GTK_DIALOG (dialog));
              gtk_widget_destroy (dialog);
              g_error_free (error);
            }
          else
            {
              char *file_name;
              GFileInfo *file_info;
              GFileType file_type;
              GtkTreeIter child;
              
              file_name = g_file_get_basename (destination);
              file_info = g_file_query_info (destination, "standard::*",
                                             G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 
                                             NULL, NULL);
              file_type = g_file_info_get_file_type (file_info);

              gtk_tree_store_append (priv->treestore, &child, &iter);
              gtk_tree_store_set (priv->treestore, &child,
                                  IMAGE, file_type == G_FILE_TYPE_DIRECTORY ? priv->folder_pixbuf : priv->text_pixbuf, 
                                  FILE_TYPE, file_type == G_FILE_TYPE_DIRECTORY ? "G_FILE_TYPE_DIRECTORY" : "G_FILE_TYPE_REGULAR", 
                                  FILE_NAME, file_name,
                                  PROJECT, project, -1);
              if (file_type == G_FILE_TYPE_DIRECTORY)
                {
                  GtkTreeIter leaf;
                  gtk_tree_store_append (priv->treestore, &leaf, &child);
                  gtk_tree_store_set (priv->treestore, &leaf,
                                      IMAGE, NULL,
                                      FILE_TYPE, NULL,
                                      FILE_NAME, NULL, 
                                      PROJECT, NULL, -1);
                }
              g_free (file_name);
              g_object_unref (file_info);
            }

          g_object_unref (destination);
          g_free (file_path);
          sources = g_list_next (sources);
        }
      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
}

static GFile *
create_destination (GFile       *source, 
                    const gchar *file_path)
{
  char *basename;
  char *full_path;
  GFile *destination;

  basename = g_file_get_basename (source);
  full_path = g_build_filename (file_path, basename, NULL);
  destination = g_file_new_for_path (full_path);

  if (g_file_query_exists (destination, NULL))
    {
      GtkWidget *dialog;
      gchar *title;
      GtkWidget *content_area;
      GtkWidget *hbox;
      GtkWidget *label;
      GtkWidget *entry;
      gchar *basename_copy = NULL;
      gint response;

      g_object_unref (destination);
      destination = NULL;

      title = g_strdup_printf (_("Rename copy of %s"), basename);
      dialog = gtk_dialog_new_with_buttons (title, 
                                            NULL,
                                            GTK_DIALOG_MODAL,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_OK, GTK_RESPONSE_OK,
                                            NULL);
      gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
      gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
                                            
      content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
      /*gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);*/

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);

      label = gtk_label_new (_("New Name:"));
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 4);

      entry = gtk_entry_new ();
      gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
      gtk_entry_set_width_chars (GTK_ENTRY (entry), 50);
      gtk_entry_set_text (GTK_ENTRY (entry), basename);
      gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 4);

      gtk_widget_show_all (hbox);
      gtk_container_add (GTK_CONTAINER (content_area), hbox);

      response = gtk_dialog_run (GTK_DIALOG (dialog));
      if (response == GTK_RESPONSE_OK)
        basename_copy = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

      gtk_widget_destroy (dialog);
      g_free (title);

      if (basename_copy != NULL)
        {
          gchar *full_path_copy = g_build_filename (file_path, basename_copy, 
                                                    NULL);
          destination = g_file_new_for_path (full_path_copy);
          g_free (basename_copy);
          g_free (full_path_copy);
        }
    }

  g_free (basename);
  g_free (full_path);

  return destination;
}

static void
refresh_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      CodeSlayerProject *project;
      GtkTreeIter iter;
      GtkTreeIter child;
      gchar *file_path;

      GtkTreePath *tree_path = tmp->data;

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (tree_model, &iter, PROJECT, &project, -1);

      file_path = get_file_path_from_iter (tree_model, &iter, project);

      if (gtk_tree_model_iter_children (tree_model, &child, &iter))
        {
          while (gtk_tree_store_remove (GTK_TREE_STORE (tree_model), &child)) {}
          append_treestore_children (projects, project, iter, file_path);
        }

      gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview), tree_path, FALSE);

      g_free (file_path);
      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
}

static void
move_to_trash_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkWidget *dialog;
  gint response;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  GList *tree_row_references = NULL;

  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  
  if (!is_popup_item_showable (projects, priv->move_to_trash_item))
    return;

  dialog = gtk_message_dialog_new (NULL, 
                                   GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_WARNING,
                                   GTK_BUTTONS_OK_CANCEL,
                                   _("Are you sure you want to move to the trash?"));
  gtk_window_set_title (GTK_WINDOW (dialog), _("Move To Trash"));
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_CANCEL)
    {
      gtk_widget_destroy (dialog);
      return;
    }
  gtk_widget_destroy (dialog);

  /* confirmed that will delete the file/folder */

  tree_model = GTK_TREE_MODEL (priv->treestore);
  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      CodeSlayerProject *project;
      GtkTreeIter iter;
      gchar *file_path;
      GFile *file;
      
      GtkTreePath *tree_path = tmp->data;

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (tree_model, &iter, PROJECT, &project, -1);

      file_path = get_file_path_from_iter (tree_model, &iter, project);
      file = g_file_new_for_path (file_path);
      if (g_file_trash (file, NULL, NULL))
        {
          GtkTreeRowReference *tree_row_reference;
          tree_row_reference = gtk_tree_row_reference_new (tree_model, 
                                                           tree_path);
          tree_row_references = g_list_append (tree_row_references, 
                                               tree_row_reference);
        }

      g_free (file_path);
      g_object_unref (file);

      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);

  tmp = tree_row_references;

  while (tmp != NULL)
    {
      GtkTreeRowReference *tree_row_reference = tmp->data;
      if (tree_row_reference)
        {
          GtkTreePath *tree_path;
          GtkTreeIter iter;
          tree_path = gtk_tree_row_reference_get_path (tree_row_reference);
          gtk_tree_model_get_iter (tree_model, &iter, tree_path);
          gtk_tree_store_remove (GTK_TREE_STORE (tree_model), &iter);
          tmp = g_list_next (tmp);
          gtk_tree_path_free (tree_path);
        }
    }
  g_list_free (tree_row_references);
}

static void
rename_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows;
  GList *tmp;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  
  if (!is_popup_item_showable (projects, priv->rename_item))
    return;

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      GtkTreeIter iter;
      GtkTreeViewColumn *column;
      GtkTreePath *tree_path = tmp->data;
      gtk_tree_model_get_iter (tree_model, &iter, tree_path);
      g_object_set (G_OBJECT (priv->cell_text), "editable", TRUE, NULL);
      column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview), 0);
      gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->treeview), tree_path, column, TRUE);
      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);
}

static void
edited_action (CodeSlayerProjects *projects, 
               gchar              *path,
               gchar              *new_text)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeIter iter;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (priv->treestore), 
                                           &iter, path))
    {
      CodeSlayerProject *project;
      gchar *file_path;
      GFile *file;
      GFile *renamed_file;
      
      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                          PROJECT, &project, -1);

      file_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                           &iter, project);

      file = g_file_new_for_path (file_path);
      renamed_file = g_file_set_display_name (file, new_text, NULL, NULL);
      if (renamed_file)
        {
          char *renamed_file_path = g_file_get_path (renamed_file);
          gtk_tree_store_set (priv->treestore, &iter, FILE_NAME, new_text, -1);
          g_signal_emit_by_name ((gpointer) projects, "file-path-renamed",
                                 file_path, renamed_file_path);
          g_free (renamed_file_path);
        }

      g_object_set (G_OBJECT (priv->cell_text), "editable", FALSE, NULL);

      g_object_unref (file);
      g_free (file_path);
      if (renamed_file)
        g_object_unref (renamed_file);
    }
}

static void
editing_canceled_action (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);
  g_object_set (G_OBJECT (priv->cell_text), "editable", FALSE, NULL);
}

static void
append_treestore_children (CodeSlayerProjects *projects,
                           CodeSlayerProject  *project, 
                           GtkTreeIter         iter,
                           const gchar        *parentdir)
{
  CodeSlayerProjectsPrivate *priv;
  GFile *file;
  char *file_path;
  GFileEnumerator *enumerator;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  file = g_file_new_for_path (parentdir);
  file_path = g_file_get_path (file);

  enumerator = g_file_enumerate_children (file, "standard::*",
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 
                                          NULL, NULL);
  if (enumerator != NULL)
    {
      GFileInfo *file_info;
      while ((file_info = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
        {
          const char *file_name;
          GFileType file_type;
          GtkTreeIter child;
          
          file_name = g_file_info_get_name (file_info);
          file_type = g_file_info_get_file_type (file_info);

          if (!is_file_shown (priv->preferences, file_name, file_type))
            {
              g_object_unref (file_info);
              continue;
            }

          gtk_tree_store_append (priv->treestore, &child, &iter);
          gtk_tree_store_set (priv->treestore, &child,
                              IMAGE, file_type == G_FILE_TYPE_DIRECTORY ? priv->folder_pixbuf : priv->text_pixbuf, 
                              FILE_TYPE, file_type == G_FILE_TYPE_DIRECTORY ? "G_FILE_TYPE_DIRECTORY" : "G_FILE_TYPE_REGULAR", 
                              FILE_NAME, file_name,
                              PROJECT, project, -1);
          if (file_type == G_FILE_TYPE_DIRECTORY)
            {
              GtkTreeIter leaf;
              gtk_tree_store_append (priv->treestore, &leaf, &child);
              gtk_tree_store_set (priv->treestore, &leaf,
                                  IMAGE, NULL,
                                  FILE_TYPE, NULL,
                                  FILE_NAME, NULL, 
                                  PROJECT, NULL, -1);
            }

          g_object_unref (file_info);
        }
      g_object_unref (enumerator);
    }

  g_object_unref (file);
  g_free (file_path);
}

static gboolean
is_file_shown (CodeSlayerPreferences *preferences, 
               const char            *file_name, 
               GFileType              file_type)
{
  gchar *exclude_types_str;
  GList *exclude_types = NULL;
  gchar *exclude_dirs_str;
  GList *exclude_dirs = NULL;
  gboolean result = TRUE;

  exclude_types_str = codeslayer_preferences_get_string (preferences,
                                                         CODESLAYER_PREFERENCES_PROJECTS_EXCLUDE_TYPES);

  exclude_types = codeslayer_utils_string_to_list (exclude_types_str);
  
  if (file_type == G_FILE_TYPE_REGULAR
      && codeslayer_utils_contains_element_with_suffix (exclude_types, file_name))
    {
      result = FALSE;
    }
  
  exclude_dirs_str = codeslayer_preferences_get_string (preferences,
                                                        CODESLAYER_PREFERENCES_PROJECTS_EXCLUDE_DIRS);

  exclude_dirs = codeslayer_utils_string_to_list (exclude_dirs_str);
  
  if (file_type == G_FILE_TYPE_DIRECTORY
      && codeslayer_utils_contains_element (exclude_dirs, file_name))
    {
      result = FALSE;
    }
  
  g_free (exclude_types_str);
  if (exclude_types)
    {
      g_list_foreach (exclude_types, (GFunc) g_free, NULL);
      g_list_free (exclude_types);
    }

  g_free (exclude_dirs_str);
  if (exclude_dirs)
    {
      g_list_foreach (exclude_dirs, (GFunc) g_free, NULL);
      g_list_free (exclude_dirs);
    }
  
  return result;
}

static gboolean
treeview_row_expanded (CodeSlayerProjects *projects, 
                       GtkTreeIter        *iter,
                       GtkTreePath        *tree_path)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeIter child;
  GValue value = { 0 };
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->treestore), &child, iter);

  gtk_tree_model_get_value (GTK_TREE_MODEL (priv->treestore), &child, 0, &value);
  if (g_value_get_object (&value) == NULL)
    {
      CodeSlayerProject *project;
      gchar *file_path;

      gtk_tree_store_remove (priv->treestore, &child);

      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), iter, 
                          PROJECT, &project, -1);

      file_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                           iter, project);

      append_treestore_children (projects, project, *iter, file_path);

      g_free (file_path);
    }
  g_value_unset (&value);

  return FALSE;
}

static gboolean
open_document (CodeSlayerProjects *projects, 
               GtkTreeIter        *treeiter,
               GtkTreeViewColumn  *column)
{
  CodeSlayerProjectsPrivate *priv;
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  GList *selected_rows = NULL;
  GList *tmp = NULL;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  tree_model = GTK_TREE_MODEL (priv->treestore);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      CodeSlayerProject *project;
      GtkTreeIter iter;
      gchar *file_path;
      GFile *file;
      GFileType file_type;

      GtkTreePath *tree_path = tmp->data;

      gtk_tree_model_get_iter (tree_model, &iter, tree_path);

      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter, 
                          PROJECT, &project, -1);

      file_path = get_file_path_from_iter (GTK_TREE_MODEL (priv->treestore), 
                                           &iter, project);

      file = g_file_new_for_path (file_path);
      file_type = g_file_query_file_type (file, G_FILE_QUERY_INFO_NONE, NULL);
      g_object_unref (file);

      if (file_type == G_FILE_TYPE_REGULAR)
        {
          CodeSlayerDocument *document;
          GtkTreeRowReference *tree_row_reference;

          document = codeslayer_document_new ();
          codeslayer_document_set_file_path (document, file_path);
          codeslayer_document_set_project (document, project);

          tree_row_reference = gtk_tree_row_reference_new (GTK_TREE_MODEL (priv->treestore),
                                                           tree_path);
          codeslayer_document_set_tree_row_reference (document, tree_row_reference);

          g_signal_emit_by_name ((gpointer) projects, "open-document", document);
          
          g_object_unref (document);
        }
      else if (file_type == G_FILE_TYPE_DIRECTORY)
        {
          if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (priv->treeview), tree_path))
            gtk_tree_view_collapse_row (GTK_TREE_VIEW (priv->treeview), tree_path);
          else
            gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview), tree_path, FALSE);
        }

      g_free (file_path);
      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);

  return FALSE;
}

/* Walk up the tree and create the file path. This is better than storing 
   the path on the node because we do do not have to worry about effecting 
   other nodes as things change. */
static gchar *
get_file_path_from_iter (GtkTreeModel      *model, 
                         GtkTreeIter       *iter,
                         CodeSlayerProject *project)
{
  GString *result;
  const gchar *folder_path;
  GList *list = NULL;
  GList *tmp = NULL;
  GtkTreeIter parent;
  GtkTreeIter child = *iter;

  if (iter == NULL)
    return NULL;
    
  folder_path = codeslayer_project_get_folder_path (project);

  result = g_string_new (folder_path);

  /* go up the tree and get the file path */

  while (gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &parent, &child))
    {
      gchar *file_name;
      gtk_tree_model_get (GTK_TREE_MODEL (model), &parent, 
                          FILE_NAME, &file_name, -1);
      list = g_list_prepend (list, file_name);
      child = parent;
    }
    
  /* remove the first node that duplicates the project path */

  if (list != NULL)
    {
      gchar *file_name = list->data;
      list = g_list_remove (list, file_name);
      g_free (file_name);
    }

  tmp = list;

  while (tmp != NULL)
    {
      gchar *file_name = tmp->data;
      result = g_string_append (result, G_DIR_SEPARATOR_S);
      result = g_string_append (result, file_name);
      g_free (file_name);
      tmp = g_list_next (tmp);
    }

  /*g_list_foreach (list, (GFunc) g_free, NULL);*/
  g_list_free (list);

  /* append the current path at the bottom of the tree if we are not the parent node */

  if (gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &parent, iter))
    {
      gchar *file_name;
      gtk_tree_model_get (GTK_TREE_MODEL (model), iter, 
                          FILE_NAME, &file_name, -1);
      result = g_string_append (result, G_DIR_SEPARATOR_S);
      result = g_string_append (result, file_name);
      g_free (file_name);
    }

  return g_string_free (result, FALSE);
}

/* 
 * Show the popup menu with the proper items enabled 
 * based on where we are in the tree. 
 */
static gboolean
show_popup_menu (CodeSlayerProjects *projects, 
                 GdkEventButton     *event)
{
  CodeSlayerProjectsPrivate *priv;
  
  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
      GtkTreeSelection *tree_selection;
      GList *items;
      GList *tmp;

      gtk_container_foreach (GTK_CONTAINER (priv->menu), (GtkCallback) gtk_widget_hide, NULL);
    
      if (priv->ccp->sources == NULL)
        gtk_widget_set_sensitive (priv->paste_item, FALSE);
      else
        gtk_widget_set_sensitive (priv->paste_item, TRUE);

      tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
      if (gtk_tree_selection_count_selected_rows (tree_selection) <= 1)
        {
          GtkTreePath *path;

          /* Get tree path for row that was clicked */
          if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (priv->treeview),
                                            (gint) event->x, 
                                            (gint) event->y,
                                            &path, NULL, NULL, NULL))
            {
              gtk_tree_selection_unselect_all (tree_selection);
              gtk_tree_selection_select_path (tree_selection, path);
              gtk_tree_path_free (path);
            }
        }

      items = get_showable_popup_items (projects);
      tmp = items;
      
      if (tmp != NULL)
        {
          while (tmp != NULL)
            {
              GtkWidget *popup_menu_item = tmp->data;
              gtk_widget_show_all (popup_menu_item);
              tmp = g_list_next (tmp);
            }
          g_list_free (items);
          
          gtk_menu_popup (GTK_MENU (priv->menu), NULL, NULL, NULL, NULL, 
                          (event != NULL) ? event->button : 0,
                          gdk_event_get_time ((GdkEvent *) event));

          return TRUE;
        }
    }

  return FALSE;
}

static gboolean 
is_popup_item_showable (CodeSlayerProjects *projects, 
                        GtkWidget          *popup_menu_item)
{
  GList *items;
  int result;
  items = get_showable_popup_items (projects);
  result = g_list_index (items, popup_menu_item);
  g_list_free (items);
  return result != -1;
}

static GList*
get_showable_popup_items (CodeSlayerProjects *projects)
{
  CodeSlayerProjectsPrivate *priv;
    
  GtkTreeModel *tree_model;
  GtkTreeSelection *tree_selection;
  gint selected_rows_count;
  GList *selected_rows;
  GList *tmp;
  GList *results = NULL;

  priv = CODESLAYER_PROJECTS_GET_PRIVATE (projects);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  selected_rows_count = gtk_tree_selection_count_selected_rows (tree_selection);
  if (selected_rows_count == 0)
    return results;
    
  results = gtk_container_get_children (GTK_CONTAINER (priv->menu));

  tree_model = GTK_TREE_MODEL (priv->treestore);

  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);
  tmp = selected_rows;
  
  while (tmp != NULL)
    {
      GtkTreePath *tree_path = tmp->data;
      gint depth = gtk_tree_path_get_depth (tree_path);
      
      if (depth == 1 && selected_rows_count > 1)
        {
          g_list_foreach (selected_rows, (GFunc) gtk_tree_path_free, NULL);
          g_list_free (selected_rows);
          g_list_free (results);
          return NULL;
        }
        
      if (depth == 1 && selected_rows_count == 1)
        {
          results = g_list_remove (results, priv->rename_item);
          results = g_list_remove (results, priv->move_to_trash_item);
          results = g_list_remove (results, priv->cut_item);
          results = g_list_remove (results, priv->copy_item);
          results = g_list_remove (results, priv->ccp_separator);
        }
      else
        {
          GtkTreeIter iter;
          gchar *file_type;

          results = g_list_remove (results, priv->remove_project_item);
          results = g_list_remove (results, priv->project_separator);
          results = g_list_remove (results, priv->properties_separator);
          results = g_list_remove (results, priv->properties_item);

          if (selected_rows_count > 1)
            results = g_list_remove (results, priv->rename_item);

          gtk_tree_model_get_iter (tree_model, &iter, tree_path);
          gtk_tree_model_get (tree_model, &iter, FILE_TYPE, &file_type, -1);

          if (g_strcmp0 (file_type, "G_FILE_TYPE_DIRECTORY") != 0)
            results = g_list_remove (results, priv->find_item);

          if (g_strcmp0 (file_type, "G_FILE_TYPE_DIRECTORY") != 0 || 
              selected_rows_count > 1)
            {
              results = g_list_remove (results, priv->refresh_item);
              results = g_list_remove (results, priv->new_folder_item);
              results = g_list_remove (results, priv->new_file_item);
              results = g_list_remove (results, priv->new_separator);
              results = g_list_remove (results, priv->paste_item);
            }
          g_free (file_type);
        }

      gtk_tree_path_free (tree_path);
      tmp = g_list_next (tmp);
    }
  g_list_free (selected_rows);

  if (g_list_length (priv->plugins) == 0)
    results = g_list_remove (results, priv->plugins_separator);
    
  return results;
}                 
