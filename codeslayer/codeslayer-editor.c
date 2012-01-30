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
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcestyleschememanager.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-completion.h>

/**
 * SECTION:codeslayer-editor
 * @short_description: The view to edit a document.
 * @title: CodeSlayerEditor
 * @include: codeslayer/codeslayer-editor.h
 *
 * Extends the #GtkSourceView to further enhance how a document can be edited.
 */

static void codeslayer_editor_class_init      (CodeSlayerEditorClass *klass);
static void codeslayer_editor_init            (CodeSlayerEditor      *editor);
static void codeslayer_editor_finalize        (CodeSlayerEditor      *editor);

static GtkSourceBuffer* create_source_buffer  (const gchar           *file_name);
static gboolean key_press_action              (CodeSlayerEditor      *editor,
                                               GdkEventKey           *event);
static void copy_lines_action                 (CodeSlayerEditor      *editor);
static void to_uppercase_action               (CodeSlayerEditor      *editor);
static void to_lowercase_action               (CodeSlayerEditor      *editor);
static void completion_action                 (CodeSlayerEditor      *editor);
static void change_case                       (CodeSlayerEditor      *editor,
                                               GString* (*convert) (GString*));
                                               
#define CODESLAYER_EDITOR_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_EDITOR_TYPE, CodeSlayerEditorPrivate))

typedef struct _CodeSlayerEditorPrivate CodeSlayerEditorPrivate;

struct _CodeSlayerEditorPrivate
{
  GtkWindow             *window;
  CodeSlayerDocument    *document;
  CodeSlayerPreferences *preferences;
  CodeSlayerCompletion  *completion;
};

G_DEFINE_TYPE (CodeSlayerEditor, codeslayer_editor, GTK_SOURCE_TYPE_VIEW)
      
enum
{
  COPY_LINES,
  TO_UPPERCASE,
  TO_LOWERCASE,
  COMPLETION,
  LAST_SIGNAL
};

static guint codeslayer_editor_signals[LAST_SIGNAL] = { 0 };      
          
static void 
codeslayer_editor_class_init (CodeSlayerEditorClass *klass)
{
  GtkBindingSet *binding_set;

  klass->copy_lines = copy_lines_action;
  klass->to_uppercase = to_uppercase_action;
  klass->to_lowercase = to_lowercase_action;
  klass->completion = completion_action;

  /**
	 * CodeSlayerEditor::copy-lines
	 * @editor: the editor that received the signal
	 *
	 * The ::copy-lines signal enables the (Ctrl + Shift + Down) keystroke to copy 
	 * the currently selected lines.
	 */
  codeslayer_editor_signals[COPY_LINES] =
    g_signal_new ("copy-lines", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerEditorClass, copy_lines),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
	 * CodeSlayerEditor::to-uppercase
	 * @editor: the editor that received the signal
	 *
	 * The ::to-uppercase signal enables the (Ctrl + U) keystroke to uppercase 
	 * the selected text.
	 */
  codeslayer_editor_signals[TO_UPPERCASE] =
    g_signal_new ("to-uppercase", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerEditorClass, to_uppercase),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
	 * CodeSlayerEditor::to-lowercase
	 * @editor: the editor that received the signal
	 *
	 * The ::to-lowercase signal enables the (Ctrl + L) keystroke to lowercase the 
	 * selected text.
	 */
  codeslayer_editor_signals[TO_LOWERCASE] =
    g_signal_new ("to-lowercase", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerEditorClass, to_lowercase),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
	 * CodeSlayerEditor::completion
	 * @editor: the editor that received the signal
	 *
	 * The ::completion signal enables the (Ctrl + Space) keystroke to invoke the
	 * completion widget.
	 */
  codeslayer_editor_signals[COMPLETION] =
    g_signal_new ("completion", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (CodeSlayerEditorClass, completion),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_editor_finalize;

  binding_set = gtk_binding_set_by_class (klass);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Down,
                                GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                "copy-lines", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_U, 
                                GDK_CONTROL_MASK,
                                "to-uppercase", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_L, 
                                GDK_CONTROL_MASK,
                                "to-lowercase", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_space, 
                                GDK_CONTROL_MASK,
                                "completion", 0);

  g_type_class_add_private (klass, sizeof (CodeSlayerEditorPrivate));
}

static void
codeslayer_editor_init (CodeSlayerEditor *editor)
{
  CodeSlayerEditorPrivate *priv;
  priv = CODESLAYER_EDITOR_GET_PRIVATE(editor);
  priv->completion = NULL;
}

