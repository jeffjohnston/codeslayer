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

#include <codeslayer/codeslayer-preferences-editor.h>
#include <codeslayer/codeslayer-preferences-utils.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-preferences-editor
 * @short_description: The preferences editor tab.
 * @title: CodeSlayerPreferencesEditor
 * @include: codeslayer/codeslayer-preferences-editor.h
 */

static void codeslayer_preferences_editor_class_init  (CodeSlayerPreferencesEditorClass *klass);
static void codeslayer_preferences_editor_init        (CodeSlayerPreferencesEditor      *preferences_editor);
static void codeslayer_preferences_editor_finalize    (CodeSlayerPreferencesEditor      *preferences_editor);

static void add_line_number                           (CodeSlayerPreferencesEditor      *preferences_editor);
static void add_current_line                          (CodeSlayerPreferencesEditor      *preferences_editor);
static void add_right_margin                          (CodeSlayerPreferencesEditor      *preferences_editor);
static void add_bracket_matching                      (CodeSlayerPreferencesEditor      *preferences_editor);
static void add_tab_stops                             (CodeSlayerPreferencesEditor      *preferences_editor);
static void add_automatic_indentation                 (CodeSlayerPreferencesEditor      *preferences_editor);
static void display_line_number_action                (CodeSlayerPreferencesEditor      *preferences_editor);
static void highlight_current_line_action             (CodeSlayerPreferencesEditor      *preferences_editor);
static void display_right_margin_action               (CodeSlayerPreferencesEditor      *preferences_editor);
static void highlight_matching_bracket_action         (CodeSlayerPreferencesEditor      *preferences_editor);
static void insert_spaces_instead_of_tabs_action      (CodeSlayerPreferencesEditor      *preferences_editor);
static void enable_automatic_indentation_action       (CodeSlayerPreferencesEditor      *preferences_editor);
static void right_margin_position_action              (CodeSlayerPreferencesEditor      *preferences_editor);
static void tab_width_action                          (CodeSlayerPreferencesEditor      *preferences_editor);                                                   

#define CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PREFERENCES_EDITOR_TYPE, CodeSlayerPreferencesEditorPrivate))

typedef struct _CodeSlayerPreferencesEditorPrivate CodeSlayerPreferencesEditorPrivate;

struct _CodeSlayerPreferencesEditorPrivate
{
  CodeSlayerPreferences *preferences;
  CodeSlayerProfile     *profile;
  GtkCheckButton        *display_line_number;
  GtkCheckButton        *highlight_current_line;
  GtkCheckButton        *display_right_margin;
  GtkCheckButton        *highlight_matching_bracket;
  GtkCheckButton        *insert_spaces_instead_of_tabs;
  GtkCheckButton        *enable_automatic_indentation;
  GtkSpinButton         *right_margin_position;
  GtkSpinButton         *tab_width;
};

G_DEFINE_TYPE (CodeSlayerPreferencesEditor, codeslayer_preferences_editor, GTK_TYPE_VBOX)

static void
codeslayer_preferences_editor_class_init (CodeSlayerPreferencesEditorClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_preferences_editor_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPreferencesEditorPrivate));
}

static void
codeslayer_preferences_editor_init (CodeSlayerPreferencesEditor *preferences_editor)
{
  gtk_box_set_homogeneous (GTK_BOX (preferences_editor), FALSE);
  gtk_box_set_spacing (GTK_BOX (preferences_editor), 15);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_editor), 5);
}

static void
codeslayer_preferences_editor_finalize (CodeSlayerPreferencesEditor *preferences_editor)
{
  G_OBJECT_CLASS (codeslayer_preferences_editor_parent_class)->finalize (G_OBJECT (preferences_editor));
}

/**
 * codeslayer_preferences_editor_new:
 * @preferences: a #CodeSlayerPreferences.
 * @profile: a #CodeSlayerProfile.
 *
 * Creates a new #CodeSlayerPreferencesEditor.
 *
 * Returns: a new #CodeSlayerPreferencesEditor. 
 */
GtkWidget*
codeslayer_preferences_editor_new (CodeSlayerPreferences *preferences, 
                                   CodeSlayerProfile     *profile)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  GtkWidget *preferences_editor;
  
  preferences_editor = g_object_new (codeslayer_preferences_editor_get_type (), NULL);
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  priv->preferences = preferences;
  priv->profile = profile;
  
  add_line_number (CODESLAYER_PREFERENCES_EDITOR (preferences_editor));
  add_current_line (CODESLAYER_PREFERENCES_EDITOR (preferences_editor));
  add_right_margin (CODESLAYER_PREFERENCES_EDITOR (preferences_editor));
  add_bracket_matching (CODESLAYER_PREFERENCES_EDITOR (preferences_editor));
  add_tab_stops (CODESLAYER_PREFERENCES_EDITOR (preferences_editor));
  add_automatic_indentation (CODESLAYER_PREFERENCES_EDITOR (preferences_editor));
  
  return preferences_editor;
}

