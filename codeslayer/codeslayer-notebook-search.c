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
#include <codeslayer/codeslayer-notebook-search.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-notebook-search.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-notebook-search
 * @short_description: The notebook inline search.
 * @title: CodeSlayerNotebookSearch
 * @include: codeslayer/codeslayer-notebook-search.h
 */

static void codeslayer_notebook_search_class_init  (CodeSlayerNotebookSearchClass *klass);
static void codeslayer_notebook_search_init        (CodeSlayerNotebookSearch      *notebook_search);
static void codeslayer_notebook_search_finalize    (CodeSlayerNotebookSearch      *notebook_search);

static void add_close_button                       (CodeSlayerNotebookSearch      *notebook_search);
static void close_search_action                    (CodeSlayerNotebookSearch      *notebook_search);
static void add_find_entry                         (CodeSlayerNotebookSearch      *notebook_search);
static void add_find_previous_button               (CodeSlayerNotebookSearch      *notebook_search);
static void add_find_next_button                   (CodeSlayerNotebookSearch      *notebook_search);
static void add_match_case_button                  (CodeSlayerNotebookSearch      *notebook_search);
static void add_match_word_button                  (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_entry                      (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_button                     (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_all_button                 (CodeSlayerNotebookSearch      *notebook_search);
static void set_find_entry_color                   (CodeSlayerNotebookSearch      *notebook_search);
static void find_previous_action                   (CodeSlayerNotebookSearch      *notebook_search);
static void find_next_action                       (CodeSlayerNotebookSearch      *notebook_search);
static GtkWidget *get_editor                       (CodeSlayerNotebookSearch      *notebook_search);
static void find_entry_action                      (CodeSlayerNotebookSearch      *notebook_search);
static gboolean entry_keypress_action              (CodeSlayerNotebookSearch      *notebook_search,
                                                    GdkEventKey                   *event);
static void create_search_marks                    (CodeSlayerNotebookSearch      *notebook_search);
static void clear_search_marks                     (CodeSlayerNotebookSearch      *notebook_search);
static void clear_all_search_marks                 (CodeSlayerNotebookSearch      *notebook_search);
static void replace_action                         (CodeSlayerNotebookSearch      *notebook_search);
static void replace_all_action                     (CodeSlayerNotebookSearch      *notebook_search);
static gboolean forward_search                     (const gchar                   *find, 
                                                    GtkTextIter                   *start, 
                                                    GtkTextIter                   *begin, 
                                                    GtkTextIter                   *end, 
                                                    CodeSlayerNotebookSearch      *notebook_search);
static gboolean backward_search                    (const gchar                   *find, 
                                                    GtkTextIter                   *start, 
                                                    GtkTextIter                   *begin, 
                                                    GtkTextIter                   *end, 
                                                    CodeSlayerNotebookSearch      *notebook_search);
static gchar* entry_get_current_text               (GtkWidget                     *entry, 
                                                    GtkListStore                  *store);
static void entry_set_text                         (GtkWidget                     *entry,
                                                    GtkListStore                  *store,
                                                    gchar                         *text);
static gboolean is_active                          (GtkWidget *toggle_button);
static void search_find                            (CodeSlayerNotebookSearch      *notebook_search);
static gboolean search_marks_in_view               (GtkWidget                     *editor, 
                                                    GdkRectangle                   rect, 
                                                    GtkTextIter                    begin, 
                                                    GtkTextIter                    end);
static void save_search_options                    (CodeSlayerNotebookSearch      *notebook_search);

#define FIND "Find:"
#define FIND_INCREMENTAL "Find [I]:"

#define CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_SEARCH_TYPE, CodeSlayerNotebookSearchPrivate))

typedef struct _CodeSlayerNotebookSearchPrivate CodeSlayerNotebookSearchPrivate;

struct _CodeSlayerNotebookSearchPrivate
{
  GtkWidget          *grid;
  GtkWidget          *notebook;
  CodeSlayerSettings *settings;
  GtkWidget          *close_button;
  GtkWidget          *find_label;
  GtkWidget          *find_entry;
  GtkListStore       *find_store;
  GtkWidget          *replace_spacer;
  GtkWidget          *replace_label;
  GtkWidget          *replace_entry;
  GtkListStore       *replace_store;
  GtkWidget          *replace_button;
  GtkWidget          *replace_all_button;
  GtkWidget          *find_previous_button;  
  GtkWidget          *find_next_button;  
  GtkWidget          *match_case_button;
  GtkWidget          *match_word_button;
  GdkRGBA             error_color;
  GdkRGBA             default_color;
};

enum
{
  TEXT = 0,
  COLUMNS
};

enum
{
  CLOSE_SEARCH,
  LAST_SIGNAL
};

static guint codeslayer_notebook_search_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerNotebookSearch, codeslayer_notebook_search, GTK_TYPE_VBOX)

static void
codeslayer_notebook_search_class_init (CodeSlayerNotebookSearchClass *klass)
{

  /**
   * CodeSlayerNotebookSearch::close-search
   * @codeslayernotebooksearch: the search that received the signal.
   *
   * Note: for internal use only.
   *
   * The ::close-search signal is a request for the inline search to be closed.
   */
  codeslayer_notebook_search_signals[CLOSE_SEARCH] =
    g_signal_new ("close-search", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookSearchClass, close_search), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_notebook_search_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerNotebookSearchPrivate));
}

