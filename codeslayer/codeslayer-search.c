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
#include <codeslayer/codeslayer-search.h>
#include <codeslayer/codeslayer-sourceview.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-notebook-search
 * @short_description: The notebook inline search.
 * @title: CodeSlayerSearch
 * @include: codeslayer/codeslayer-notebook-search.h
 */

static void codeslayer_search_class_init  (CodeSlayerSearchClass *klass);
static void codeslayer_search_init        (CodeSlayerSearch      *search);
static void codeslayer_search_finalize    (CodeSlayerSearch      *search);

static gboolean forward_search            (CodeSlayerSearch      *search, 
                                           const gchar           *find, 
                                           GtkTextIter           *start, 
                                           GtkTextIter           *begin, 
                                           GtkTextIter           *end, 
                                           gboolean               match_case, 
                                           gboolean               match_word, 
                                           gboolean               regular_expression);
static gboolean backward_search           (CodeSlayerSearch      *search, 
                                           const gchar           *find, 
                                           GtkTextIter           *start, 
                                           GtkTextIter           *begin, 
                                           GtkTextIter           *end, 
                                           gboolean               match_case, 
                                           gboolean               match_word,
                                           gboolean               regular_expression);
static gchar* forward_regex_match         (CodeSlayerSearch      *search, 
                                           const gchar           *find, 
                                           GtkTextIter           *start);
static gchar* backward_regex_match        (CodeSlayerSearch      *search, 
                                           const gchar           *find, 
                                           GtkTextIter           *start);
static gboolean find_in_view              (GtkWidget             *source_view, 
                                           GdkRectangle           rect, 
                                           GtkTextIter            begin, 
                                           GtkTextIter            end);

#define FIND "Find:"
#define FIND_INCREMENTAL "Find [I]:"

#define CODESLAYER_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_SEARCH_TYPE, CodeSlayerSearchPrivate))

typedef struct _CodeSlayerSearchPrivate CodeSlayerSearchPrivate;

struct _CodeSlayerSearchPrivate
{
  CodeSlayerSourceView *source_view;
  GdkRGBA               error_color;
  GdkRGBA               default_color;
};

G_DEFINE_TYPE (CodeSlayerSearch, codeslayer_search, G_TYPE_OBJECT)

static void
codeslayer_search_class_init (CodeSlayerSearchClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_search_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerSearchPrivate));
}

static void
codeslayer_search_init (CodeSlayerSearch *search)
{
}

static void
codeslayer_search_finalize (CodeSlayerSearch *search)
{
  G_OBJECT_CLASS (codeslayer_search_parent_class)->finalize (G_OBJECT (search));
}

/**
 * codeslayer_search_new:
 * @source_view: a #CodeSlayerSourceView.
 *
 * Creates a new #CodeSlayerSearch.
 *
 * Returns: a new #CodeSlayerSearch. 
 */
CodeSlayerSearch*
codeslayer_search_new (GObject *source_view)
{
  CodeSlayerSearchPrivate *priv;
  CodeSlayerSearch *search;
  
  search = g_object_new (codeslayer_search_get_type (), NULL);
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  priv->source_view = CODESLAYER_SOURCE_VIEW (source_view);

  return search;
}

/**
 * codeslayer_search_has_matches:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 * @regular_expression: is true if should use regular expression
 * 
 * Returns: is TRUE if matches were found. 
 */
gboolean
codeslayer_search_has_matches (CodeSlayerSearch *search,
                               gchar            *find,
                               gboolean          match_case,
                               gboolean          match_word,
                               gboolean          regular_expression)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  if (g_strcmp0 (find, "") == 0)
    return FALSE;

  gtk_text_buffer_get_start_iter (buffer, &start);

  return forward_search (search, find, &start, &begin, &end, match_case, match_word, regular_expression);
}

/**
 * codeslayer_search_find:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 * @regular_expression: is true if should use regular expression
 * 
 * Find the first search value. Will start looking in the current view. If
 * a match is not found then start looking at the start of the buffer.
 * 
 * Returns: is TRUE if matches were found. 
 */
gboolean
codeslayer_search_find (CodeSlayerSearch *search,
                        gchar            *find,
                        gboolean          match_case,
                        gboolean          match_word,
                        gboolean          regular_expression)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  GdkRectangle rect;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  if (g_strcmp0 (find, "") == 0)
    {
      gtk_text_buffer_get_start_iter (buffer, &start);
      gtk_text_buffer_select_range (buffer, &start, &start);
      return FALSE;
    }

  gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (priv->source_view), &rect);
  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (priv->source_view), 
                                      &start, rect.x, rect.y);

  if (!forward_search (search, find, &start, &begin, &end, match_case, match_word, regular_expression))
    {
      gtk_text_buffer_get_start_iter (buffer, &start);
      if (!forward_search (search, find, &start, &begin, &end, match_case, match_word, regular_expression))
        return FALSE;
    }

  if (!find_in_view (GTK_WIDGET (priv->source_view), rect, begin, end))
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (priv->source_view), 
                                    &begin, .1, FALSE, 0, 0);
    }

  gtk_text_buffer_select_range (buffer, &begin, &end);

  return TRUE;
}

