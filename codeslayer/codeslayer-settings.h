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

#ifndef __CODESLAYER_SETTINGS_H__
#define __CODESLAYER_SETTINGS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-config.h>
#include <codeslayer/codeslayer-config-handler.h>

G_BEGIN_DECLS

#define CODESLAYER_SETTINGS_TYPE            (codeslayer_settings_get_type ())
#define CODESLAYER_SETTINGS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_SETTINGS_TYPE, CodeSlayerSettings))
#define CODESLAYER_SETTINGS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_SETTINGS_TYPE, CodeSlayerSettingsClass))
#define IS_CODESLAYER_SETTINGS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_SETTINGS_TYPE))
#define IS_CODESLAYER_SETTINGS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_SETTINGS_TYPE))

#define CODESLAYER_SETTINGS_HPANED_POSITION "hpaned_position"
#define CODESLAYER_SETTINGS_VPANED_POSITION "vpaned_position"
#define CODESLAYER_SETTINGS_WINDOW_WIDTH "window_width"
#define CODESLAYER_SETTINGS_WINDOW_HEIGHT "window_height"
#define CODESLAYER_SETTINGS_WINDOW_X "window_x"
#define CODESLAYER_SETTINGS_WINDOW_Y "window_y"
#define CODESLAYER_SETTINGS_SEARCH_WIDTH "search_width"
#define CODESLAYER_SETTINGS_SEARCH_HEIGHT "search_height"
#define CODESLAYER_SETTINGS_SEARCH_X "search_x"
#define CODESLAYER_SETTINGS_SEARCH_Y "search_y"
#define CODESLAYER_SETTINGS_MAIN_TITLE "CodeSlayer"
#define CODESLAYER_SETTINGS_SIDE_PANE_VISIBLE "side_pane_visible"
#define CODESLAYER_SETTINGS_BOTTOM_PANE_VISIBLE "bottom_pane_visible"
#define CODESLAYER_SETTINGS_NOTEBOOK_SEARCH_MATCH_CASE "notebook_search_match_case"
#define CODESLAYER_SETTINGS_NOTEBOOK_SEARCH_MATCH_WORD "notebook_search_match_word"
#define CODESLAYER_SETTINGS_DRAW_SPACES "draw_spaces"
#define CODESLAYER_SETTINGS_WORD_WRAP "word_wrap"
#define CODESLAYER_SETTINGS_SYNC_WITH_EDITOR "sync_with_editor"

typedef struct _CodeSlayerSettings CodeSlayerSettings;
typedef struct _CodeSlayerSettingsClass CodeSlayerSettingsClass;

struct _CodeSlayerSettings
{
  GObject parent_instance;
};

struct _CodeSlayerSettingsClass
{
  GObjectClass parent_class;
};

GType codeslayer_settings_get_type (void) G_GNUC_CONST;

CodeSlayerSettings*  codeslayer_settings_new          (CodeSlayerConfigHandler *config_handler);

gint                 codeslayer_settings_get_integer  (CodeSlayerSettings *settings, 
                                                       gchar              *key);
void                 codeslayer_settings_set_integer  (CodeSlayerSettings *settings, 
                                                       gchar              *key,
                                                       gint                value);
gboolean             codeslayer_settings_get_boolean  (CodeSlayerSettings *settings, 
                                                       gchar              *key);
void                 codeslayer_settings_set_boolean  (CodeSlayerSettings *settings, 
                                                       gchar              *key,
                                                       gboolean            value);
gchar*               codeslayer_settings_get_string   (CodeSlayerSettings *settings,
                                                       gchar              *key);
void                 codeslayer_settings_set_string   (CodeSlayerSettings *settings, 
                                                       gchar              *key,
                                                       gchar              *value);

G_END_DECLS

#endif /* __CODESLAYER_SETTINGS_H__ */