static void
codeslayer_notebook_search_init (CodeSlayerNotebookSearch *notebook_search)
{
}

static void
codeslayer_notebook_search_finalize (CodeSlayerNotebookSearch *notebook_search)
{
  G_OBJECT_CLASS (codeslayer_notebook_search_parent_class)->finalize (G_OBJECT (notebook_search));
}

/**
 * codeslayer_notebook_search_new:
 * @notebook: a #CodeSlayerNotebook.
 * @settings: a #CodeSlayerSettings.
 *
 * Creates a new #CodeSlayerNotebookSearch.
 *
 * Returns: a new #CodeSlayerNotebookSearch. 
 */
GtkWidget*
codeslayer_notebook_search_new (GtkWidget          *notebook, 
                                CodeSlayerSettings *settings)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *notebook_search;
  
  notebook_search = g_object_new (codeslayer_notebook_search_get_type (), NULL);
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  priv->notebook = notebook;
  priv->settings = settings;
  
  priv->grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (priv->grid), 2);
  gtk_grid_set_column_spacing (GTK_GRID (priv->grid), 2);
  gtk_widget_set_margin_left (priv->grid, 4);
  
  add_close_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_find_entry (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_find_previous_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_find_next_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_match_case_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_match_word_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_replace_entry (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_replace_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_replace_all_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  
  gtk_box_pack_start (GTK_BOX (notebook_search), priv->grid, FALSE, FALSE, 2);
  
  set_find_entry_color (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  
  return notebook_search;
}

static void
set_find_entry_color (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkStyleContext *style_context;
  GtkWidget *label;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  label = gtk_bin_get_child (GTK_BIN (priv->find_entry));
  style_context = gtk_widget_get_style_context (label);

  gdk_rgba_parse (&(priv->error_color), "#ed3636");
  gtk_style_context_get_color (style_context, GTK_STATE_FLAG_NORMAL, &(priv->default_color));  
}

/**
 * codeslayer_notebook_search_find:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 *
 * Search the editors highlighted text.
 */
void
codeslayer_notebook_search_find (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  gtk_label_set_text (GTK_LABEL (priv->find_label), _(FIND));
  
  gtk_widget_hide (priv->replace_spacer);
  gtk_widget_hide (priv->replace_label);
  gtk_widget_hide (priv->replace_entry);
  gtk_widget_hide (priv->replace_button);
  gtk_widget_hide (priv->replace_all_button);
  
  search_find (notebook_search);
}

/**
 * codeslayer_notebook_search_find_next:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 *
 * Find the next search value.
 */
void
codeslayer_notebook_search_find_next (CodeSlayerNotebookSearch *notebook_search)
{
  find_next_action (notebook_search);
}

/**
 * codeslayer_notebook_search_find_previous:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 *
 * Find the previous search value.
 */
void
codeslayer_notebook_search_find_previous (CodeSlayerNotebookSearch *notebook_search)
{
  find_previous_action (notebook_search);
}

/**
 * codeslayer_notebook_search_replace:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 *
 * Replace the editors highlighted text.
 */
void
codeslayer_notebook_search_replace (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  GtkTextIter start, end;
  gchar *current;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  editor = get_editor (notebook_search);
  if (editor == NULL)
    return;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  insert_mark = gtk_text_buffer_get_insert (buffer);
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  current = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  if (g_strcmp0 (current, "") != 0)
    {
      entry_set_text (priv->find_entry, priv->find_store, current);
      entry_set_text (priv->replace_entry, priv->replace_store, current);
      gtk_widget_grab_focus (GTK_WIDGET (priv->replace_entry));
    }
    
  gtk_label_set_text (GTK_LABEL (priv->find_label), _(FIND));

  gtk_widget_show (priv->replace_spacer);
  gtk_widget_show (priv->replace_label);
  gtk_widget_show (priv->replace_entry);
  gtk_widget_show (priv->replace_button);
  gtk_widget_show (priv->replace_all_button);

  create_search_marks (notebook_search);
  
  if (current != NULL)
    g_free (current);
}

static void
add_close_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *button;
  GtkWidget *image;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  button = gtk_button_new ();
  priv->close_button = button;

  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_widget_set_can_focus (button, FALSE);
  codeslayer_utils_style_close_button (button);

  g_signal_connect_swapped (G_OBJECT (button), "clicked",
                            G_CALLBACK (close_search_action), notebook_search);

  gtk_grid_attach (GTK_GRID (priv->grid), button, 0, 0, 1, 1);
}

static void
close_search_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  clear_all_search_marks (notebook_search);
  entry_set_text (priv->replace_entry, priv->replace_store, "");
  g_signal_emit_by_name ((gpointer) notebook_search, "close-search");
}