/**
 * codeslayer_search_find_next:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 * @regular_expression: is true if should use regular expression
 *
 * Find the next search value.
 */
void
codeslayer_search_find_next (CodeSlayerSearch *search, 
                             gchar            *find, 
                             gboolean          match_case, 
                             gboolean          match_word, 
                             gboolean          regular_expression)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  GtkTextMark *insert_mark;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  insert_mark = gtk_text_buffer_get_selection_bound (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  
  if (forward_search (search, find, &start, &begin, &end, match_case, match_word, regular_expression))
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (priv->source_view), 
                                    &begin, .1, FALSE, 0, 0);
      gtk_text_buffer_select_range (buffer, &begin, &end);
    }
  else
    {
      gtk_text_buffer_get_start_iter (buffer, &start);
      if (forward_search (search, find, &start, &begin, &end, match_case, match_word, regular_expression))
        {
          gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (priv->source_view), 
                                        &begin, .1, FALSE, 0, 0);
          gtk_text_buffer_select_range (buffer, &begin, &end);
        }
    }
}

/**
 * codeslayer_search_find_previous:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 * @regular_expression: is true if should use regular expression
 *
 * Find the previous search value.
 */
void
codeslayer_search_find_previous (CodeSlayerSearch *search, 
                                 gchar            *find, 
                                 gboolean          match_case, 
                                 gboolean          match_word, 
                                 gboolean          regular_expression)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  GtkTextMark *insert_mark;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  insert_mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);

  if (backward_search (search, find, &start, &begin, &end, match_case, match_word, regular_expression))
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (priv->source_view), 
                                    &begin, .1, FALSE, 0, 0);
      gtk_text_buffer_select_range (buffer, &begin, &end);
    }
  else
    {
      gtk_text_buffer_get_end_iter (buffer, &start);
      if (backward_search (search, find, &start, &begin, &end, match_case, match_word, regular_expression))
        {
          gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (priv->source_view), 
                                        &begin, .1, FALSE, 0, 0);
          gtk_text_buffer_select_range (buffer, &begin, &end);
        }
    }
}

/**
 * codeslayer_search_replace:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @replace: the text to replace
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 * @regular_expression: is true if should use regular expression
 *
 * Replace the source view highlighted text.
 */
void
codeslayer_search_replace (CodeSlayerSearch *search, 
                           gchar            *find, 
                           gchar            *replace, 
                           gboolean          match_case, 
                           gboolean          match_word, 
                           gboolean          regular_expression)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  gchar *current;
  GtkTextIter start, end;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  insert_mark = gtk_text_buffer_get_insert (buffer);
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  /* make sure that the highlighted text is in the current search */
  current = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  if (regular_expression)
    {
      GRegex *regex;
      gchar *text;

      regex = g_regex_new (find, 0, 0, NULL);
      text =  g_regex_replace (regex, current, -1, 0, replace, 0, NULL);
      
      if (text != NULL)
        {
          gtk_text_buffer_begin_user_action (buffer);
          gtk_text_buffer_delete (buffer, &start, &end);
          gtk_text_buffer_insert (buffer, &start, text, -1);
          gtk_text_buffer_end_user_action (buffer);
          g_free (text);        
        }

      g_regex_unref (regex);
    }
  else
    {
      gboolean is_search_same = FALSE;

      if (match_case)
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
    }
    
  if (current != NULL)
    g_free (current);    

  codeslayer_search_find_next (search, find, match_case, match_word, regular_expression);
}

/**
 * codeslayer_search_replace_all:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @replace: the text to replace
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 * @regular_expression: is true if should use regular expression
 *
 * Replace all the source view highlighted text.
 */