static void
add_line_number (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  GtkBox *content_area;
  GtkWidget *display_line_number;
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_editor), 
                                                            _("Line Numbers"));

  display_line_number = gtk_check_button_new_with_label (_("Display line numbers"));
  priv->display_line_number = GTK_CHECK_BUTTON (display_line_number);
  active = codeslayer_registry_get_boolean (registry, 
                                            CODESLAYER_REGISTRY_DISPLAY_LINE_NUMBERS);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->display_line_number), active);

  g_signal_connect_swapped (G_OBJECT (display_line_number), "toggled",
                            G_CALLBACK (display_line_number_action), preferences_editor);
                    
  gtk_box_pack_start (content_area, display_line_number, FALSE, FALSE, 0);
}

static void
add_current_line (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  GtkBox *content_area;
  GtkWidget *highlight_current_line;
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_editor), 
                                                            _("Current Line"));

  highlight_current_line = gtk_check_button_new_with_label (_("Highlight current line"));
  priv->highlight_current_line = GTK_CHECK_BUTTON (highlight_current_line);
  active = codeslayer_registry_get_boolean (registry,
                                            CODESLAYER_REGISTRY_HIGHLIGHT_CURRENT_LINE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->highlight_current_line), active);
  
  g_signal_connect_swapped (G_OBJECT (highlight_current_line), "toggled",
                            G_CALLBACK (highlight_current_line_action), preferences_editor);
                    
  gtk_box_pack_start (content_area, highlight_current_line, FALSE, FALSE, 0);
}

static void
add_right_margin (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry;  
  GtkBox *content_area;
  GtkWidget *display_right_margin;
  gboolean active;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *right_margin_position;
  gdouble value;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_editor), 
                                                            _("Right Margin"));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
                                                            
  display_right_margin = gtk_check_button_new_with_label (_("Display right margin"));
  priv->display_right_margin = GTK_CHECK_BUTTON (display_right_margin);
  active = codeslayer_registry_get_boolean (registry,
                                               CODESLAYER_REGISTRY_DISPLAY_RIGHT_MARGIN);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->display_right_margin), active);

  g_signal_connect_swapped (G_OBJECT (display_right_margin), "toggled",
                            G_CALLBACK (display_right_margin_action), preferences_editor);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
  gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);

  label = gtk_label_new (_("Right margin position: "));
  right_margin_position = gtk_spin_button_new_with_range (0, 160, 1);
  priv->right_margin_position = GTK_SPIN_BUTTON (right_margin_position);
  value = codeslayer_registry_get_double (registry,
                                             CODESLAYER_REGISTRY_RIGHT_MARGIN_POSITION);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (right_margin_position), value);

  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), right_margin_position, FALSE, FALSE, 0);
  
  g_signal_connect_swapped (G_OBJECT (right_margin_position), "value-changed",
                            G_CALLBACK (right_margin_position_action), preferences_editor);
  
  gtk_box_pack_start (GTK_BOX (vbox), display_right_margin, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  gtk_box_pack_start (content_area, vbox, FALSE, FALSE, 0);  
}

static void
add_bracket_matching (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  GtkBox *content_area;
  GtkWidget *highlight_matching_bracket;
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_editor), 
                                                            _("Bracket Matching"));

  highlight_matching_bracket = gtk_check_button_new_with_label (_("Highlight matching bracket"));
  priv->highlight_matching_bracket = GTK_CHECK_BUTTON (highlight_matching_bracket);
  active = codeslayer_registry_get_boolean (registry,
                                               CODESLAYER_REGISTRY_HIGHLIGHT_MATCHING_BRACKET);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->highlight_matching_bracket), active);
  
  g_signal_connect_swapped (G_OBJECT (highlight_matching_bracket), "toggled",
                            G_CALLBACK (highlight_matching_bracket_action), preferences_editor);
                    
  gtk_box_pack_start (content_area, highlight_matching_bracket, FALSE, FALSE, 0);
}

