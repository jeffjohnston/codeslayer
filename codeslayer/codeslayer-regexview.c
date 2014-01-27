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
 * SECTION:codeslayer-regex
 * @short_description: The view that contains the additional regular expression tools.
 * @title: CodeSlayerRegexView
 * @include: codeslayer/codeslayer-regex.h
 */

static void codeslayer_regex_view_class_init         (CodeSlayerRegexViewClass *klass);
static void codeslayer_regex_view_init               (CodeSlayerRegexView      *regex_view);
static void codeslayer_regex_view_finalize           (CodeSlayerRegexView      *regex_view);

static void search_changed_action                    (CodeSlayerRegexView      *search, 
                                                      gchar                    *find, 
                                                      gchar                    *replace, 
                                                      gboolean                  match_case, 
                                                      gboolean                  match_word, 
                                                      gboolean                  regular_expression);
static void add_buttons                              (CodeSlayerRegexView      *regex_view);                                               
static void add_text_view                            (CodeSlayerRegexView      *regex_view);
static void extract_action                           (CodeSlayerRegexView      *regex_view);
static void extract_action                           (CodeSlayerRegexView      *regex_view);
static GtkWidget* codeslayer_get_active_source_view  (CodeSlayerRegexView      *regex_view);

#define CODESLAYER_REGEX_VIEW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_REGEX_VIEW_TYPE, CodeSlayerRegexViewPrivate))

typedef struct _CodeSlayerRegexViewPrivate CodeSlayerRegexViewPrivate;

struct _CodeSlayerRegexViewPrivate
{
  GtkWidget         *notebook;
  CodeSlayerProfile *profile;
  GtkWidget         *text_view;
  gchar             *find;
  gchar             *replace;
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

  gtk_container_set_border_width (GTK_CONTAINER (regex_view), 2);
}

static void
codeslayer_regex_view_finalize (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  if (priv->find)
    g_free (priv->find);

  if (priv->replace)
    g_free (priv->replace);

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
  
  priv->notebook = notebook;
  priv->profile = profile;
  
  g_signal_connect_swapped (G_OBJECT (notebook_search), "search-changed",
                            G_CALLBACK (search_changed_action), regex_view);
                            
  add_buttons (CODESLAYER_REGEX_VIEW (regex_view));
  add_text_view (CODESLAYER_REGEX_VIEW (regex_view));

  return regex_view;
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
  
  if (priv->find)
    {
      g_free (priv->find);
      priv->find = NULL;    
    }

  if (priv->replace)
    {
      g_free (priv->replace);
      priv->replace = NULL;    
    }
    
  if (regular_expression == TRUE)
    {
      priv->find = g_strdup (find);
      priv->replace = g_strdup (replace);
    }
}

static void
add_buttons (CodeSlayerRegexView *regex_view)
{
  GtkWidget *hbox;
  GtkWidget *extract_button;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  
  extract_button =  gtk_button_new_with_label ("Extract Find/Replace");
  
  gtk_box_pack_start (GTK_BOX (hbox), extract_button, FALSE, FALSE, 0);
  
  gtk_box_pack_start (GTK_BOX (regex_view), hbox, FALSE, FALSE, 0);
  
  g_signal_connect_swapped (G_OBJECT (extract_button), "clicked",
                            G_CALLBACK (extract_action), regex_view);
}

static void
add_text_view (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  GtkWidget *scrolled_window;
  GtkWidget *text_view;

  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  text_view = gtk_text_view_new ();
  priv->text_view = text_view;
  
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (text_view));

  gtk_box_pack_start (GTK_BOX (regex_view), scrolled_window, TRUE, TRUE, 2);
}

static void
extract_action (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  GtkWidget *source_view;
  GtkTextBuffer *source_buffer;
  GtkTextIter start, end;
  gchar *source_text;
  GRegex *regex;
  GMatchInfo *match_info = NULL;
  GString *string;
  GtkTextBuffer *buffer;
  gchar *text;

  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  if (!codeslayer_utils_has_text (priv->find))
    return;
    
  regex = g_regex_new (priv->find, 0, 0, NULL);

  if (regex == NULL)
    return;
    
  source_view = codeslayer_get_active_source_view (regex_view);
  source_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));

  gtk_text_buffer_get_bounds (source_buffer, &start, &end);
  source_text = gtk_text_buffer_get_text (source_buffer, &start, &end, FALSE);
  
  string = g_string_new ("");
  
  g_regex_match (regex, source_text, 0, &match_info);
  while (g_match_info_matches (match_info))
    {
      gchar *find_match = g_match_info_fetch (match_info, 0);
      
      if (codeslayer_utils_has_text (priv->replace))
        {
          gchar *replace_match;
          replace_match = g_regex_replace (regex, find_match, -1, 0, priv->replace, 0, NULL);
          string = g_string_append (string, replace_match);
          g_free (replace_match);
        }
      else
        {
          string = g_string_append (string, find_match);
          string = g_string_append (string, "\n");
        }

      g_free (find_match);
      g_match_info_next (match_info, NULL);
    }
    
  text = g_string_free (string, FALSE);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->text_view));
  gtk_text_buffer_set_text (buffer, text, -1);

  if (text)
    g_free (text);
}

static GtkWidget*
codeslayer_get_active_source_view (CodeSlayerRegexView *regex_view)
{
  CodeSlayerRegexViewPrivate *priv;
  
  priv = CODESLAYER_REGEX_VIEW_GET_PRIVATE (regex_view);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 0)
    return NULL;

  return codeslayer_notebook_get_active_source_view (CODESLAYER_NOTEBOOK (priv->notebook));
}