void
codeslayer_search_replace_all (CodeSlayerSearch *search, 
                               gchar            *find, 
                               gchar            *replace, 
                               gboolean          match_case, 
                               gboolean          match_word, 
                               gboolean          regular_expression)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  gtk_text_buffer_get_start_iter (buffer, &start);

  gtk_text_buffer_begin_user_action (buffer);

  while (forward_search (search, find, &start, &begin, &end, match_case, match_word, regular_expression))
    {
      if (regular_expression)
        {
          GRegex *regex;
          gchar *current;
          gchar *text;

          regex = g_regex_new (find, 0, 0, NULL);
          current = gtk_text_buffer_get_text (buffer, &begin, &end, FALSE);
          text =  g_regex_replace (regex, current, -1, 0, replace, 0, NULL);
          
          if (text != NULL)
            {
              gtk_text_buffer_delete (buffer, &begin, &end);
              gtk_text_buffer_insert (buffer, &begin, text, -1);
              g_free (text);
            }

          g_regex_unref (regex);
          
          if (current != NULL)
            g_free (current);
          
          start = begin;
        }
      else
        {
          gtk_text_buffer_delete (buffer, &begin, &end);
          gtk_text_buffer_insert (buffer, &begin, replace, -1);
          start = begin;
        }
    }

  gtk_text_buffer_end_user_action (buffer);
}

/**
 * codeslayer_search_highlight_all:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 * @regular_expression: is true if should use regular expression
 * @search_time: the amount of time, in seconds, that the search will run before timing out
 * @search_timed_out: is TRUE if went past the search time [OUT].
 * 
 * Create the search marks based on the current find entry.
 * 
 * Returns: is TRUE if matches were found. 
 */
gboolean
codeslayer_search_highlight_all (CodeSlayerSearch *search, 
                                 gchar            *find,
                                 gboolean          match_case, 
                                 gboolean          match_word, 
                                 gboolean          regular_expression, 
                                 gdouble           search_time, 
                                 gboolean         *search_timed_out)
{

  CodeSlayerSearchPrivate *priv;
  gboolean success;
  GtkTextBuffer *buffer;
  GtkTextIter start, first, begin, end;
  gchar *text;
  GRegex *regex;
  GMatchInfo *match_info = NULL;
  GHashTable *hashtable;
  GList *list;
  GTimer *timer;
  GList *keys;
  gdouble elapsed = 0;
  gboolean time_expired = FALSE;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  
  if (search_timed_out != NULL)
    *search_timed_out = FALSE;
  
  if (g_strcmp0 (find, "") == 0)
    return FALSE;
    
  if (regular_expression && (g_strcmp0 (find, ".") == 0 || g_strcmp0 (find, ".*") == 0))
    return TRUE;

  regex = g_regex_new (find, 0, 0, NULL);

  if (regex == NULL)
    return FALSE;
    
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  hashtable = g_hash_table_new_full ((GHashFunc)g_str_hash, (GEqualFunc)g_str_equal, 
                                     (GDestroyNotify)g_free, NULL);
  
  timer = g_timer_new ();
  g_timer_start (timer);
  
  g_regex_match (regex, text, 0, &match_info);
  while (g_match_info_matches (match_info))
    {
      gchar *key = g_match_info_fetch (match_info, 0);
      if (!g_hash_table_contains (hashtable, key))
        g_hash_table_insert (hashtable, key, NULL);
      else
        g_free (key);
                
      g_match_info_next (match_info, NULL);
    }
    
  keys = g_hash_table_get_keys (hashtable);
  list = keys;
  
  while (list != NULL)
  {
    gchar *match = list->data;

    first = start;
    success = forward_search (search, match, &first, &begin, &end, TRUE, FALSE, FALSE);
    while (success)
      {
        elapsed = g_timer_elapsed (timer, NULL);
        if (elapsed >= search_time)
          {
            time_expired = TRUE;
            success = FALSE;
            break;
          }
        
        gtk_text_buffer_apply_tag_by_name (buffer, "search-marks", &begin, &end);
        gtk_text_iter_forward_char (&first);
        success = forward_search (search, match, &first, &begin, &end, TRUE, FALSE, FALSE);
        first = begin;
      }

    elapsed = g_timer_elapsed (timer, NULL);
    if (elapsed >= search_time)
      {
        time_expired = TRUE;
        break;
      }

    list = g_list_next (list);
  }
  
  g_regex_unref (regex);
  g_match_info_free (match_info);
  g_free (text);
  
  if (keys)
    g_list_free (keys);
    
  g_hash_table_destroy (hashtable);
  g_timer_destroy (timer);
  
  if (time_expired)
    {
      codeslayer_search_clear_highlight (search);
    
      if (search_timed_out != NULL)
        *search_timed_out = TRUE;
    }

  return success;    
}

/**
 * codeslayer_search_clear_highlight:
 * @search: a #CodeSlayerSearch.
 */
void
codeslayer_search_clear_highlight (CodeSlayerSearch *search)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, end;

  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "search-marks", &start, &end);
}