static void
add_find_entry (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *find_label;
  GtkWidget *find_entry;
  GtkListStore *find_store;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  find_label = gtk_label_new (_(FIND));
  priv->find_label = find_label;
  gtk_misc_set_alignment (GTK_MISC (find_label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (find_label), 4, 0);
  gtk_grid_attach_next_to (GTK_GRID (priv->grid), find_label, priv->close_button, 
                           GTK_POS_RIGHT, 1, 1);

  find_store = gtk_list_store_new (COLUMNS, G_TYPE_STRING);
  priv->find_store = find_store;

  find_entry = gtk_combo_box_text_new_with_entry ();
  priv->find_entry = find_entry;
  gtk_combo_box_set_model (GTK_COMBO_BOX (find_entry), GTK_TREE_MODEL (find_store));
  g_object_unref (find_store);
  
  entry_set_text (find_entry, find_store, "");                                                   
  gtk_widget_set_size_request (find_entry, 250, -1);                                                   
  
  g_signal_connect_swapped (G_OBJECT (find_entry), "changed",
                            G_CALLBACK (find_entry_action), notebook_search);
                    
  g_signal_connect_swapped (G_OBJECT (find_entry), "key-press-event",
                            G_CALLBACK (entry_keypress_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), find_entry, find_label, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_find_previous_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *find_previous_button;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  find_previous_button = gtk_button_new_with_label (_("Previous"));
  priv->find_previous_button = find_previous_button;
  gtk_button_set_relief (GTK_BUTTON (find_previous_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (find_previous_button), FALSE);

  gtk_widget_set_can_focus (find_previous_button, FALSE);

  g_signal_connect_swapped (G_OBJECT (find_previous_button), "clicked",
                            G_CALLBACK (find_previous_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), find_previous_button, priv->find_entry, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_find_next_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *find_next_button;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  find_next_button = gtk_button_new_with_label (_("Next"));
  priv->find_next_button = find_next_button;
  gtk_button_set_relief (GTK_BUTTON (find_next_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (find_next_button), FALSE);
  gtk_widget_set_can_focus (find_next_button, FALSE);

  g_signal_connect_swapped (G_OBJECT (find_next_button), "clicked",
                            G_CALLBACK (find_next_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), find_next_button, priv->find_previous_button, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_match_case_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *match_case_button;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  match_case_button = gtk_check_button_new_with_label (_("Match Case"));
  gtk_widget_set_can_focus (match_case_button, FALSE);
  priv->match_case_button = match_case_button;
  
  g_signal_connect_swapped (G_OBJECT (match_case_button), "clicked",
                            G_CALLBACK (create_search_marks), notebook_search);

  g_signal_connect_swapped (G_OBJECT (match_case_button), "clicked",
                            G_CALLBACK (save_search_options), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), match_case_button, priv->find_next_button, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_match_word_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *match_word_button;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  match_word_button = gtk_check_button_new_with_label (_("Match Word"));
  gtk_widget_set_can_focus (match_word_button, FALSE);
  priv->match_word_button = match_word_button;
  
  g_signal_connect_swapped (G_OBJECT (match_word_button), "clicked",
                            G_CALLBACK (create_search_marks), notebook_search);

  g_signal_connect_swapped (G_OBJECT (match_word_button), "clicked",
                            G_CALLBACK (save_search_options), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), match_word_button, priv->match_case_button, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_replace_entry (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *replace_spacer;
  GtkWidget *replace_label;
  GtkWidget *replace_entry;
  GtkListStore *replace_store;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  replace_spacer = gtk_label_new ("");
  priv->replace_spacer = replace_spacer;
  gtk_grid_attach (GTK_GRID (priv->grid), replace_spacer, 0, 1, 1, 1);

  replace_label = gtk_label_new (_("Replace:"));
  priv->replace_label = replace_label;
  
  gtk_misc_set_alignment (GTK_MISC (replace_label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (replace_label), 4, 0);
  gtk_grid_attach_next_to (GTK_GRID (priv->grid), replace_label, replace_spacer, 
                           GTK_POS_RIGHT, 1, 1);

  replace_store = gtk_list_store_new (COLUMNS, G_TYPE_STRING);
  priv->replace_store = replace_store;

  replace_entry = gtk_combo_box_text_new_with_entry ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (replace_entry), GTK_TREE_MODEL (replace_store));
  entry_set_text (replace_entry, replace_store, "");
  gtk_widget_set_size_request (replace_entry, 250, -1);                                                   
  priv->replace_entry = replace_entry;
  
  g_signal_connect_swapped (G_OBJECT (replace_entry), "key-press-event",
                            G_CALLBACK (entry_keypress_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), replace_entry, replace_label, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_replace_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *replace_button;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  replace_button = gtk_button_new_with_label (_("Replace"));
  priv->replace_button = replace_button;
  gtk_button_set_relief (GTK_BUTTON (replace_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (replace_button), FALSE);
  gtk_widget_set_can_focus (replace_button, FALSE);

  g_signal_connect_swapped (G_OBJECT (replace_button), "clicked",
                            G_CALLBACK (replace_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), replace_button, priv->replace_entry, 
                           GTK_POS_RIGHT, 2, 1);
}

static void
add_replace_all_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *replace_all_button;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  replace_all_button = gtk_button_new_with_label (_("Replace All"));
  priv->replace_all_button = replace_all_button;
  gtk_button_set_relief (GTK_BUTTON (replace_all_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (replace_all_button), FALSE);
  gtk_widget_set_can_focus (replace_all_button, FALSE);

  g_signal_connect_swapped (G_OBJECT (replace_all_button), "clicked",
                            G_CALLBACK (replace_all_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), replace_all_button, priv->replace_button, 
                           GTK_POS_RIGHT, 1, 1);
}

/**
 * codeslayer_notebook_search_sync_with_notebook:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 * 
 * Update the sensitivity of search related items based on the current 
 * state of the notebook #CodeSlayerEditor widgets.
 */
void
codeslayer_notebook_search_sync_with_notebook (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  gboolean match_case_selected;
  gboolean match_word_selected;
  gboolean sensitive;
  gint pages;

  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));
  sensitive = pages > 0;
  gtk_widget_set_sensitive (priv->find_entry, sensitive);
  gtk_widget_set_sensitive (priv->replace_entry, sensitive);
  gtk_widget_set_sensitive (priv->find_next_button, sensitive);
  gtk_widget_set_sensitive (priv->find_previous_button, sensitive);
  gtk_widget_set_sensitive (priv->replace_button, sensitive);
  gtk_widget_set_sensitive (priv->replace_all_button, sensitive);
  gtk_widget_set_sensitive (priv->match_case_button, sensitive);
  gtk_widget_set_sensitive (priv->match_word_button, sensitive);
    
  match_case_selected = codeslayer_settings_get_boolean (priv->settings,
                                                         CODESLAYER_SETTINGS_NOTEBOOK_SEARCH_MATCH_CASE);
  match_word_selected = codeslayer_settings_get_boolean (priv->settings,
                                                         CODESLAYER_SETTINGS_NOTEBOOK_SEARCH_MATCH_WORD);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->match_case_button), match_case_selected);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->match_word_button), match_word_selected);
}

static void
find_entry_action (CodeSlayerNotebookSearch *notebook_search)
{
  create_search_marks (notebook_search);
}

static gboolean            
entry_keypress_action (CodeSlayerNotebookSearch *notebook_search,
                       GdkEventKey              *event)
{
  if (event->keyval == GDK_KEY_Escape)
    close_search_action (notebook_search);

  return FALSE;
}                            

static void
search_find (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  GtkTextIter start, end;
  gchar *current;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  editor = get_editor (notebook_search);
  if (editor == NULL)
    return;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  insert_mark = gtk_text_buffer_get_insert (buffer);
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  current = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  if (g_strcmp0 (current, "") != 0)
    entry_set_text (priv->find_entry, priv->find_store, current);
  else
    gtk_widget_grab_focus (GTK_WIDGET (priv->find_entry));
    
  if (current != NULL)
    g_free (current);

  codeslayer_notebook_search_create_search_marks (notebook_search, FALSE);
}

static void
create_search_marks (CodeSlayerNotebookSearch *notebook_search)
{
  codeslayer_notebook_search_create_search_marks (notebook_search, TRUE);
}

/**
 * codeslayer_notebook_search_create_search_marks:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 * @scrollable: is TRUE if should scroll after finding marks.
 * 
 * Create the search marks based on the current settings.
 */
void
codeslayer_notebook_search_create_search_marks (CodeSlayerNotebookSearch *notebook_search, 
                                                gboolean                  scrollable)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *editor;
  GtkWidget *label;
  GdkRectangle rect;
  GtkTextBuffer *buffer;
  GtkTextIter start, first, begin, end;
  gchar *find;
  gboolean success;
  gboolean need_to_scroll = FALSE;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  clear_search_marks (notebook_search);

  find = entry_get_current_text (priv->find_entry, priv->find_store);
  label = gtk_bin_get_child (GTK_BIN (priv->find_entry));

  editor = get_editor (notebook_search);
  if (editor == NULL || g_strcmp0 (find, "") == 0)
    {
      gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->default_color));
      if (find)
        g_free (find);
      return;
    }

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (editor), &rect);

  success = forward_search (find, &start, &begin, &end, notebook_search);
  
  if (!success)
    {
      gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->error_color));
      if (find)
        g_free (find);
        
      return;
    }

  gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->default_color));
  need_to_scroll = !search_marks_in_view (editor, rect, begin, end);
  first = begin;
  
  while (success)
    {
      if (need_to_scroll)
        need_to_scroll = !search_marks_in_view (editor, rect, begin, end);
    
      gtk_text_buffer_apply_tag_by_name (buffer, "search-marks", &begin, &end);
      gtk_text_iter_forward_char (&start);
      success = forward_search (find, &start, &begin, &end, notebook_search);
      start = begin;
    }

  if (need_to_scroll && scrollable)
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor), &first, .1, FALSE, 0, 0);
      gtk_text_buffer_place_cursor (buffer, &first);
    }
    
  if (find)
    g_free (find);
}

