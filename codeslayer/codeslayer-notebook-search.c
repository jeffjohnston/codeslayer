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

#include <gtksourceview/gtksourceiter.h>
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
static void add_match_entire_word_button           (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_entry                      (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_button                     (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_all_button                 (CodeSlayerNotebookSearch      *notebook_search);
static void find_previous_action                   (CodeSlayerNotebookSearch      *notebook_search);
static void find_next_action                       (CodeSlayerNotebookSearch      *notebook_search);
static GtkWidget *get_editor                       (CodeSlayerNotebookSearch      *notebook_search);
static void find_entry_action                      (CodeSlayerNotebookSearch      *notebook_search);
static gboolean entry_keypress_action              (CodeSlayerNotebookSearch      *notebook_search,
                                                    GdkEventKey                   *event);
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
static void create_search_marks                    (CodeSlayerNotebookSearch      *notebook_search);

#define FIND "Find:"
#define FIND_INCREMENTAL "Find [I]:"

#define CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_SEARCH_TYPE, CodeSlayerNotebookSearchPrivate))

typedef struct _CodeSlayerNotebookSearchPrivate CodeSlayerNotebookSearchPrivate;

struct _CodeSlayerNotebookSearchPrivate
{
  GtkWidget *notebook;
  GtkWidget *find_label;
  GtkWidget *find_entry;
  GtkListStore *find_store;
  GtkWidget *replace_label;
  GtkWidget *replace_entry;
  GtkListStore *replace_store;
  GtkWidget *replace_button;
  GtkWidget *replace_all_button;
  GtkWidget *find_previous_button;  
  GtkWidget *find_next_button;  
  GtkWidget *match_case_button;
  GtkWidget *match_entire_word_button;
  gboolean incremental;
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

G_DEFINE_TYPE (CodeSlayerNotebookSearch, codeslayer_notebook_search, GTK_TYPE_TABLE)

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
  gtk_table_set_homogeneous (GTK_TABLE (notebook_search), FALSE);
  gtk_table_resize (GTK_TABLE (notebook_search), 2, 7);

  add_close_button (notebook_search);
  add_find_entry (notebook_search);
  add_find_previous_button (notebook_search);
  add_find_next_button (notebook_search);
  add_match_case_button (notebook_search);
  add_match_entire_word_button (notebook_search);
  add_replace_entry (notebook_search);
  add_replace_button (notebook_search);
  add_replace_all_button (notebook_search);
}

static void
codeslayer_notebook_search_finalize (CodeSlayerNotebookSearch *notebook_search)
{
  G_OBJECT_CLASS (codeslayer_notebook_search_parent_class)->finalize (G_OBJECT (notebook_search));
}

/**
 * codeslayer_notebook_search_new:
 * @notebook: a #CodeSlayerNotebook.
 *
 * Creates a new #CodeSlayerNotebookSearch.
 *
 * Returns: a new #CodeSlayerNotebookSearch. 
 */
GtkWidget*
codeslayer_notebook_search_new (GtkWidget *notebook)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *notebook_search;
  
  notebook_search = g_object_new (codeslayer_notebook_search_get_type (), NULL);
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  priv->notebook = notebook;
  priv->incremental = FALSE;
  
  return notebook_search;
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
  
  priv->incremental = FALSE;
  gtk_label_set_text (GTK_LABEL (priv->find_label), _(FIND));
  
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
 * codeslayer_notebook_search_find_incremental:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 *
 * Find the search value using the incremental search.
 */
void
codeslayer_notebook_search_find_incremental (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  priv->incremental = TRUE;
  gtk_label_set_text (GTK_LABEL (priv->find_label), _(FIND_INCREMENTAL));
  
  gtk_widget_hide (priv->replace_label);
  gtk_widget_hide (priv->replace_entry);
  gtk_widget_hide (priv->replace_button);
  gtk_widget_hide (priv->replace_all_button);
  
  clear_all_search_marks (notebook_search);
  entry_set_text (priv->find_entry, priv->find_store, "");
  gtk_widget_grab_focus (GTK_WIDGET (priv->find_entry));
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
    
  priv->incremental = FALSE;
  gtk_label_set_text (GTK_LABEL (priv->find_label), _(FIND));

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
  GtkWidget *close_button;
  GtkWidget *close_image;
  
  close_button = gtk_button_new ();

  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
  close_image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (close_button), close_image);
  gtk_widget_set_can_focus (close_button, FALSE);

  gtk_rc_parse_string ("style \"my-button-style\"\n"
                       "{\n"
                       "  GtkWidget::focus-padding = 0\n"
                       "  GtkWidget::focus-line-width = 0\n"
                       "  xthickness = 0\n"
                       "  ythickness = 0\n"
                       "}\n"
                       "widget \"*.my-close-button\" style \"my-button-style\"");
  gtk_widget_set_name (close_button, "my-close-button");

  g_signal_connect_swapped (G_OBJECT (close_button), "clicked",
                            G_CALLBACK (close_search_action), notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), close_button, 
                    0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
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
  gtk_table_attach (GTK_TABLE (notebook_search), find_label, 
                    1, 2, 0, 1, GTK_FILL, GTK_SHRINK, 4, 0);

  find_store = gtk_list_store_new (COLUMNS, G_TYPE_STRING);
  priv->find_store = find_store;

  find_entry = gtk_combo_box_text_new_with_entry ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (find_entry), GTK_TREE_MODEL (find_store));
  entry_set_text (find_entry, find_store, "");                                                   
  gtk_widget_set_size_request (find_entry, 250, -1);                                                   
  priv->find_entry = find_entry;
  
  g_signal_connect_swapped (G_OBJECT (find_entry), "changed",
                            G_CALLBACK (find_entry_action), notebook_search);
                    
  g_signal_connect_swapped (G_OBJECT (find_entry), "key-press-event",
                            G_CALLBACK (entry_keypress_action), notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), find_entry, 
                    2, 3, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
}

static void
add_find_previous_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *find_previous_button;
  GtkWidget *find_previous_label;
  GtkWidget *find_previous_image;
  GtkWidget *hbox;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  find_previous_button = gtk_button_new ();
  priv->find_previous_button = find_previous_button;
  gtk_button_set_relief (GTK_BUTTON (find_previous_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (find_previous_button), FALSE);

  find_previous_label = gtk_label_new (_("Previous"));
  find_previous_image = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, 
                                                  GTK_ICON_SIZE_BUTTON);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), find_previous_image, FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (hbox), find_previous_label, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (find_previous_button), hbox);
  gtk_widget_set_can_focus (find_previous_button, FALSE);

  g_signal_connect_swapped (G_OBJECT (find_previous_button), "clicked",
                            G_CALLBACK (find_previous_action), notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), find_previous_button, 
                    3, 4, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
}

