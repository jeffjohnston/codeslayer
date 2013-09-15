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

#ifndef __CODESLAYER_PREFERENCES_H__
#define __CODESLAYER_PREFERENCES_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-config.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-config-handler.h>

G_BEGIN_DECLS

#define CODESLAYER_PREFERENCES_TYPE            (codeslayer_preferences_get_type ())
#define CODESLAYER_PREFERENCES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PREFERENCES_TYPE, CodeSlayerPreferences))
#define CODESLAYER_PREFERENCES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PREFERENCES_TYPE, CodeSlayerPreferencesClass))
#define IS_CODESLAYER_PREFERENCES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PREFERENCES_TYPE))
#define IS_CODESLAYER_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PREFERENCES_TYPE))

#define CODESLAYER_PREFERENCES_EDITOR_DISPLAY_LINE_NUMBERS "editor_display_line_numbers"
#define CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_CURRENT_LINE "editor_highlight_current_line"
#define CODESLAYER_PREFERENCES_EDITOR_DISPLAY_RIGHT_MARGIN "editor_display_right_margin"
#define CODESLAYER_PREFERENCES_EDITOR_HIGHLIGHT_MATCHING_BRACKET "editor_highlight_matching_bracket"
#define CODESLAYER_PREFERENCES_EDITOR_INSERT_SPACES_INSTEAD_OF_TABS "editor_insert_spaces_instead_of_tabs"
#define CODESLAYER_PREFERENCES_EDITOR_ENABLE_AUTOMATIC_INDENTATION "editor_enable_automatic_indentation"
#define CODESLAYER_PREFERENCES_EDITOR_RIGHT_MARGIN_POSITION "editor_right_margin_position"
#define CODESLAYER_PREFERENCES_EDITOR_TAB_WIDTH "editor_tab_width"
#define CODESLAYER_PREFERENCES_EDITOR_FONT "editor_font"
#define CODESLAYER_PREFERENCES_EDITOR_THEME "editor_theme"
#define CODESLAYER_PREFERENCES_EDITOR_TAB_POSITION "editor_tab_position"
#define CODESLAYER_PREFERENCES_EDITOR_WORD_WRAP_TYPES "editor_word_wrap_types"
#define CODESLAYER_PREFERENCES_SIDE_PANE_TAB_POSITION "side_pane_tab_position"
#define CODESLAYER_PREFERENCES_BOTTOM_PANE_TAB_POSITION "bottom_pane_tab_position"
#define CODESLAYER_PREFERENCES_PROJECTS_EXCLUDE_TYPES "projects_exclude_types"
#define CODESLAYER_PREFERENCES_PROJECTS_EXCLUDE_DIRS "projects_exclude_dirs"

typedef struct _CodeSlayerPreferences CodeSlayerPreferences;
typedef struct _CodeSlayerPreferencesClass CodeSlayerPreferencesClass;

struct _CodeSlayerPreferences
{
  GObject parent_instance;
};

struct _CodeSlayerPreferencesClass
{
  GObjectClass parent_class;

  void (*editor_preferences_changed) (CodeSlayerPreferences *preferences);
  void (*notebook_preferences_changed) (CodeSlayerPreferences *preferences);
  void (*side_pane_preferences_changed) (CodeSlayerPreferences *preferences);
  void (*bottom_pane_preferences_changed) (CodeSlayerPreferences *preferences);
  void (*projects_preferences_changed) (CodeSlayerPreferences *preferences);
  void (*initialize_preferences) (CodeSlayerPreferences *preferences);
};

GType codeslayer_preferences_get_type (void) G_GNUC_CONST;

CodeSlayerPreferences*  codeslayer_preferences_new          (GtkWidget               *window, 
                                                             CodeSlayerConfigHandler *config_handler, 
                                                             CodeSlayerRegistry      *registry);

void                    codeslayer_preferences_run_dialog   (CodeSlayerPreferences *preferences);

G_END_DECLS

#endif /* __CODESLAYER_PREFERENCES_H__ */