static gboolean
search_marks_in_view (GtkWidget    *editor, 
                      GdkRectangle  rect, 
                      GtkTextIter   begin, 
                      GtkTextIter   end) 
{
  GtkTextIter start_visible;
  GtkTextIter end_visible;

  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (editor), &start_visible,
                                      rect.x, rect.y);
                                      
  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (editor), &end_visible,
                                      rect.x + rect.width, rect.y + rect.height);
                                      
  if (gtk_text_iter_in_range (&begin, &start_visible, &end_visible) ||
      gtk_text_iter_in_range (&end, &start_visible, &end_visible))
    {
      return TRUE;
    }

  return FALSE;
}

static void
save_search_options (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  gboolean match_case_selected;
  gboolean match_word_selected;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  match_case_selected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_button));
  match_word_selected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_button));
  
  codeslayer_settings_set_boolean (priv->settings, 
                                   CODESLAYER_SETTINGS_NOTEBOOK_SEARCH_MATCH_CASE,
                                   match_case_selected);

  codeslayer_settings_set_boolean (priv->settings, 
                                   CODESLAYER_SETTINGS_NOTEBOOK_SEARCH_MATCH_WORD,
                                   match_word_selected);
}

static void
find_previous_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  GtkTextMark *insert_mark;
  gchar *find;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  editor = get_editor (notebook_search);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  find = entry_get_current_text (priv->find_entry, priv->find_store);

  insert_mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);

  if (backward_search (find, &start, &begin, &end, notebook_search))
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor), 
                                    &begin, .1, FALSE, 0, 0);
      gtk_text_buffer_select_range (buffer, &begin, &end);
    }
  else
    {
      gtk_text_buffer_get_end_iter (buffer, &start);
      if (backward_search (find, &start, &begin, &end, notebook_search))
        {
          gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor), 
                                        &begin, .1, FALSE, 0, 0);
          gtk_text_buffer_select_range (buffer, &begin, &end);
        }
    }
    
  entry_set_text (priv->find_entry, priv->find_store, find);    
    
  if (find)
    g_free (find);    
}

