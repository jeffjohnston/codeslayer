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
#include <codeslayer/codeslayer-sourceview.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-registry.h>

/**
 * SECTION:codeslayer-notebook-search
 * @short_description: The notebook inline search.
 * @title: CodeSlayerNotebookSearch
 * @include: codeslayer/codeslayer-notebook-search.h
 */

static void codeslayer_notebook_search_class_init  (CodeSlayerNotebookSearchClass *klass);
static void codeslayer_notebook_search_init        (CodeSlayerNotebookSearch      *notebook_search);
static void codeslayer_notebook_search_finalize    (CodeSlayerNotebookSearch      *notebook_search);

static void sync_notebook_action                   (CodeSlayerNotebookSearch      *notebook_search);
static void add_close_button                       (CodeSlayerNotebookSearch      *notebook_search);
static void close_search_action                    (CodeSlayerNotebookSearch      *notebook_search);
static void add_find_entry                         (CodeSlayerNotebookSearch      *notebook_search);
static void add_find_previous_button               (CodeSlayerNotebookSearch      *notebook_search);
static void add_find_next_button                   (CodeSlayerNotebookSearch      *notebook_search);
static void add_highlight_all_checkbox             (CodeSlayerNotebookSearch      *notebook_search);
static void add_match_case_checkbox                (CodeSlayerNotebookSearch      *notebook_search);
static void add_match_word_checkbox                (CodeSlayerNotebookSearch      *notebook_search);
static void add_regex_checkbox                     (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_entry                      (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_button                     (CodeSlayerNotebookSearch      *notebook_search);
static void add_replace_all_button                 (CodeSlayerNotebookSearch      *notebook_search);
static void find_action                            (CodeSlayerNotebookSearch      *notebook_search);
static void find_next_action                       (CodeSlayerNotebookSearch      *notebook_search);
static void find_previous_action                   (CodeSlayerNotebookSearch      *notebook_search);
static GtkWidget *get_source_view                  (CodeSlayerNotebookSearch      *notebook_search);
static gboolean entry_keypress_action              (CodeSlayerNotebookSearch      *notebook_search,
                                                    GdkEventKey                   *event);
static void clear_all_search_marks                 (CodeSlayerNotebookSearch      *notebook_search);
static void replace_action                         (CodeSlayerNotebookSearch      *notebook_search);
static void replace_all_action                     (CodeSlayerNotebookSearch      *notebook_search);
static void highlight_all_action                   (CodeSlayerNotebookSearch      *notebook_search);
static gchar* entry_get_current_text               (GtkWidget                     *entry, 
                                                    GtkListStore                  *store);
static void set_find_entry_color                   (CodeSlayerNotebookSearch      *notebook_search);
static void entry_set_text                         (GtkWidget                     *entry,
                                                    GtkListStore                  *store,
                                                    gchar                         *text);
static void update_registry_action                 (CodeSlayerNotebookSearch      *notebook_search);

#define ENTRY_SIZE_REQUEST 350

#define CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_SEARCH_TYPE, CodeSlayerNotebookSearchPrivate))

typedef struct _CodeSlayerNotebookSearchPrivate CodeSlayerNotebookSearchPrivate;

struct _CodeSlayerNotebookSearchPrivate
{
  GtkWidget         *grid;
  GtkWidget         *notebook;
  CodeSlayerProfile *profile;
  GtkWidget         *close_button;
  GtkWidget         *find_label;
  GtkWidget         *find_entry;
  GtkListStore      *find_store;
  GtkWidget         *replace_spacer;
  GtkWidget         *replace_label;
  GtkWidget         *replace_entry;
  GtkListStore      *replace_store;
  GtkWidget         *replace_button;
  GtkWidget         *replace_all_button;
  GtkWidget         *find_previous_button;  
  GtkWidget         *find_next_button;  
  GtkWidget         *highlight_all_checkbox;
  GtkWidget         *match_case_checkbox;
  GtkWidget         *match_word_checkbox;
  GtkWidget         *regex_checkbox;
  GdkRGBA            error_color;
  GdkRGBA            default_color;
  gulong             find_entry_changed_id;
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
 * @profile: a #CodeSlayerProfile.
 *
 * Creates a new #CodeSlayerNotebookSearch.
 *
 * Returns: a new #CodeSlayerNotebookSearch. 
 */
GtkWidget*
codeslayer_notebook_search_new (GtkWidget         *notebook, 
                                CodeSlayerProfile *profile)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *notebook_search;
  
  notebook_search = g_object_new (codeslayer_notebook_search_get_type (), NULL);
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  priv->notebook = notebook;
  priv->profile = profile;
  
  priv->grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (priv->grid), 2);
  gtk_grid_set_column_spacing (GTK_GRID (priv->grid), 4);
  gtk_widget_set_margin_left (priv->grid, 4);
  
  add_close_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_find_entry (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_find_previous_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_find_next_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_match_case_checkbox (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_match_word_checkbox (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_highlight_all_checkbox (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_regex_checkbox (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_replace_entry (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_replace_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  add_replace_all_button (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  
  gtk_box_pack_start (GTK_BOX (notebook_search), priv->grid, FALSE, FALSE, 2);
  
  set_find_entry_color (CODESLAYER_NOTEBOOK_SEARCH (notebook_search));
  
  g_signal_connect_swapped (G_OBJECT (notebook), "sync-notebook",
                            G_CALLBACK (sync_notebook_action), notebook_search);
  
  return notebook_search;
}

/**
 * codeslayer_notebook_search_find:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 *
 * Search the source view highlighted text.
 */
void
codeslayer_notebook_search_find (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  GtkTextBuffer *buffer;
  GtkWidget *label;
  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  GtkTextIter start, end;
  gchar *current;

  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  gtk_widget_hide (priv->replace_spacer);
  gtk_widget_hide (priv->replace_label);
  gtk_widget_hide (priv->replace_entry);
  gtk_widget_hide (priv->replace_button);
  gtk_widget_hide (priv->replace_all_button);
  
  source_view = get_source_view (notebook_search);
  if (source_view == NULL)
    return;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  label = gtk_bin_get_child (GTK_BIN (priv->find_entry));

  insert_mark = gtk_text_buffer_get_insert (buffer);
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);
  
  /* re-select the range so the find next works consistently */
  gtk_text_iter_order (&start, &end);
  gtk_text_buffer_select_range (buffer, &start, &end);

  current = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  if (g_strcmp0 (current, "") != 0)
    {
      g_signal_handler_block (priv->find_entry, priv->find_entry_changed_id);
      entry_set_text (priv->find_entry, priv->find_store, current);
      g_signal_handler_unblock (priv->find_entry, priv->find_entry_changed_id);
      highlight_all_action (notebook_search);
    }
  else
    {
      gtk_widget_grab_focus (GTK_WIDGET (priv->find_entry));
    }
  
  gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->default_color));
  
  if (current != NULL)
    g_free (current);
}

/**
 * codeslayer_notebook_search_replace:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 *
 * Replace the source view highlighted text.
 */
void
codeslayer_notebook_search_replace (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  GtkTextBuffer *buffer;
  GtkWidget *label;
  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  GtkTextIter start, end;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  gtk_widget_show (priv->replace_spacer);
  gtk_widget_show (priv->replace_label);
  gtk_widget_show (priv->replace_entry);
  gtk_widget_show (priv->replace_button);
  gtk_widget_show (priv->replace_all_button);

  source_view = get_source_view (notebook_search);
  if (source_view == NULL)
    return;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
  label = gtk_bin_get_child (GTK_BIN (priv->find_entry));
    
  insert_mark = gtk_text_buffer_get_insert (buffer);
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);
  
  /* re-select the range so the find next works consistently */
  gtk_text_iter_order (&start, &end);
  gtk_text_buffer_select_range (buffer, &start, &end);

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox)))
    {
      gchar *current;
      current = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
      if (g_strcmp0 (current, "") != 0)
        {
          g_signal_handler_block (priv->find_entry, priv->find_entry_changed_id);
          entry_set_text (priv->find_entry, priv->find_store, current);
          g_signal_handler_unblock (priv->find_entry, priv->find_entry_changed_id);

          entry_set_text (priv->replace_entry, priv->replace_store, current);
          highlight_all_action (notebook_search);
        }
                  
      if (current != NULL)
        g_free (current);
    }

  gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->default_color));    
  gtk_widget_grab_focus (GTK_WIDGET (priv->replace_entry));
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
 * codeslayer_notebook_search_validate_matches:
 * @notebook_search: a #CodeSlayerNotebookSearch.
 * 
 * Highlight all the search values.
 */