static void
add_find_next_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *find_next_button;
  GtkWidget *find_next_label;
  GtkWidget *find_next_image;
  GtkWidget *hbox;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  find_next_button = gtk_button_new ();
  priv->find_next_button = find_next_button;
  gtk_button_set_relief (GTK_BUTTON (find_next_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (find_next_button), FALSE);

  find_next_label = gtk_label_new (_("Next"));
  find_next_image = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, 
                                              GTK_ICON_SIZE_BUTTON);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), find_next_image, FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (hbox), find_next_label, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (find_next_button), hbox);
  gtk_widget_set_can_focus (find_next_button, FALSE);

  g_signal_connect_swapped (G_OBJECT (find_next_button), "clicked",
                            G_CALLBACK (find_next_action), notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), find_next_button, 
                    4, 5, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
}

static void
add_match_case_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *match_case_button;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  match_case_button = gtk_check_button_new_with_label (_("Match Case"));
  gtk_widget_set_can_focus (match_case_button, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_case_button), TRUE);
  priv->match_case_button = match_case_button;

  g_signal_connect_swapped (G_OBJECT (match_case_button), "clicked",
                            G_CALLBACK (create_search_marks), 
                            notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), match_case_button, 
                    5, 6, 0, 1, GTK_SHRINK, GTK_SHRINK, 4, 0);
}

static void
add_match_entire_word_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *match_entire_word_button;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  match_entire_word_button = gtk_check_button_new_with_label (_("Match Word"));
  gtk_widget_set_can_focus (match_entire_word_button, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_entire_word_button), 
                                                TRUE);
  priv->match_entire_word_button = match_entire_word_button;

  g_signal_connect_swapped (G_OBJECT (match_entire_word_button), "clicked",
                            G_CALLBACK (create_search_marks), 
                            notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), match_entire_word_button, 
                    6, 7, 0, 1, GTK_SHRINK, GTK_SHRINK, 4, 0);
}

static void
add_replace_entry (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *replace_label;
  GtkWidget *replace_entry;
  GtkListStore *replace_store;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  replace_label = gtk_label_new (_("Replace:"));
  priv->replace_label = replace_label;
  
  gtk_misc_set_alignment (GTK_MISC (replace_label), 1, .5);
  gtk_table_attach (GTK_TABLE (notebook_search), replace_label, 
                    1, 2, 1, 2, GTK_FILL, GTK_SHRINK, 4, 0);

  replace_store = gtk_list_store_new (COLUMNS, G_TYPE_STRING);
  priv->replace_store = replace_store;

  replace_entry = gtk_combo_box_text_new_with_entry ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (replace_entry), GTK_TREE_MODEL (replace_store));
  entry_set_text (replace_entry, replace_store, "");
  gtk_widget_set_size_request (replace_entry, 250, -1);                                                   
  priv->replace_entry = replace_entry;
  
  g_signal_connect_swapped (G_OBJECT (replace_entry), "key-press-event",
                            G_CALLBACK (entry_keypress_action), notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), replace_entry, 
                    2, 3, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
}

