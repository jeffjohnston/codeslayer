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
 
#include <codeslayer/codeslayer-regexview.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-regexview
 * @short_description: The view that contains the additional regular expression tools.
 * @title: CodeSlayerRegexView
 * @include: codeslayer/codeslayer-regexview.h
 */
 
typedef struct
{
  gchar *find;
  gchar *replace;
  gchar **groups;
} Match;

static void codeslayer_regex_view_class_init  (CodeSlayerRegexViewClass *klass);
static void codeslayer_regex_view_init        (CodeSlayerRegexView      *regex_view);
static void codeslayer_regex_view_finalize    (CodeSlayerRegexView      *regex_view);

static void search_changed_action             (CodeSlayerRegexView      *search, 
                                               gchar                    *find, 
                                               gchar                    *replace, 
                                               gboolean                  match_case, 
                                               gboolean                  match_word, 
                                               gboolean                  regular_expression);
static void add_buttons                       (CodeSlayerRegexView      *regex_view);                                               
static void add_paned                         (CodeSlayerRegexView      *regex_view);
static void process_action                    (CodeSlayerRegexView      *regex_view);
static void process_action                    (CodeSlayerRegexView      *regex_view);
static GtkWidget* get_active_source_view      (CodeSlayerRegexView      *regex_view);
static GList* get_matches                     (CodeSlayerRegexView      *regex_view);
static void free_match                        (Match                    *match);
static void add_paned1                        (CodeSlayerRegexView      *regex_view);
static void add_paned2                        (CodeSlayerRegexView      *regex_view);
static void populate_find_replace_view        (CodeSlayerRegexView      *regex_view, 
                                               GList                    *matches);
static void populate_groups_view              (CodeSlayerRegexView      *regex_view, 
                                               GList                    *matches);

#define CODESLAYER_REGEX_VIEW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_REGEX_VIEW_TYPE, CodeSlayerRegexViewPrivate))

typedef struct _CodeSlayerRegexViewPrivate CodeSlayerRegexViewPrivate;

struct _CodeSlayerRegexViewPrivate
{
  GtkWidget         *notebook_search;
  GtkWidget         *notebook;
  CodeSlayerProfile *profile;
  gchar             *find;
  gchar             *replace;
  GtkWidget         *paned;  
  GtkWidget         *find_replace_text_view;
  GtkWidget         *groups_text_view;
  GtkWidget         *distinct_checkbox;
  GtkWidget         *sort_checkbox;
  GtkWidget         *auto_sync_checkbox;  
  gulong             search_changed_id;
};

enum
{
  FIND = 0, 
  REPLACE,
  COLUMNS
};

G_DEFINE_TYPE (CodeSlayerRegexView, codeslayer_regex_view, GTK_TYPE_VBOX)

static void
codeslayer_regex_view_class_init (CodeSlayerRegexViewClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_regex_view_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerRegexViewPrivate));
}

static void
codeslayer_regex_view_init (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);
  gtk_container_set_border_width (GTK_CONTAINER (regex_view), 4);
  priv->find = NULL;
  priv->replace = NULL;
}

static void
codeslayer_regex_view_finalize (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  if (priv->find != NULL)
    g_free (priv->find);

  if (priv->replace != NULL)
    g_free (priv->replace);
    
  g_signal_handler_disconnect (priv->notebook_search, priv->search_changed_id);
    
  G_OBJECT_CLASS (codeslayer_regex_view_parent_class)->finalize (G_OBJECT (regex_view));
}

GtkWidget*
codeslayer_regex_view_new (GtkWidget         *notebook_search, 
                           GtkWidget         *notebook,
                           CodeSlayerProfile *profile)
{
  CodeSlayerRegexViewPrivate *priv;
  GtkWidget *regex_view;  
  
  regex_view = g_object_new (codeslayer_regex_view_get_type (), NULL);
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);
  
  priv->notebook_search = notebook_search;
  priv->notebook = notebook;
  priv->profile = profile;
  
  priv->search_changed_id = g_signal_connect_swapped (G_OBJECT (notebook_search), "search-changed",
                                                      G_CALLBACK (search_changed_action), regex_view);
                            
  add_buttons (CODESLAYER_REGEX_VIEW (regex_view));
  add_paned (CODESLAYER_REGEX_VIEW (regex_view));

  return regex_view;
}