void
codeslayer_notebook_search_verify_matches (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  gchar *find;
  GtkWidget *label;
  CodeSlayerSearch *search;
  gboolean match_case;
  gboolean match_word;
  gboolean regex;

  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  source_view = get_source_view (notebook_search);
  find = entry_get_current_text (priv->find_entry, priv->find_store);
  label = gtk_bin_get_child (GTK_BIN (priv->find_entry));
  
  search = codeslayer_source_view_get_search (CODESLAYER_SOURCE_VIEW (source_view));
  
  codeslayer_search_clear_highlight (search);
  
  if (g_strcmp0 (find, "") == 0)
    {
      if (find)
        g_free (find);
      return;
    }

  match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox));
  match_word = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox));
  regex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox));
  
  if (codeslayer_search_has_matches (search, find, match_case, match_word, regex))
    gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->default_color));
  else
    gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->error_color));

  highlight_all_action (notebook_search);
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

  find_label = gtk_label_new (_("Find"));
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
  gtk_widget_set_size_request (find_entry, ENTRY_SIZE_REQUEST, -1);                                                   
  
  priv->find_entry_changed_id = g_signal_connect_swapped (G_OBJECT (find_entry), "changed",
                                                          G_CALLBACK (find_action), notebook_search);
                    
  g_signal_connect_swapped (G_OBJECT (find_entry), "key-press-event",
                            G_CALLBACK (entry_keypress_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), find_entry, find_label, 
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
add_match_case_checkbox (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *match_case_checkbox;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  match_case_checkbox = gtk_check_button_new_with_label (_("Match Case"));
  gtk_widget_set_can_focus (match_case_checkbox, FALSE);
  priv->match_case_checkbox = match_case_checkbox;
  
  g_signal_connect_swapped (G_OBJECT (match_case_checkbox), "clicked",
                            G_CALLBACK (find_action), notebook_search);

  g_signal_connect_swapped (G_OBJECT (match_case_checkbox), "clicked",
                            G_CALLBACK (update_registry_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), match_case_checkbox, priv->highlight_all_checkbox, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_match_word_checkbox (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *match_word_checkbox;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  match_word_checkbox = gtk_check_button_new_with_label (_("Match Word"));
  gtk_widget_set_can_focus (match_word_checkbox, FALSE);
  priv->match_word_checkbox = match_word_checkbox;
  
  g_signal_connect_swapped (G_OBJECT (match_word_checkbox), "clicked",
                            G_CALLBACK (find_action), notebook_search);

  g_signal_connect_swapped (G_OBJECT (match_word_checkbox), "clicked",
                            G_CALLBACK (update_registry_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), match_word_checkbox, priv->match_case_checkbox, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_highlight_all_checkbox (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *highlight_all_checkbox;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  highlight_all_checkbox = gtk_check_button_new_with_label (_("Highlight All"));
  gtk_widget_set_can_focus (highlight_all_checkbox, FALSE);
  priv->highlight_all_checkbox = highlight_all_checkbox;
  
  g_signal_connect_swapped (G_OBJECT (highlight_all_checkbox), "clicked",
                            G_CALLBACK (highlight_all_action), notebook_search);

  g_signal_connect_swapped (G_OBJECT (highlight_all_checkbox), "clicked",
                            G_CALLBACK (update_registry_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), highlight_all_checkbox, priv->match_word_checkbox, 
                           GTK_POS_RIGHT, 1, 1);
}

static void
add_regex_checkbox (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *regex_checkbox;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  regex_checkbox = gtk_check_button_new_with_label (_("Regex"));
  gtk_widget_set_can_focus (regex_checkbox, FALSE);
  priv->regex_checkbox = regex_checkbox;
  
  g_signal_connect_swapped (G_OBJECT (regex_checkbox), "clicked",
                            G_CALLBACK (find_action), notebook_search);

  g_signal_connect_swapped (G_OBJECT (regex_checkbox), "clicked",
                            G_CALLBACK (update_registry_action), notebook_search);

  g_signal_connect_swapped (G_OBJECT (regex_checkbox), "clicked",
                            G_CALLBACK (sync_notebook_action), notebook_search);

  gtk_grid_attach_next_to (GTK_GRID (priv->grid), regex_checkbox, priv->highlight_all_checkbox, 
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
  gtk_widget_set_size_request (replace_entry, ENTRY_SIZE_REQUEST, -1);                                                   
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

static void
sync_notebook_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  CodeSlayerRegistry *registry;
  gboolean match_case_selected;
  gboolean match_word_selected;
  gboolean regex_selected;
  gboolean has_open_documents;

  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  has_open_documents = codeslayer_notebook_has_open_documents (CODESLAYER_NOTEBOOK (priv->notebook));
  
  gtk_widget_set_sensitive (priv->find_entry, has_open_documents);
  gtk_widget_set_sensitive (priv->replace_entry, has_open_documents);
  gtk_widget_set_sensitive (priv->find_next_button, has_open_documents);
  gtk_widget_set_sensitive (priv->find_previous_button, has_open_documents);
  gtk_widget_set_sensitive (priv->replace_button, has_open_documents);
  gtk_widget_set_sensitive (priv->replace_all_button, has_open_documents);
  gtk_widget_set_sensitive (priv->match_case_checkbox, has_open_documents);
  gtk_widget_set_sensitive (priv->match_word_checkbox, has_open_documents);
    
  match_case_selected = codeslayer_registry_get_boolean (registry,
                                                         CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_MATCH_CASE);
  match_word_selected = codeslayer_registry_get_boolean (registry,
                                                         CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_MATCH_WORD);
  regex_selected = codeslayer_registry_get_boolean (registry,
                                                    CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_REGEX);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox), match_case_selected);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox), match_word_selected);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox), regex_selected);
  
  if (regex_selected)
    {
      gtk_widget_set_sensitive (priv->match_case_checkbox, FALSE);
      gtk_widget_set_sensitive (priv->match_word_checkbox, FALSE);
    }
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
update_registry_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  CodeSlayerRegistry *registry;
  gboolean highlight_all_selected;
  gboolean match_case_selected;
  gboolean match_word_selected;
  gboolean regex_selected;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  highlight_all_selected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->highlight_all_checkbox));
  match_case_selected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox));
  match_word_selected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox));
  regex_selected = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox));
  
  codeslayer_registry_set_boolean (registry, 
                                   CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_HIGHLIGHT_ALL,
                                   highlight_all_selected);

  codeslayer_registry_set_boolean (registry, 
                                   CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_MATCH_CASE,
                                   match_case_selected);

  codeslayer_registry_set_boolean (registry, 
                                   CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_MATCH_WORD,
                                   match_word_selected);

  codeslayer_registry_set_boolean (registry, 
                                   CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_REGEX,
                                   regex_selected);
}

