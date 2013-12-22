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
                                           gboolean               match_word);
static gboolean backward_search           (CodeSlayerSearch      *search, 
                                           const gchar           *find, 
                                           GtkTextIter           *start, 
                                           GtkTextIter           *begin, 
                                           GtkTextIter           *end, 
                                           gboolean               match_case, 
                                           gboolean               match_word);
static gboolean search_marks_in_view      (GtkWidget             *source_view, 
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
 * codeslayer_search_find_next:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 *
 * Find the next search value.
 */
void
codeslayer_search_find_next (CodeSlayerSearch *search, 
                             gchar            *find, 
                             gboolean          match_case, 
                             gboolean          match_word)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  GtkTextMark *insert_mark;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  insert_mark = gtk_text_buffer_get_selection_bound (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);

  if (forward_search (search, find, &start, &begin, &end, match_case, match_word))
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (priv->source_view), 
                                    &begin, .1, FALSE, 0, 0);
      gtk_text_buffer_select_range (buffer, &begin, &end);
    }
  else
    {
      gtk_text_buffer_get_start_iter (buffer, &start);
      if (forward_search (search, find, &start, &begin, &end, match_case, match_word))
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
 *
 * Find the previous search value.
 */
void
codeslayer_search_find_previous (CodeSlayerSearch *search, 
                                 gchar            *find, 
                                 gboolean          match_case, 
                                 gboolean          match_word)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  GtkTextMark *insert_mark;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  insert_mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);

  if (backward_search (search, find, &start, &begin, &end, match_case, match_word))
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (priv->source_view), 
                                    &begin, .1, FALSE, 0, 0);
      gtk_text_buffer_select_range (buffer, &begin, &end);
    }
  else
    {
      gtk_text_buffer_get_end_iter (buffer, &start);
      if (backward_search (search, find, &start, &begin, &end, match_case, match_word))
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
 * @find: the text to replace
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 *
 * Replace the source view highlighted text.
 */
void
codeslayer_search_replace (CodeSlayerSearch *search, 
                           gchar            *find, 
                           gchar            *replace, 
                           gboolean          match_case, 
                           gboolean          match_word)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  gchar *current;
  GtkTextIter start, end;
  gboolean is_search_same;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  insert_mark = gtk_text_buffer_get_insert (buffer);
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  /* make sure that the highlighted text is in the current search */
  current = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  is_search_same = FALSE;

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
    
  if (current != NULL)
    g_free (current);    

  codeslayer_search_find_next (search, find, match_case, match_word);
}

/**
 * codeslayer_search_replace_all:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @find: the text to replace
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 *
 * Replace all the source view highlighted text.
 */
void
codeslayer_search_replace_all (CodeSlayerSearch *search, 
                               gchar            *find, 
                               gchar            *replace, 
                               gboolean          match_case, 
                               gboolean          match_word)
{
  CodeSlayerSearchPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start, begin, end;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  gtk_text_buffer_get_start_iter (buffer, &start);

  gtk_text_buffer_begin_user_action (buffer);

  while (forward_search (search, find, &start, &begin, &end, match_case, match_word))
    {
      gtk_text_buffer_delete (buffer, &begin, &end);
      gtk_text_buffer_insert (buffer, &begin, replace, -1);
      start = begin;
    }

  gtk_text_buffer_end_user_action (buffer);
}

/**
 * codeslayer_search_create_search_marks:
 * @search: a #CodeSlayerSearch.
 * @find: the text to find
 * @match_case: is true if should match case
 * @match_word: is true if should match word
 * @scrollable: is TRUE if should scroll after finding marks.
 * 
 * Create the search marks based on the current registry.
 * 
 * Returns: is TRUE if matches were found. 
 */
gboolean
codeslayer_search_create_search_marks (CodeSlayerSearch *search,
                                       gchar            *find,
                                       gboolean          match_case, 
                                       gboolean          match_word,                                       
                                       gboolean          scrollable)
{
  CodeSlayerSearchPrivate *priv;
  GdkRectangle rect;
  GtkTextBuffer *buffer;
  GtkTextIter start, first, begin, end;
  gboolean success;
  gboolean need_to_scroll = FALSE;
  
  priv = CODESLAYER_SEARCH_GET_PRIVATE (search);
  
  codeslayer_search_clear_search_marks (search);

  if (g_strcmp0 (find, "") == 0)
    return FALSE;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->source_view));

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (priv->source_view), &rect);

  success = forward_search (search, find, &start, &begin, &end, match_case, match_word);
  
  if (!success)
    return FALSE;

  need_to_scroll = !search_marks_in_view (GTK_WIDGET (priv->source_view), rect, begin, end);
  first = begin;
  
  while (success)
    {
      if (need_to_scroll)
        need_to_scroll = !search_marks_in_view (GTK_WIDGET (priv->source_view), rect, begin, end);
    
      gtk_text_buffer_apply_tag_by_name (buffer, "search-marks", &begin, &end);
      gtk_text_iter_forward_char (&start);
      success = forward_search (search, find, &start, &begin, &end, match_case, match_word);
      start = begin;
    }

  if (need_to_scroll && scrollable)
    {
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (priv->source_view), &first, .1, FALSE, 0, 0);
      gtk_text_buffer_place_cursor (buffer, &first);
    }
    
  return TRUE;
}

static gboolean
search_marks_in_view (GtkWidget    *source_view, 
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

void
codeslayer_search_clear_search_marks (CodeSlayerSearch *search)
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
forward_search (CodeSlayerSearch *search, 
                const gchar      *find, 
                GtkTextIter      *start, 
                GtkTextIter      *begin,
                GtkTextIter      *end, 
                gboolean          match_case, 
                gboolean          match_word)
{
  gboolean result;
  
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
          result = forward_search (search, find, start, begin, end, match_case, match_word);
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
                 gboolean          match_word)
{
  gboolean result;
  
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
          result = backward_search (search, find, start, begin, end, match_case, match_word);
        }
    }

  return result;
}