static void
find_next_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  GtkTextMark *insert_mark;
  gchar *find;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  editor = get_editor (notebook_search);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  find = entry_get_current_text (priv->find_entry, priv->find_store);

  insert_mark = gtk_text_buffer_get_selection_bound (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);

  if (forward_search (find, &start, &begin, &end, notebook_search))
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor), 
                                    &begin, .1, FALSE, 0, 0);
      gtk_text_buffer_select_range (buffer, &begin, &end);
    }
  else
    {
      gtk_text_buffer_get_start_iter (buffer, &start);
      if (forward_search (find, &start, &begin, &end, notebook_search))
        {
          gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor), 
                                        &begin, .1, FALSE, 0, 0);
          gtk_text_buffer_select_range (buffer, &begin, &end);
        }
    }
    
  entry_set_text (priv->find_entry, priv->find_store, find);    
    
  if (find)
    g_free (find);    
}

static GtkWidget*
get_editor (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  gint page;
  GtkWidget *notebook_page;

  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (priv->notebook));
  
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
  if (notebook_page == NULL)
    return NULL;

  return codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
}

static void
clear_search_marks (CodeSlayerNotebookSearch *notebook_search)
{
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  GtkTextIter start, end;

  editor = get_editor (notebook_search);
  if (editor == NULL)
    return;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "search-marks", &start, &end);
}

