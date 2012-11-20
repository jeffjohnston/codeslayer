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

#include <stdlib.h>
#include <codeslayer/codeslayer-linker.h>

typedef struct
{
  gchar *file_path;
  gint   line_number;
  gint   start_offset;
  gint   end_offset;
} Link;

static void codeslayer_linker_class_init    (CodeSlayerLinkerClass *klass);
static void codeslayer_linker_init          (CodeSlayerLinker      *linker);
static void codeslayer_linker_finalize      (CodeSlayerLinker      *linker);
static void codeslayer_linker_get_property  (GObject                *object, 
                                             guint                   prop_id,
                                             GValue                 *value,
                                             GParamSpec             *pspec);
static void codeslayer_linker_set_property  (GObject                *object, 
                                             guint                   prop_id,
                                             const GValue           *value,
                                             GParamSpec             *pspec);

static void clear_links                     (CodeSlayerLinker      *linker);
static void create_links                    (CodeSlayerLinker      *linker);
static GList* mark_links                    (CodeSlayerLinker      *linker, 
                                             GtkTextBuffer         *buffer, 
                                             GList                 *matches);
static GList* find_matches                  (CodeSlayerLinker      *linker, 
                                             gchar                 *text);
static Link* create_link                    (CodeSlayerLinker      *linker,
                                             gchar                 *text,
                                             GtkTextIter           *begin, 
                                             GtkTextIter           *end);
static gboolean select_link_action          (CodeSlayerLinker      *linker, 
                                             GdkEventButton        *event);
static gboolean notify_link_action          (CodeSlayerLinker      *linker, 
                                             GdkEventButton        *event);
                                             
#define LINKER_DEFAULT_PATTERN "(\\/.*?:\\d+)"                                             

#define CODESLAYER_LINKER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_LINKER_TYPE, CodeSlayerLinkerPrivate))

typedef struct _CodeSlayerLinkerPrivate CodeSlayerLinkerPrivate;

struct _CodeSlayerLinkerPrivate
{
  CodeSlayer  *codeslayer;
  GtkTextView *text_view;
  gchar       *pattern;
  GList       *links;
  GtkTextTag  *underline_tag;
};

enum
{
  PROP_0,
  PROP_PATTERN
};

G_DEFINE_TYPE (CodeSlayerLinker, codeslayer_linker, G_TYPE_OBJECT)
      
static void 
codeslayer_linker_class_init (CodeSlayerLinkerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_linker_finalize;

  gobject_class->get_property = codeslayer_linker_get_property;
  gobject_class->set_property = codeslayer_linker_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerLinkerPrivate));
  
  /**
   * CodeSlayerLinker:pattern:
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_PATTERN,
                                   g_param_spec_string ("pattern", 
                                                        "Pattern",
                                                        "Pattern Object", "",
                                                        G_PARAM_READWRITE));
}

static void
codeslayer_linker_init (CodeSlayerLinker *linker) 
{
}

static void
codeslayer_linker_finalize (CodeSlayerLinker *linker)
{
  CodeSlayerLinkerPrivate *priv;
  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);

  clear_links (linker);

  if (priv->pattern)
    {
      g_free (priv->pattern);
      priv->pattern = NULL;
    }

  G_OBJECT_CLASS (codeslayer_linker_parent_class)->finalize (G_OBJECT (linker));
}

static void
codeslayer_linker_get_property (GObject    *object, 
                                guint       prop_id,
                                GValue     *value, 
                                GParamSpec *pspec)
{
  CodeSlayerLinker *linker;
  CodeSlayerLinkerPrivate *priv;
  
  linker = CODESLAYER_LINKER (object);
  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);

  switch (prop_id)
    {
    case PROP_PATTERN:
      g_value_set_string (value, priv->pattern);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_linker_set_property (GObject      *object, 
                                guint         prop_id,
                                const GValue *value, 
                                GParamSpec   *pspec)
{
  CodeSlayerLinker *linker;
  linker = CODESLAYER_LINKER (object);

  switch (prop_id)
    {
    case PROP_PATTERN:
      codeslayer_linker_set_pattern (linker, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

CodeSlayerLinker*
codeslayer_linker_new (CodeSlayer  *codeslayer, 
                       GtkTextView *text_view)
{
  return codeslayer_linker_new_with_pattern (codeslayer, text_view, 
                                             LINKER_DEFAULT_PATTERN);
}

CodeSlayerLinker*
codeslayer_linker_new_with_pattern (CodeSlayer  *codeslayer, 
                                    GtkTextView *text_view,
                                    gchar       *pattern)
{
  GtkWidget *linker;
  CodeSlayerLinkerPrivate *priv;
  GtkTextBuffer *buffer;
 
  linker = g_object_new (codeslayer_linker_get_type (), NULL);
  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);
  priv->codeslayer = codeslayer;
  priv->text_view = text_view;
  priv->pattern = g_strdup (pattern);
  
  buffer = gtk_text_view_get_buffer (priv->text_view);
  
  priv->links = NULL;
  gtk_text_view_set_editable (priv->text_view, FALSE);
  gtk_text_view_set_wrap_mode (priv->text_view, GTK_WRAP_WORD);

  priv->underline_tag = gtk_text_buffer_create_tag (buffer, "underline", "underline", 
                                                    PANGO_UNDERLINE_SINGLE, NULL);
                                                    
  g_signal_connect_swapped (G_OBJECT (priv->text_view), "button-press-event",
                            G_CALLBACK (select_link_action), linker);
  g_signal_connect_swapped (G_OBJECT (priv->text_view), "motion-notify-event",
                            G_CALLBACK (notify_link_action), linker);
  g_signal_connect_swapped (G_OBJECT (buffer), "changed",
                            G_CALLBACK (create_links), linker);

  return CODESLAYER_LINKER (linker);
}

/**
 * codeslayer_linker_get_pattern:
 * @linker: a #CodeSlayerLinker.
 *
 * Returns: the text to display for the project.
 */
