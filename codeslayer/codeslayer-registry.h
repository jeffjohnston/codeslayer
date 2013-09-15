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

#ifndef __CODESLAYER_REGISTRY_H__
#define __CODESLAYER_REGISTRY_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-config.h>
#include <codeslayer/codeslayer-config-handler.h>

G_BEGIN_DECLS

#define CODESLAYER_REGISTRY_TYPE            (codeslayer_registry_get_type ())
#define CODESLAYER_REGISTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_REGISTRY_TYPE, CodeSlayerRegistry))
#define CODESLAYER_REGISTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_REGISTRY_TYPE, CodeSlayerRegistryClass))
#define IS_CODESLAYER_REGISTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_REGISTRY_TYPE))
#define IS_CODESLAYER_REGISTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_REGISTRY_TYPE))

#define CODESLAYER_REGISTRY_HPANED_POSITION "hpaned_position"
#define CODESLAYER_REGISTRY_VPANED_POSITION "vpaned_position"
#define CODESLAYER_REGISTRY_WINDOW_WIDTH "window_width"
#define CODESLAYER_REGISTRY_WINDOW_HEIGHT "window_height"
#define CODESLAYER_REGISTRY_WINDOW_X "window_x"
#define CODESLAYER_REGISTRY_WINDOW_Y "window_y"
#define CODESLAYER_REGISTRY_SEARCH_WIDTH "search_width"
#define CODESLAYER_REGISTRY_SEARCH_HEIGHT "search_height"
#define CODESLAYER_REGISTRY_SEARCH_X "search_x"
#define CODESLAYER_REGISTRY_SEARCH_Y "search_y"
#define CODESLAYER_REGISTRY_MAIN_TITLE "CodeSlayer"
#define CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE "side_pane_visible"
#define CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE "bottom_pane_visible"
#define CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_MATCH_CASE "notebook_search_match_case"
#define CODESLAYER_REGISTRY_NOTEBOOK_SEARCH_MATCH_WORD "notebook_search_match_word"
#define CODESLAYER_REGISTRY_DRAW_SPACES "draw_spaces"
#define CODESLAYER_REGISTRY_WORD_WRAP "word_wrap"
#define CODESLAYER_REGISTRY_SYNC_WITH_EDITOR "sync_with_editor"

#define CODESLAYER_REGISTRY_EDITOR_DISPLAY_LINE_NUMBERS "editor_display_line_numbers"
#define CODESLAYER_REGISTRY_EDITOR_HIGHLIGHT_CURRENT_LINE "editor_highlight_current_line"
#define CODESLAYER_REGISTRY_EDITOR_DISPLAY_RIGHT_MARGIN "editor_display_right_margin"
#define CODESLAYER_REGISTRY_EDITOR_HIGHLIGHT_MATCHING_BRACKET "editor_highlight_matching_bracket"
#define CODESLAYER_REGISTRY_EDITOR_INSERT_SPACES_INSTEAD_OF_TABS "editor_insert_spaces_instead_of_tabs"
#define CODESLAYER_REGISTRY_EDITOR_ENABLE_AUTOMATIC_INDENTATION "editor_enable_automatic_indentation"
#define CODESLAYER_REGISTRY_EDITOR_RIGHT_MARGIN_POSITION "editor_right_margin_position"
#define CODESLAYER_REGISTRY_EDITOR_TAB_WIDTH "editor_tab_width"
#define CODESLAYER_REGISTRY_EDITOR_FONT "editor_font"
#define CODESLAYER_REGISTRY_EDITOR_THEME "editor_theme"
#define CODESLAYER_REGISTRY_EDITOR_TAB_POSITION "editor_tab_position"
#define CODESLAYER_REGISTRY_EDITOR_WORD_WRAP_TYPES "editor_word_wrap_types"
#define CODESLAYER_REGISTRY_SIDE_PANE_TAB_POSITION "side_pane_tab_position"
#define CODESLAYER_REGISTRY_BOTTOM_PANE_TAB_POSITION "bottom_pane_tab_position"
#define CODESLAYER_REGISTRY_PROJECTS_EXCLUDE_TYPES "projects_exclude_types"
#define CODESLAYER_REGISTRY_PROJECTS_EXCLUDE_DIRS "projects_exclude_dirs"

typedef struct _CodeSlayerRegistry CodeSlayerRegistry;
typedef struct _CodeSlayerRegistryClass CodeSlayerRegistryClass;

struct _CodeSlayerRegistry
{
  GObject parent_instance;
};

struct _CodeSlayerRegistryClass
{
  GObjectClass parent_class;
};

GType codeslayer_registry_get_type (void) G_GNUC_CONST;

CodeSlayerRegistry*  codeslayer_registry_new          (CodeSlayerConfigHandler *config_handler);

gint                 codeslayer_registry_get_integer  (CodeSlayerRegistry *registry, 
                                                       gchar              *key);
void                 codeslayer_registry_set_integer  (CodeSlayerRegistry *registry, 
                                                       gchar              *key,
                                                       gint                value);
gdouble              codeslayer_registry_get_double   (CodeSlayerRegistry *registry, 
                                                       gchar              *key);
void                 codeslayer_registry_set_double   (CodeSlayerRegistry *registry, 
                                                       gchar              *key,
                                                       gdouble             value);
gboolean             codeslayer_registry_get_boolean  (CodeSlayerRegistry *registry, 
                                                       gchar              *key);
void                 codeslayer_registry_set_boolean  (CodeSlayerRegistry *registry, 
                                                       gchar              *key,
                                                       gboolean            value);
gchar*               codeslayer_registry_get_string   (CodeSlayerRegistry *registry,
                                                       gchar              *key);
void                 codeslayer_registry_set_string   (CodeSlayerRegistry *registry, 
                                                       gchar              *key,
                                                       gchar              *value);

G_END_DECLS

#endif /* __CODESLAYER_REGISTRY_H__ */