static void
codeslayer_editor_finalize (CodeSlayerEditor *editor)
{
  CodeSlayerEditorPrivate *priv;
  priv = CODESLAYER_EDITOR_GET_PRIVATE(editor);
  
  if (priv->completion != NULL)
    g_object_unref (priv->completion);

  G_OBJECT_CLASS (codeslayer_editor_parent_class)->finalize (G_OBJECT (editor));
}

/**
 * codeslayer_editor_new:
 * @document: the document for this editor.
 * @preferences: a #CodeSlayerPreferences.
 *
 * Creates a new #CodeSlayerEditor.
 *
 * Returns: a new #CodeSlayerEditor. 
 */
GtkWidget*
codeslayer_editor_new (GtkWindow             *window, 
                       CodeSlayerDocument    *document,
                       CodeSlayerPreferences *preferences)
{
  CodeSlayerEditorPrivate *priv;
  GtkWidget *editor;
  GtkSourceBuffer *buffer;
  const gchar *file_path;
  gchar *file_name;
  
  editor = g_object_new (codeslayer_editor_get_type (), NULL);
  priv = CODESLAYER_EDITOR_GET_PRIVATE (editor);
  priv->document = document;
  priv->preferences = preferences;
  priv->window = window;
  
  file_path = codeslayer_document_get_file_path (document);
  file_name = g_path_get_basename (file_path);
  buffer = create_source_buffer (file_name);
  g_free (file_name);
  
  gtk_text_view_set_buffer (GTK_TEXT_VIEW (editor), GTK_TEXT_BUFFER (buffer));
  g_object_unref (buffer);

  codeslayer_editor_sync_preferences (CODESLAYER_EDITOR (editor));

  g_signal_connect_swapped (G_OBJECT (editor), "key-press-event",
                            G_CALLBACK (key_press_action), editor);  

  return editor;
}

CodeSlayerDocument*  
codeslayer_editor_get_document (CodeSlayerEditor *editor)
{
  CodeSlayerEditorPrivate *priv;
  priv = CODESLAYER_EDITOR_GET_PRIVATE (editor);
  return priv->document;
}

void
codeslayer_editor_add_completion_provider (CodeSlayerEditor             *editor, 
                                           CodeSlayerCompletionProvider *provider)
{
  CodeSlayerEditorPrivate *priv;
  priv = CODESLAYER_EDITOR_GET_PRIVATE (editor);

  if (priv->completion == NULL)
    priv->completion = codeslayer_completion_new (priv->window);
    
  codeslayer_completion_add_provider (priv->completion, provider);
}

static GtkSourceBuffer*
create_source_buffer (const gchar *file_name)
{
  GtkSourceLanguageManager *language_manager;
  GtkSourceLanguage *language;
  GtkSourceBuffer *buffer;
  gboolean result_uncertain;
  gchar *content_type;
  
  content_type = g_content_type_guess (file_name, NULL, 0, &result_uncertain);
  if (result_uncertain)
    {
      g_free (content_type);
      content_type = NULL;
    }  
  
  language_manager = gtk_source_language_manager_get_default ();
  language = gtk_source_language_manager_guess_language (language_manager, 
                                                         file_name, content_type);

  if (language)
    buffer = gtk_source_buffer_new_with_language (language);
  else
    buffer = gtk_source_buffer_new (NULL);

  if (content_type)                        
    g_free (content_type);                              
  
  gtk_source_buffer_set_highlight_syntax (buffer, TRUE);
  gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer), 
                              "search-marks", "background", "#fff3a0", NULL);
  
  return buffer;
}

static void
copy_lines_action (CodeSlayerEditor *editor)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  GtkTextMark *end_mark;
  gint text_length;
  gchar *text;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  /* find the highlighted text */
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

  end_mark = gtk_text_buffer_create_mark (buffer, NULL, &end, TRUE);
  text_length = gtk_text_iter_get_offset (&end) - gtk_text_iter_get_offset (&start);

  /* copy the highlighted text */
  text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
  gtk_text_buffer_insert_interactive (buffer, &end, text, -1,
                                      gtk_text_view_get_editable
                                      (GTK_TEXT_VIEW (editor)));

  /* move the cursor and highlight the text */
  gtk_text_buffer_get_iter_at_mark (buffer, &start, end_mark);

  end = start;
  gtk_text_iter_forward_chars (&end, text_length);
  gtk_text_buffer_select_range (buffer, &start, &end);

  /* scroll to the new text */
  gtk_text_buffer_move_mark (buffer, end_mark, &end);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (editor), end_mark);

  /* remove the line mark */
  gtk_text_buffer_delete_mark (buffer, end_mark);
  
  g_free (text);
}