static void
find_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  GtkWidget *label;
  gchar *find;
  CodeSlayerSearch *search;
  gboolean highlight_all;
  gboolean match_case;
  gboolean match_word;
  gboolean regex;

  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  source_view = get_source_view (notebook_search);
  label = gtk_bin_get_child (GTK_BIN (priv->find_entry));
  find = entry_get_current_text (priv->find_entry, priv->find_store);
  
  search = codeslayer_source_view_get_search (CODESLAYER_SOURCE_VIEW (source_view));
  
  codeslayer_search_clear_highlight (search);
  
  if (g_strcmp0 (find, "") == 0)
    {
      GtkTextBuffer *buffer;
      GtkTextIter start;

      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));
      gtk_text_buffer_get_start_iter (buffer, &start);
      gtk_text_buffer_select_range (buffer, &start, &start);

      gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->default_color));

      if (find)
        g_free (find);

      return;
    }

  highlight_all = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->highlight_all_checkbox));
  match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox));
  match_word = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox));
  regex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox));
  
  if (codeslayer_search_find (search, find, match_case, match_word, regex))
    {
      if (highlight_all)
        codeslayer_search_highlight_all (search, find, match_case, match_word, regex);
      gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->default_color));
    }
  else
    {
      gtk_widget_override_color (label, GTK_STATE_FLAG_NORMAL, &(priv->error_color));    
    }

  if (find)
    g_free (find);
}