static void
add_replace_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *replace_button;
  GtkWidget *replace_label;
  GtkWidget *replace_image;
  GtkWidget *hbox;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  replace_button = gtk_button_new ();
  priv->replace_button = replace_button;
  gtk_button_set_relief (GTK_BUTTON (replace_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (replace_button), FALSE);

  replace_label = gtk_label_new (_("Replace"));
  gtk_misc_set_alignment (GTK_MISC (replace_label), 0, .5);
  replace_image = gtk_image_new_from_stock (GTK_STOCK_FIND_AND_REPLACE,
                                            GTK_ICON_SIZE_BUTTON);
  gtk_misc_set_alignment (GTK_MISC (replace_image), 1, .5);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), replace_image, FALSE, TRUE, 3);
  gtk_box_pack_start (GTK_BOX (hbox), replace_label, FALSE, TRUE, 0);
  gtk_widget_set_can_focus (replace_button, FALSE);

  gtk_container_add (GTK_CONTAINER (replace_button), hbox);

  g_signal_connect_swapped (G_OBJECT (replace_button), "clicked",
                            G_CALLBACK (replace_action), notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), replace_button, 
                    3, 5, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);
}

static void
add_replace_all_button (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *replace_all_button;
  GtkWidget *replace_all_label;
  GtkWidget *hbox;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  replace_all_button = gtk_button_new ();
  priv->replace_all_button = replace_all_button;
  gtk_button_set_relief (GTK_BUTTON (replace_all_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (replace_all_button), FALSE);

  replace_all_label = gtk_label_new (_("Replace All"));
  gtk_misc_set_alignment (GTK_MISC (replace_all_label), 1, .5);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), replace_all_label, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (replace_all_button), hbox);
  gtk_widget_set_can_focus (replace_all_button, FALSE);

  g_signal_connect_swapped (G_OBJECT (replace_all_button), "clicked",
                            G_CALLBACK (replace_all_action), notebook_search);

  gtk_table_attach (GTK_TABLE (notebook_search), replace_all_button, 
                    5, 6, 1, 2, GTK_FILL, GTK_SHRINK, 0, 0);
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
  gtk_widget_set_sensitive (priv->match_entire_word_button, sensitive);
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

  create_search_marks (notebook_search);
}

static void
create_search_marks (CodeSlayerNotebookSearch *notebook_search)
{
  codeslayer_notebook_search_create_search_marks (notebook_search, FALSE);
}

/**
 * codeslayer_notebook_search_create_search_marks:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 * @override_incremental: if TRUE the incremental flag will be overridden (if set).
 * 
 * Create the search marks based on the current settings.
 */
void
codeslayer_notebook_search_create_search_marks (CodeSlayerNotebookSearch *notebook_search,
                                                gboolean                  override_incremental)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *editor;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  gchar *find;
  gboolean success;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  clear_search_marks (notebook_search);

  find = entry_get_current_text (priv->find_entry, priv->find_store);

  editor = get_editor (notebook_search);
  if (editor == NULL || g_strcmp0 (find, "") == 0)
    {
      if (find)
        g_free (find);
      return;
    }

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  gtk_text_buffer_get_start_iter (buffer, &start);

  success = forward_search (find, &start, &begin, &end, notebook_search);

  if (success && (!override_incremental && priv->incremental))
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (editor), &begin, .1, FALSE, 0, 0);
      gtk_text_buffer_place_cursor (buffer, &begin);
    }

  while (success)
    {
      gtk_text_buffer_apply_tag_by_name (buffer, "search-marks", &begin, &end);
      gtk_text_iter_forward_char (&start);
      success = forward_search (find, &start, &begin, &end, notebook_search);
      start = begin;
    }
    
  if (find)
    g_free (find);
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
      result = gtk_source_iter_backward_search (start, find, 0, begin, end, NULL);
  else
      result = gtk_source_iter_backward_search (start, find,
                                                GTK_SOURCE_SEARCH_CASE_INSENSITIVE,
                                                begin, end, NULL);

  if (result && is_active (priv->match_entire_word_button))
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
      result = gtk_source_iter_forward_search (start, find, 0, begin, end, NULL);
  else
      result = gtk_source_iter_forward_search (start, find,
                                               GTK_SOURCE_SEARCH_CASE_INSENSITIVE,
                                               begin, end, NULL);

  if (result && is_active (priv->match_entire_word_button))
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