static void
to_uppercase_action (CodeSlayerEditor *editor)
{
  change_case (editor, g_string_ascii_up);
}

static void
to_lowercase_action (CodeSlayerEditor *editor)
{
  change_case (editor, g_string_ascii_down);
}

static void
change_case (CodeSlayerEditor *editor, 
             GString* (*convert) (GString*))
{
  GtkTextBuffer *buffer;
  GtkTextMark *start_mark;
  GtkTextIter start, end;
  gint text_length;
  gchar *text;
  GString *replace;
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  /* find the highlighted text */
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

  start_mark = gtk_text_buffer_create_mark (buffer, NULL, &start, TRUE);
  text_length = gtk_text_iter_get_offset (&end) - gtk_text_iter_get_offset (&start);

  text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);

  /* convert to uppercase */
  replace = g_string_new (text);
  replace = convert (replace);
  g_free (text);
  text = g_string_free (replace, FALSE);

  /* write back out */
  gtk_text_buffer_begin_user_action (buffer);
  gtk_text_buffer_delete (buffer, &start, &end);
  gtk_text_buffer_insert (buffer, &start, text, -1);
  gtk_text_buffer_end_user_action (buffer);

  /* select the range again */
  gtk_text_buffer_get_iter_at_mark (buffer, &start, start_mark);
  end = start;
  gtk_text_iter_forward_chars (&end, text_length);
  gtk_text_buffer_select_range (buffer, &start, &end);

  /* remove the line marks */
  gtk_text_buffer_delete_mark (buffer, start_mark);
}

/**
 * codeslayer_editor_scroll_to_line:
 * @editor: a #CodeSlayerEditor.
 * @line_number: the line to scroll to within the editor.
 */
void
codeslayer_editor_scroll_to_line (CodeSlayerEditor *editor, 
                                  gint              line_number)
{
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GtkTextMark *text_mark;
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_iter_set_line_offset (&iter, 0);
  gtk_text_iter_forward_lines (&iter, line_number - 1);

  /* Checks if any events are pending. This can be used to update the GUI and invoke 
     timeouts etc. while doing some time intensive computation. This needs to be done
     so that the height can be computed. */
  while (gtk_events_pending ())
      gtk_main_iteration ();

  text_mark = gtk_text_buffer_create_mark (buffer, NULL, &iter, TRUE);
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (editor), text_mark, .1, FALSE, 0, 0);

  gtk_text_buffer_place_cursor (buffer, &iter);
}

static void
completion_action (CodeSlayerEditor *editor)
{
  CodeSlayerEditorPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextMark *mark;
  GtkTextIter iter;
  
  priv = CODESLAYER_EDITOR_GET_PRIVATE (editor);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

  if (priv->completion != NULL)
    codeslayer_completion_invoke (priv->completion, GTK_TEXT_VIEW (editor), iter);
}

static gboolean
key_press_action (CodeSlayerEditor *editor,
                  GdkEventKey      *event)
{
  /*CodeSlayerEditorPrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextMark *mark;
  GtkTextIter iter;
  
  priv = CODESLAYER_EDITOR_GET_PRIVATE (editor);
  
  if (event->is_modifier == 1)
    return FALSE;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  mark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

  if (priv->completion != NULL)
    codeslayer_completion_invoke (priv->completion, iter);*/

  return FALSE;
}

/**
 * codeslayer_editor_sync_preferences:
 * @editor: a #CodeSlayerEditor.
 *
 * Apply the #CodeSlayerPreferences settings to the current editor.
 */