static void
find_next_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  gchar *find;
  CodeSlayerSearch *search;
  gboolean match_case;
  gboolean match_word;
  gboolean regex;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  source_view = get_source_view (notebook_search);
  find = entry_get_current_text (priv->find_entry, priv->find_store);
  
  search = codeslayer_source_view_get_search (CODESLAYER_SOURCE_VIEW (source_view));

  match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox));
  match_word = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox));
  regex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox));

  codeslayer_search_find_next (search, find, match_case, match_word, regex);
    
  g_signal_handler_block (priv->find_entry, priv->find_entry_changed_id);
  entry_set_text (priv->find_entry, priv->find_store, find);
  g_signal_handler_unblock (priv->find_entry, priv->find_entry_changed_id);
    
  if (find)
    g_free (find);    
}

static void
find_previous_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  gchar *find;
  CodeSlayerSearch *search;
  gboolean match_case;
  gboolean match_word;
  gboolean regex;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  source_view = get_source_view (notebook_search);
  find = entry_get_current_text (priv->find_entry, priv->find_store);
  
  search = codeslayer_source_view_get_search (CODESLAYER_SOURCE_VIEW (source_view));

  match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox));
  match_word = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox));
  regex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox));

  codeslayer_search_find_previous (search, find, match_case, match_word, regex);
    
  g_signal_handler_block (priv->find_entry, priv->find_entry_changed_id);
  entry_set_text (priv->find_entry, priv->find_store, find);
  g_signal_handler_unblock (priv->find_entry, priv->find_entry_changed_id);
    
  if (find)
    g_free (find);    
}