const gchar *
codeslayer_linker_get_pattern (CodeSlayerLinker *linker)
{
  return CODESLAYER_LINKER_GET_PRIVATE (linker)->pattern;
}

/**
 * codeslayer_linker_set_pattern:
 * @linker: a #CodeSlayerLinker.
 * @pattern: the pattern used to search for valid links.
 */
void
codeslayer_linker_set_pattern (CodeSlayerLinker *linker, 
                               const gchar      *pattern)
{
  CodeSlayerLinkerPrivate *priv;
  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);
  if (priv->pattern)
    {
      g_free (priv->pattern);
      priv->pattern = NULL;
    }
  priv->pattern = g_strdup (pattern);
}

/**
 * codeslayer_linker_create_links:
 * @linker: a #CodeSlayerLinker.
 */
void
create_links (CodeSlayerLinker *linker)
{
  CodeSlayerLinkerPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start;
  GtkTextIter end;
  gchar *text;
  GList *matches;

  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);
  
  buffer = gtk_text_view_get_buffer (priv->text_view);

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  matches = find_matches (linker, text);
  clear_links (linker);
  priv->links = mark_links (linker, buffer, matches);
  g_list_foreach (matches, (GFunc) g_free, NULL);
  g_free (text);
}

static void
clear_links (CodeSlayerLinker *linker)
{
  CodeSlayerLinkerPrivate *priv;
  GList *list;
  
  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);

  if (priv->links == NULL)
    return;
  
  list = priv->links;
  
  while (list != NULL)
    {
      Link *link = list->data;
      g_free (link->file_path);
      g_free (link);
      list = g_list_next (list);
    }
  
  g_list_free (priv->links);
  priv->links = NULL;
}

static GList*
find_matches (CodeSlayerLinker *linker, 
              gchar            *text)
{
  CodeSlayerLinkerPrivate *priv;
  GList *results = NULL;
  GRegex *regex;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);
  
  regex = g_regex_new (priv->pattern, 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (g_match_info_matches (match_info))
    {
      gchar *match_text = NULL;
      match_text = g_match_info_fetch (match_info, 1);
      
      results = g_list_prepend (results, g_strdup (match_text));
        
      g_free (match_text);
      g_match_info_next (match_info, &error);
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for completion word error: %s\n", error->message);
      g_error_free (error);
    }

  return results;    
}