void
codeslayer_editor_sync_preferences (CodeSlayerEditor *editor)
{
  CodeSlayerEditorPrivate *priv;
  gboolean display_line_number;
  gboolean display_right_margin;
  gdouble right_margin_position;
  gdouble editor_tab_width;
  gboolean draw_spaces;
  gboolean enable_automatic_indentation;
  gboolean insert_spaces_instead_of_tabs;
  gboolean highlight_current_line;
  gboolean highlight_matching_bracket;
  gchar *theme;
  GtkTextBuffer *buffer;
  GtkSourceStyleSchemeManager *style_scheme_manager;
  GtkSourceStyleScheme *style_scheme;
  gchar *fontname;
  PangoFontDescription *font_description;
  const gchar *document_file_path;
  gchar *word_wrap_types_str;
  GList *word_wrap_types = NULL;
  
  priv = CODESLAYER_EDITOR_GET_PRIVATE (editor);

  display_line_number = codeslayer_preferences_get_boolean (priv->preferences,
                                                            CODESLAYER_PREFERENCES_EDITOR_DISPLAY_LINE_NUMBER);
  gtk_source_view_set_show_line_numbers (GTK_SOURCE_VIEW (editor), 
                                         display_line_number);

  display_right_margin = codeslayer_preferences_get_boolean (priv->preferences,
                                                             CODESLAYER_PREFERENCES_EDITOR_DISPLAY_RIGHT_MARGIN);
  gtk_source_view_set_show_right_margin (GTK_SOURCE_VIEW (editor), 
                                         display_right_margin);

  right_margin_position = codeslayer_preferences_get_double (priv->preferences,
                                                             CODESLAYER_PREFERENCES_EDITOR_RIGHT_MARGIN_POSITION);
  gtk_source_view_set_right_margin_position (GTK_SOURCE_VIEW (editor), 
                                             right_margin_position);

  editor_tab_width = codeslayer_preferences_get_double (priv->preferences,
                                                        CODESLAYER_PREFERENCES_EDITOR_TAB_WIDTH);
  gtk_source_view_set_tab_width (GTK_SOURCE_VIEW (editor), editor_tab_width);
  gtk_source_view_set_indent_width (GTK_SOURCE_VIEW (editor), -1);

  enable_automatic_indentation = codeslayer_preferences_get_boolean (priv->preferences,
                                                                     CODESLAYER_PREFERENCES_EDITOR_ENABLE_AUTOMATIC_INDENTATION);
  gtk_source_view_set_auto_indent (GTK_SOURCE_VIEW (editor), 
                                   enable_automatic_indentation);
  gtk_source_view_set_indent_on_tab (GTK_SOURCE_VIEW (editor),
                                     enable_automatic_indentation);

  draw_spaces = codeslayer_preferences_get_boolean (priv->preferences,
                                                    CODESLAYER_PREFERENCES_EDITOR_DRAW_SPACES);
  if (draw_spaces)
    gtk_source_view_set_draw_spaces (GTK_SOURCE_VIEW (editor), GTK_SOURCE_DRAW_SPACES_ALL);
  else
    gtk_source_view_set_draw_spaces (GTK_SOURCE_VIEW (editor), 0);

  insert_spaces_instead_of_tabs = codeslayer_preferences_get_boolean (priv->preferences,
                                                                      CODESLAYER_PREFERENCES_EDITOR_INSERT_SPACES_INSTEAD_OF_TABS);
  gtk_source_view_set_insert_spaces_instead_of_tabs (GTK_SOURCE_VIEW (editor),
                                                     insert_spaces_instead_of_tabs);

  highlight_current_line = codeslayer_preferences_get_boolean (priv->preferences,
                                                               CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_CURRENT_LINE);
  gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW (editor),
                                              highlight_current_line);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  highlight_matching_bracket = codeslayer_preferences_get_boolean (priv->preferences,
                                                                   CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_MATCHING_BRACKET);
  gtk_source_buffer_set_highlight_matching_brackets (GTK_SOURCE_BUFFER (buffer),
                                                     highlight_matching_bracket);

  theme = codeslayer_preferences_get_string (priv->preferences,
                                             CODESLAYER_PREFERENCES_EDITOR_THEME);
  
  style_scheme_manager = gtk_source_style_scheme_manager_get_default ();
  style_scheme = gtk_source_style_scheme_manager_get_scheme (style_scheme_manager, 
                                                             theme);
  if (theme)
    g_free (theme);

  if (style_scheme)
    gtk_source_buffer_set_style_scheme (GTK_SOURCE_BUFFER (buffer), style_scheme);

  fontname = codeslayer_preferences_get_string (priv->preferences,
                                                CODESLAYER_PREFERENCES_EDITOR_FONT);
  font_description = pango_font_description_from_string (fontname);
  
  if (fontname)
    g_free (fontname);
  
  gtk_widget_override_font (GTK_WIDGET (editor), font_description);
  pango_font_description_free (font_description);
  
  /* word wrap */
  
  document_file_path = codeslayer_document_get_file_path (priv->document);
  
  word_wrap_types_str = codeslayer_preferences_get_string (priv->preferences,
                                                           CODESLAYER_PREFERENCES_EDITOR_WORD_WRAP_TYPES);

  word_wrap_types = codeslayer_utils_string_to_list (word_wrap_types_str);
  
  if (codeslayer_utils_contains_element_with_suffix (word_wrap_types, document_file_path))
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (GTK_WIDGET (editor)), GTK_WRAP_WORD);
  else
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (GTK_WIDGET (editor)), GTK_WRAP_NONE);
  
  g_free (word_wrap_types_str);
  if (word_wrap_types)
    {
      g_list_foreach (word_wrap_types, (GFunc) g_free, NULL);
      g_list_free (word_wrap_types);
    }
}