static void
add_buttons (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  GtkWidget *hbox;
  GtkWidget *execute_button;
  GtkWidget *execute_image;
  GtkWidget *distinct_checkbox;
  GtkWidget *sort_checkbox;
  GtkWidget *auto_sync_checkbox;
  
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);

  execute_button = gtk_button_new ();
  gtk_widget_set_tooltip_text (execute_button, "Execute");

  gtk_button_set_relief (GTK_BUTTON (execute_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (execute_button), FALSE);
  execute_image = gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (execute_button), execute_image);
  gtk_widget_set_can_focus (execute_button, FALSE);

  gtk_box_pack_start (GTK_BOX (hbox), execute_button, FALSE, FALSE, 0);  

  distinct_checkbox = gtk_check_button_new_with_label (_("Distinct"));
  priv->distinct_checkbox = distinct_checkbox;
  gtk_box_pack_start (GTK_BOX (hbox), distinct_checkbox, FALSE, FALSE, 0);  

  sort_checkbox = gtk_check_button_new_with_label (_("Sort"));
  priv->sort_checkbox = sort_checkbox;
  gtk_box_pack_start (GTK_BOX (hbox), sort_checkbox, FALSE, FALSE, 0);  

  auto_sync_checkbox = gtk_check_button_new_with_label (_("Auto Sync"));
  priv->auto_sync_checkbox = auto_sync_checkbox;
  gtk_box_pack_start (GTK_BOX (hbox), auto_sync_checkbox, FALSE, FALSE, 0);  

  gtk_box_pack_start (GTK_BOX (regex_view), hbox, FALSE, FALSE, 0);
  
  g_signal_connect_swapped (G_OBJECT (execute_button), "clicked",
                            G_CALLBACK (process_action), regex_view);
}

static void
add_paned (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  GtkWidget *paned;

  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);
  
  paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  priv->paned = paned;

  add_paned1 (regex_view);
  add_paned2 (regex_view);

  gtk_box_pack_start (GTK_BOX (regex_view), paned, TRUE, TRUE, 2);
  
  gtk_paned_set_position (GTK_PANED (priv->paned), 700);
}

static void
add_paned1 (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  GtkWidget *scrolled_window;
  GtkWidget *text_view;
  
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  text_view = gtk_text_view_new ();
  priv->find_replace_text_view = text_view;
  
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (text_view));

  gtk_paned_add1 (GTK_PANED (priv->paned), scrolled_window);
}

static void
add_paned2 (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  GtkWidget *scrolled_window;
  GtkWidget *text_view;

  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  text_view = gtk_text_view_new ();
  priv->groups_text_view = text_view;
  
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (text_view));

  gtk_paned_add2 (GTK_PANED (priv->paned), scrolled_window);
}

static void
search_changed_action (CodeSlayerRegexView *regex_view, 
                       gchar               *find, 
                       gchar               *replace, 
                       gboolean             match_case, 
                       gboolean             match_word, 
                       gboolean             regular_expression)
{
  CodeSlayerRegexViewPrivate *priv;
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);
  
  if (priv->find == NULL)
    {
      g_free (priv->find);
      priv->find = NULL;    
    }

  if (priv->replace == NULL)
    {
      g_free (priv->replace);
      priv->replace = NULL;    
    }
    
  if (regular_expression == TRUE)
    {
      priv->find = g_strdup (find);
      priv->replace = g_strdup (replace);
    }
    
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->auto_sync_checkbox)))
    process_action (regex_view);
}