static void
replace_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  gchar *find;
  gchar *replace;
  CodeSlayerSearch *search;
  gboolean match_case;
  gboolean match_word;
  gboolean regex;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  source_view = get_source_view (notebook_search);

  find = entry_get_current_text (priv->find_entry, priv->find_store);
  replace = entry_get_current_text (priv->replace_entry, priv->replace_store);

  search = codeslayer_source_view_get_search (CODESLAYER_SOURCE_VIEW (source_view));

  match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox));
  match_word = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox));
  regex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox));

  codeslayer_search_replace (search, find, replace, match_case, match_word, regex);

  g_signal_handler_block (priv->find_entry, priv->find_entry_changed_id);
  entry_set_text (priv->find_entry, priv->find_store, find);    
  g_signal_handler_unblock (priv->find_entry, priv->find_entry_changed_id);

  entry_set_text (priv->replace_entry, priv->replace_store, replace);    

  if (find != NULL)
    g_free (find);
    
  if (replace != NULL)
    g_free (replace);    
}

static void
replace_all_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  gchar *find;
  gchar *replace;
  CodeSlayerSearch *search;
  gboolean match_case;
  gboolean match_word;
  gboolean regex;
  
  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  source_view = get_source_view (notebook_search);

  find = entry_get_current_text (priv->find_entry, priv->find_store);
  replace = entry_get_current_text (priv->replace_entry, priv->replace_store);

  search = codeslayer_source_view_get_search (CODESLAYER_SOURCE_VIEW (source_view));

  match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox));
  match_word = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox));
  regex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox));

  codeslayer_search_replace_all (search, find, replace, match_case, match_word, regex);

  g_signal_handler_block (priv->find_entry, priv->find_entry_changed_id);
  entry_set_text (priv->find_entry, priv->find_store, find);    
  g_signal_handler_unblock (priv->find_entry, priv->find_entry_changed_id);
   
  entry_set_text (priv->replace_entry, priv->replace_store, replace);    

  if (find != NULL)
    g_free (find);
    
  if (replace != NULL)
    g_free (replace);    
}

static void
highlight_all_action (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  GtkWidget *source_view;
  gchar *find;
  CodeSlayerSearch *search;
  gboolean highlight_all;
  gboolean match_case;
  gboolean match_word;
  gboolean regex;

  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);
  
  source_view = get_source_view (notebook_search);
  search = codeslayer_source_view_get_search (CODESLAYER_SOURCE_VIEW (source_view));

  codeslayer_search_clear_highlight (search);

  highlight_all = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->highlight_all_checkbox));
  if (!highlight_all)
    return;
  
  find = entry_get_current_text (priv->find_entry, priv->find_store);
  
  match_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_case_checkbox));
  match_word = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->match_word_checkbox));
  regex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->regex_checkbox));
  
  codeslayer_search_highlight_all (search, find, match_case, match_word, regex);
  
  if (find)
    g_free (find);
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
      GtkWidget *source_view;
      CodeSlayerSearch *search;
      
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      source_view = codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      search = codeslayer_source_view_get_search (CODESLAYER_SOURCE_VIEW (source_view));
      
      codeslayer_search_clear_highlight (search);
    }
}

static GtkWidget*
get_source_view (CodeSlayerNotebookSearch *notebook_search)
{
  CodeSlayerNotebookSearchPrivate *priv;
  gint page;
  GtkWidget *notebook_page;

  priv = CODESLAYER_NOTEBOOK_SEARCH_GET_PRIVATE (notebook_search);

  page = gtk_notebook_get_current_page (GTK_NOTEBOOK (priv->notebook));
  
  notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
  if (notebook_page == NULL)
    return NULL;

  return codeslayer_notebook_page_get_source_view (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
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