static gboolean
find_in_view (GtkWidget    *source_view, 
              GdkRectangle  rect, 
              GtkTextIter   begin, 
              GtkTextIter   end) 
{
  GtkTextIter start_visible;
  GtkTextIter end_visible;

  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (source_view), &start_visible,
                                      rect.x, rect.y);
                                      
  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (source_view), &end_visible,
                                      rect.x + rect.width, rect.y + rect.height);
                                      
  if (gtk_text_iter_in_range (&begin, &start_visible, &end_visible) ||
      gtk_text_iter_in_range (&end, &start_visible, &end_visible))
    {
      return TRUE;
    }

  return FALSE;
}

static gboolean
forward_search (CodeSlayerSearch *search, 
                const gchar      *find, 
                GtkTextIter      *start, 
                GtkTextIter      *begin,
                GtkTextIter      *end, 
                gboolean          match_case, 
                gboolean          match_word, 
                gboolean          regular_expression)
{
  gboolean result = FALSE;
  
  if (regular_expression)
    {
      gchar *match = forward_regex_match (search, find, start);
      if (match != NULL)
        {
          result = gtk_text_iter_forward_search (start, match, 0, begin, end, NULL);
          g_free (match);
        }
    }
  else
    {
      if (match_case)
          result = gtk_text_iter_forward_search (start, find, 0, begin, end, NULL);
      else
          result = gtk_text_iter_forward_search (start, find,
                                                 GTK_TEXT_SEARCH_CASE_INSENSITIVE,
                                                 begin, end, NULL);

      if (result && match_word)
        {
          if (!(gtk_text_iter_starts_word (begin) && gtk_text_iter_ends_word (end)))
            {
              *start = *begin;
              gtk_text_iter_forward_char (start);
              result = forward_search (search, find, start, begin, end, match_case, match_word, regular_expression);
            }
        }
    }

  return result;
}

static gboolean
backward_search (CodeSlayerSearch *search, 
                 const gchar      *find, 
                 GtkTextIter      *start, 
                 GtkTextIter      *begin,
                 GtkTextIter      *end, 
                 gboolean          match_case, 
                 gboolean          match_word, 
                 gboolean          regular_expression)
{
  gboolean result = FALSE;
  
  if (regular_expression)
    {
      gchar *match = backward_regex_match (search, find, start);
      if (match != NULL)
        {
          result = gtk_text_iter_backward_search (start, match, 0, begin, end, NULL);
          g_free (match);
        }
    }
  else
    {
      if (match_case)
          result = gtk_text_iter_backward_search (start, find, 0, begin, end, NULL);
      else
          result = gtk_text_iter_backward_search (start, find,
                                                  GTK_TEXT_SEARCH_CASE_INSENSITIVE,
                                                  begin, end, NULL);

      if (result && match_word)
        {
          if (!(gtk_text_iter_starts_word (begin) && 
                 gtk_text_iter_ends_word (end)))
            {
              *start = *begin;
              result = backward_search (search, find, start, begin, end, match_case, match_word, regular_expression);
            }
        }
    }
    
  return result;
}

static gchar*
forward_regex_match (CodeSlayerSearch *search, 
                     const gchar      *find, 
                     GtkTextIter      *start)
{
  CodeSlayerSearchPrivate *priv;
  gchar *result = NULL;
  GtkTextIter end;  
  GtkTextBuffer *buffer;
  gchar *text;
  GRegex *regex;
  GMatchInfo *match_info = NULL;

  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  regex = g_regex_new (find, 0, 0, NULL);

  if (regex == NULL)
    return NULL;
    
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));
  gtk_text_buffer_get_end_iter (buffer, &end);
  text = gtk_text_buffer_get_text (buffer, start, &end, FALSE);
  
  g_regex_match (regex, text, 0, &match_info);
  if (g_match_info_matches (match_info))
    result = g_match_info_fetch (match_info, 0);
    
  g_regex_unref (regex);
  g_match_info_free (match_info);
  g_free (text);
  
  return result;
}

static gchar*
backward_regex_match (CodeSlayerSearch *search, 
                      const gchar      *find, 
                      GtkTextIter      *end)
{
  CodeSlayerSearchPrivate *priv;
  gchar *result = NULL;
  GtkTextIter start;  
  GtkTextBuffer *buffer;
  gchar *text;
  GRegex *regex;
  GMatchInfo *match_info = NULL;

  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  regex = g_regex_new (find, 0, 0, NULL);
  
  if (regex == NULL)
    return NULL;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));
  gtk_text_buffer_get_start_iter (buffer, &start);
  text = gtk_text_buffer_get_text (buffer, &start, end, FALSE);
  
  g_regex_match (regex, text, 0, &match_info);
  while (g_match_info_matches (match_info))
    {
      if (result != NULL)
        g_free (result);
      result = g_match_info_fetch (match_info, 0);
      g_match_info_next (match_info, NULL);
    }
  
  g_regex_unref (regex);    
  g_match_info_free (match_info);
  g_free (text);
    
  return result;
}