static void
clear_all_search_marks (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  gint pages;
  gint page;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));

  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page;
      GtkWidget *editor;
      GtkTextBuffer *buffer;
      GtkTextIter start, end;

      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      editor = codeslayer_notebook_page_get_editor (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_remove_tag_by_name (buffer, "search-marks", &start, &end);
    }
}

static void
replace_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  gchar *current;
  gchar *find;
  gchar *replace;
  GtkTextIter start, end;
  gboolean is_search_same;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  editor = get_editor (notebook_search);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  find = entry_get_current_text (priv->find_entry, priv->find_store);
  replace = entry_get_current_text (priv->replace_entry, priv->replace_store);

  insert_mark = gtk_text_buffer_get_insert (buffer);
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  /* make sure that the highlighted text is in the current search */
  current = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  is_search_same = FALSE;

  if (is_active (priv->match_case_button))
    is_search_same = g_ascii_strcasecmp (find, current) == 0;
  else
    is_search_same = g_strcmp0 (find, current) == 0;

  if (is_search_same)
    {
      gtk_text_buffer_begin_user_action (buffer);
      gtk_text_buffer_delete (buffer, &start, &end);
      gtk_text_buffer_insert (buffer, &start, replace, -1);
      gtk_text_buffer_end_user_action (buffer);
    }
    
  entry_set_text (priv->find_entry, priv->find_store, find);    
  entry_set_text (priv->replace_entry, priv->replace_store, replace);    

  if (find != NULL)
    g_free (find);
    
  if (replace != NULL)
    g_free (replace);    

  if (current != NULL)
    g_free (current);    

  find_next_action (notebook_search);
}