static void
process_action (CodeSlayerRegexView *regex_view)
{
  GList *matches;
  
  matches = get_matches (regex_view);

  if (matches == NULL)
    return;

  populate_find_replace_view (regex_view, matches);    
  populate_groups_view (regex_view, matches);
    
  g_list_foreach (matches, (GFunc) free_match, NULL);
  g_list_free (matches);
}

static void 
free_match (Match *match)
{
  g_free (match->find);
  
  if (match->replace != NULL)
    g_free (match->replace);
    
  if (match->groups != NULL)
    g_strfreev (match->groups);
  
  g_free (match);
}

static void
populate_find_replace_view (CodeSlayerRegexView *regex_view, 
                            GList               *matches)
{
  CodeSlayerRegexViewPrivate *priv;
  GList *list;
  GtkTextBuffer *buffer;
  GString *string;
  gchar *text;

  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);
  
  string = g_string_new ("");
  
  list = matches;
    
  while (list != NULL)
    {
      Match *match = list->data;
      
      if (match->replace != NULL)
        {
          string = g_string_append (string, match->replace);
        }
      else
        {
          string = g_string_append (string, match->find);
          string = g_string_append (string, "\n");
        }
      
      list = g_list_next (list);
    }

  text = g_string_free (string, FALSE);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->find_replace_text_view));
  gtk_text_buffer_set_text (buffer, text, -1);

  if (text)
    g_free (text);
}

static void
populate_groups_view (CodeSlayerRegexView *regex_view, 
                      GList               *matches)
{
  CodeSlayerRegexViewPrivate *priv;
  GList *list;
  GtkTextBuffer *buffer;
  GString *string;
  gchar *text;
  gint count = 1;

  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);
  
  string = g_string_new ("");
  
  list = matches;
    
  while (list != NULL)
    {
      Match *match = list->data;
      gchar **groups = match->groups;
      
      if (groups != NULL)
        {
          gchar **array = groups;
          gint i = 0;
          
          for (; *array != NULL; i++)
            {
              g_string_append_printf (string, "%d.%d: ( %s )\n", count, i, *array);
              array++;
            }
        }
      
      count++;
      list = g_list_next (list);
    }

  text = g_string_free (string, FALSE);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->groups_text_view));
  gtk_text_buffer_set_text (buffer, text, -1);

  if (text)
    g_free (text);
}

static GList*
get_matches (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  GtkWidget *source_view;
  GtkTextBuffer *source_buffer;
  GtkTextIter start, end;
  gchar *source_text;
  GRegex *regex;
  GMatchInfo *match_info = NULL;
  GList *results = NULL;

  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  if (!codeslayer_utils_has_text (priv->find))
    return NULL;
    
  regex = g_regex_new (priv->find, 0, 0, NULL);

  if (regex == NULL)
    return NULL;
    
  source_view = get_active_source_view (regex_view);
  source_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));

  gtk_text_buffer_get_bounds (source_buffer, &start, &end);
  source_text = gtk_text_buffer_get_text (source_buffer, &start, &end, FALSE);
  
  g_regex_match (regex, source_text, 0, &match_info);
  while (g_match_info_matches (match_info))
    {
      gchar *find_match = g_match_info_fetch (match_info, 0);
      Match *match = g_malloc (sizeof (Match));
      match->find = find_match;
      match->replace = NULL;
      match->groups = g_match_info_fetch_all (match_info);
      
      if (codeslayer_utils_has_text (priv->replace))
        {
          gchar *replace_match;
          replace_match = g_regex_replace (regex, find_match, -1, 0, 
                                           priv->replace, 0, NULL);
          match->replace = replace_match;
        }

      results = g_list_prepend (results, match);
      g_match_info_next (match_info, NULL);
    }
    
  if (source_text)
    g_free (source_text);
    
  results = g_list_reverse (results);
  return results;
}

static GtkWidget*
get_active_source_view (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 0)
    return NULL;

  return codeslayer_notebook_get_active_source_view (CODESLAYER_NOTEBOOK (priv->notebook));
}