static void
add_tab_stops (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  GtkBox *content_area;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *tab_width;
  gdouble value;
  GtkWidget *insert_spaces_instead_of_tabs;
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);
  
  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_editor), 
                                                            _("Tab Stops"));
                                                            
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
  gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
  
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
  gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);

  label = gtk_label_new (_("Tab width: "));
  tab_width = gtk_spin_button_new_with_range (0, 24, 1);
  priv->tab_width = GTK_SPIN_BUTTON (tab_width);
  value = codeslayer_registry_get_double (registry,
                                             CODESLAYER_REGISTRY_TAB_WIDTH);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (tab_width), value);
  
  g_signal_connect_swapped (G_OBJECT (tab_width), "value-changed",
                            G_CALLBACK (tab_width_action), preferences_editor);

  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), tab_width, FALSE, FALSE, 0);

  insert_spaces_instead_of_tabs = gtk_check_button_new_with_label (_("Insert spaces instead of tabs"));
  priv->insert_spaces_instead_of_tabs = GTK_CHECK_BUTTON (insert_spaces_instead_of_tabs);
  active = codeslayer_registry_get_boolean (registry,
                                               CODESLAYER_REGISTRY_INSERT_SPACES_INSTEAD_OF_TABS);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->insert_spaces_instead_of_tabs), active);
  
  g_signal_connect_swapped (G_OBJECT (insert_spaces_instead_of_tabs), "toggled",
                            G_CALLBACK (insert_spaces_instead_of_tabs_action), preferences_editor);
  
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), insert_spaces_instead_of_tabs, FALSE, FALSE, 0);

  gtk_box_pack_start (content_area, vbox, FALSE, FALSE, 0);  
}

static void
add_automatic_indentation (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  GtkBox *content_area;
  GtkWidget *enable_automatic_indentation;
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_editor), 
                                                            _("Automatic Indentation"));

  enable_automatic_indentation = gtk_check_button_new_with_label (_("Enable automatic indentation"));
  priv->enable_automatic_indentation = GTK_CHECK_BUTTON (enable_automatic_indentation);
  active = codeslayer_registry_get_boolean (registry,
                                               CODESLAYER_REGISTRY_ENABLE_AUTOMATIC_INDENTATION);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->enable_automatic_indentation), active);
  
  g_signal_connect_swapped (G_OBJECT (enable_automatic_indentation), "toggled",
                            G_CALLBACK (enable_automatic_indentation_action), preferences_editor);
  
  gtk_box_pack_start (content_area, enable_automatic_indentation, FALSE, FALSE, 0);
}

static void
display_line_number_action (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->display_line_number));
  codeslayer_registry_set_boolean (registry,
                                      CODESLAYER_REGISTRY_DISPLAY_LINE_NUMBERS,
                                      active);

  codeslayer_preferences_utils_notify (registry);
}

static void
highlight_current_line_action (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->highlight_current_line));
  codeslayer_registry_set_boolean (registry,
                                      CODESLAYER_REGISTRY_HIGHLIGHT_CURRENT_LINE,
                                      active);

  codeslayer_preferences_utils_notify (registry);
}

static void
display_right_margin_action (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->display_right_margin));
  codeslayer_registry_set_boolean (registry,
                                      CODESLAYER_REGISTRY_DISPLAY_RIGHT_MARGIN,
                                      active);

  codeslayer_preferences_utils_notify (registry);
}

static void
highlight_matching_bracket_action (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->highlight_matching_bracket));
  codeslayer_registry_set_boolean (registry,
                                      CODESLAYER_REGISTRY_HIGHLIGHT_MATCHING_BRACKET,
                                      active);

  codeslayer_preferences_utils_notify (registry);
}

static void
insert_spaces_instead_of_tabs_action (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->insert_spaces_instead_of_tabs));
  codeslayer_registry_set_boolean (registry,
                                      CODESLAYER_REGISTRY_INSERT_SPACES_INSTEAD_OF_TABS,
                                      active);

  codeslayer_preferences_utils_notify (registry);
}

static void
enable_automatic_indentation_action (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  gboolean active;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->enable_automatic_indentation));
  codeslayer_registry_set_boolean (registry,
                                      CODESLAYER_REGISTRY_ENABLE_AUTOMATIC_INDENTATION,
                                      active);

  codeslayer_preferences_utils_notify (registry);
}

static void
right_margin_position_action (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  gdouble value;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (priv->right_margin_position));
  codeslayer_registry_set_double (registry,
                                     CODESLAYER_REGISTRY_RIGHT_MARGIN_POSITION,
                                     value);

  codeslayer_preferences_utils_notify (registry);
}

static void
tab_width_action (CodeSlayerPreferencesEditor *preferences_editor)
{
  CodeSlayerPreferencesEditorPrivate *priv;
  CodeSlayerRegistry *registry; 
  gdouble value;
  
  priv = CODESLAYER_PREFERENCES_EDITOR_GET_PRIVATE (preferences_editor);
  
  registry = codeslayer_profile_get_registry (priv->profile);

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (priv->tab_width));
  codeslayer_registry_set_double (registry,
                                     CODESLAYER_REGISTRY_TAB_WIDTH,
                                     value);

  codeslayer_preferences_utils_notify (registry);
}