static void
replace_all_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  gchar *find;
  gchar *replace;
  GtkTextIter start, begin, end;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  editor = get_editor (notebook_search);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  find = entry_get_current_text (priv->find_entry, priv->find_store);
  replace = entry_get_current_text (priv->replace_entry, priv->replace_store);

  gtk_text_buffer_get_start_iter (buffer, &start);

  gtk_text_buffer_begin_user_action (buffer);

  while (forward_search (find, &start, &begin, &end, notebook_search))
    {
      gtk_text_buffer_delete (buffer, &begin, &end);
      gtk_text_buffer_insert (buffer, &begin, replace, -1);
      start = begin;
    }

  gtk_text_buffer_end_user_action (buffer);

  entry_set_text (priv->find_entry, priv->find_store, find);    
  entry_set_text (priv->replace_entry, priv->replace_store, replace);    

  if (find)
    g_free (find);

  if (replace)
    g_free (replace);
}

static gboolean
backward_search (const gchar *find, 
                 GtkTextIter *start, 
                 GtkTextIter *begin,
                 GtkTextIter *end,
                 CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  gboolean result;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  if (is_active (priv->match_case_button))
      result = gtk_text_iter_backward_search (start, find, 0, begin, end, NULL);
  else
      result = gtk_text_iter_backward_search (start, find,
                                              GTK_TEXT_SEARCH_CASE_INSENSITIVE,
                                              begin, end, NULL);

  if (result && is_active (priv->match_word_button))
    {
      if (!(gtk_text_iter_starts_word (begin) && 
             gtk_text_iter_ends_word (end)))
        {
          *start = *begin;
          result = backward_search (find, start, begin, end, notebook_search);
        }
    }

  return result;
}

static gboolean
forward_search (const gchar              *find, 
                GtkTextIter              *start, 
                GtkTextIter              *begin,
                GtkTextIter              *end, 
                CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  gboolean result;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  if (is_active (priv->match_case_button))
      result = gtk_text_iter_forward_search (start, find, 0, begin, end, NULL);
  else
      result = gtk_text_iter_forward_search (start, find,
                                             GTK_TEXT_SEARCH_CASE_INSENSITIVE,
                                             begin, end, NULL);

  if (result && is_active (priv->match_word_button))
    {
      if (!(gtk_text_iter_starts_word (begin) && gtk_text_iter_ends_word (end)))
        {
          *start = *begin;
          gtk_text_iter_forward_char (start);
          result = forward_search (find, start, begin, end, notebook_search);
        }
    }

  return result;
}

static gboolean
is_active (GtkWidget *toggle_button)
{
  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button));
}

static gchar*
entry_get_current_text (GtkWidget    *entry,
                        GtkListStore *store) 
{
  gchar *result;
  GtkTreeIter iter;
  
  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (entry), &iter))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, TEXT, &result, -1);
      return result;
    }
  
  return gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (entry));
}

static void
entry_set_text (GtkWidget    *entry,
                GtkListStore *store,
                gchar        *text)
{
  gchar *value;
  gint count;
  GtkTreeIter root;
  GtkTreeIter iter;

  count = 0;

  gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &root);

  if (gtk_tree_model_iter_children (GTK_TREE_MODEL(store), &iter, NULL))
    {
      do
        {
          count++;
          gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, TEXT, &value, -1);
          
          if (g_strcmp0 (text, value) == 0)
            {
              if (count > 1)
                gtk_list_store_move_after (store, &iter, &root);
              
              gtk_combo_box_set_active_iter (GTK_COMBO_BOX (entry), &iter);
              g_free (value);
              return;
            }
            
          g_free (value);
        
        }
      while (gtk_tree_model_iter_next (GTK_TREE_MODEL(store), &iter));
    }
  
  if (count >= 15)
    {
      if (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (store), &iter, NULL, 14))
        gtk_list_store_remove (store, &iter);
    }
    
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, TEXT, text, -1);

  if (count > 0)
    gtk_list_store_move_after (store, &iter, &root);
    
  gtk_combo_box_set_active_iter (GTK_COMBO_BOX (entry), &iter);
}