static GList*
mark_links (CodeSlayerLinker *linker, 
            GtkTextBuffer   *buffer, 
            GList           *matches)
{
  GList *results = NULL;

  while (matches != NULL)
    {
      gchar *match_text = matches->data;
      GtkTextIter start, begin, end;

      gtk_text_buffer_get_start_iter (buffer, &start);
      
      while (gtk_text_iter_forward_search (&start, match_text, 
                                           GTK_TEXT_SEARCH_TEXT_ONLY, 
                                           &begin, &end, NULL))
        {
          Link *link;
          gtk_text_buffer_apply_tag_by_name (buffer, "underline", &begin, &end);
          
          link = create_link (linker, match_text, &begin, &end);
          if (link != NULL)
            results = g_list_prepend (results, link);
          
          start = begin;
          gtk_text_iter_forward_char (&start);
        }
      
      matches = g_list_next (matches);
    }
    
  return results;    
}

static Link*
create_link (CodeSlayerLinker *linker,
             gchar           *text,
             GtkTextIter     *begin, 
             GtkTextIter     *end)
{
  Link *link = NULL;
  gchar **split, **tmp;
  
  split = g_strsplit (text, ":", 0);  
  
  if (split != NULL)
    {
      tmp = split;

      link = g_malloc (sizeof (Link));
      link->file_path = g_strdup (*tmp);
      tmp++;
      link->line_number = atoi(*tmp);
      link->start_offset = gtk_text_iter_get_offset (begin);
      link->end_offset = gtk_text_iter_get_offset (end);
      
      g_strfreev(split);
    }

  return link;
}

static gboolean
select_link_action (CodeSlayerLinker *linker, 
                    GdkEventButton  *event)
{
  CodeSlayerLinkerPrivate *priv;

  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);

  if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS))
    {
      GdkWindow *window;
      GtkTextIter iter;
      gint offset, x, y, bx, by;
      GList *list;

      window = gtk_text_view_get_window (priv->text_view,
                                         GTK_TEXT_WINDOW_TEXT);
                                                                                                                
      gdk_window_get_device_position (window, event->device, &x, &y, NULL);      
      
      gtk_text_view_window_to_buffer_coords (priv->text_view,
                                             GTK_TEXT_WINDOW_TEXT,
                                             x, y, &bx, &by);

      gtk_text_view_get_iter_at_location (priv->text_view,
                                          &iter, bx, by);
      
      offset = gtk_text_iter_get_offset (&iter);
      
      list = priv->links;
      
      while (list != NULL)
        {
          Link *link = list->data;
          
          if (offset >= link->start_offset && offset <= link->end_offset)
            {
              CodeSlayerDocument *document;
              CodeSlayerProject *project;
              document = codeslayer_document_new ();
              codeslayer_document_set_file_path (document, link->file_path);
              codeslayer_document_set_line_number (document, link->line_number);
              
              project = codeslayer_get_project_by_file_path (priv->codeslayer, 
                                                             link->file_path);
              codeslayer_document_set_project (document, project);
              
              codeslayer_select_editor (priv->codeslayer, document);
              g_object_unref (document);
              return FALSE;
            }
          list = g_list_next (list);
        }
    }

  return FALSE;
}

static gboolean
notify_link_action (CodeSlayerLinker *linker, 
                    GdkEventButton  *event)
{

  CodeSlayerLinkerPrivate *priv;
  GdkWindow *window;
  GdkCursor *cursor;
  GtkTextIter iter;
  gint x, y, bx, by;

  priv = CODESLAYER_LINKER_GET_PRIVATE (linker);

  window = gtk_text_view_get_window (priv->text_view,
                                     GTK_TEXT_WINDOW_TEXT);
                                                                                                                
  gdk_window_get_device_position (window, event->device, &x, &y, NULL);      

  gtk_text_view_window_to_buffer_coords (priv->text_view,
                                         GTK_TEXT_WINDOW_TEXT,
                                         x, y, &bx, &by);
      
  gtk_text_view_get_iter_at_location (priv->text_view,
                                      &iter, bx, by);

  cursor = gdk_window_get_cursor (window);

  if (gtk_text_iter_has_tag (&iter, priv->underline_tag))
    {
      if (cursor == NULL || gdk_cursor_get_cursor_type (cursor) != GDK_HAND1)
        {
          cursor = gdk_cursor_new (GDK_HAND1);
          gdk_window_set_cursor (window, cursor);
          g_object_unref (cursor);    
        }
    } 
  else 
    {
      if (cursor != NULL && gdk_cursor_get_cursor_type (cursor) != GDK_XTERM)
        {
          cursor = gdk_cursor_new (GDK_XTERM);
          gdk_window_set_cursor (window, cursor);
          g_object_unref (cursor);    
        }
    }
      
  return FALSE;
}
