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

#include <gtksourceview/gtksourcestyleschememanager.h>
#include <codeslayer/codeslayer-preferences-theme.h>
#include <codeslayer/codeslayer-preferences-utils.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-preferences-theme
 * @short_description: The preferences theme tab.
 * @title: CodeSlayerPreferencesTheme
 * @include: codeslayer/codeslayer-preferences-theme.h
 */

static void codeslayer_preferences_theme_class_init  (CodeSlayerPreferencesThemeClass *klass);
static void codeslayer_preferences_theme_init        (CodeSlayerPreferencesTheme      *preferences_theme);
static void codeslayer_preferences_theme_finalize    (CodeSlayerPreferencesTheme      *preferences_theme);

static gint sort_iter_compare_func                   (GtkTreeModel                    *model, 
                                                      GtkTreeIter                     *a,
                                                      GtkTreeIter                     *b, 
                                                      gpointer                         userdata);
static void add_font                                 (CodeSlayerPreferencesTheme      *preferences_theme);
static void add_theme                                (CodeSlayerPreferencesTheme      *preferences_theme);
static void font_action                              (CodeSlayerPreferencesTheme      *preferences_theme);
static void add_tab_positions                        (CodeSlayerPreferencesTheme      *preferences_theme);
static gboolean theme_action                         (GtkTreeView                     *treeview,
                                                      CodeSlayerPreferencesTheme      *preferences_theme);
static void editor_tab_position_action               (CodeSlayerPreferencesTheme      *preferences_theme);
static void side_pane_tab_position_action            (CodeSlayerPreferencesTheme      *preferences_theme);
static void bottom_pane_tab_position_action          (CodeSlayerPreferencesTheme      *preferences_theme);
static void add_editor_tab_position                  (CodeSlayerPreferencesTheme      *preferences_theme, 
                                                      GtkWidget                       *table);
static void add_side_pane_tab_position               (CodeSlayerPreferencesTheme      *preferences_theme, 
                                                      GtkWidget                       *table);
static void add_bottom_pane_tab_position             (CodeSlayerPreferencesTheme      *preferences_theme, 
                                                      GtkWidget                       *table);
static void populate_combo_box                       (CodeSlayerPreferencesTheme      *preferences_theme,
                                                      GtkWidget                       *combo_box,
                                                      gchar                           *preferences_key,
                                                      GtkPositionType                  default_position_type);
                                                   

#define CODESLAYER_PREFERENCES_THEME_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PREFERENCES_THEME_TYPE, CodeSlayerPreferencesThemePrivate))

typedef struct _CodeSlayerPreferencesThemePrivate CodeSlayerPreferencesThemePrivate;

struct _CodeSlayerPreferencesThemePrivate
{
  CodeSlayerPreferences *preferences;
  GtkFontButton         *font;
  GtkListStore          *themes_store;
  GtkWidget             *editor_tab_position;
  GtkWidget             *side_pane_tab_position;
  GtkWidget             *bottom_pane_tab_position;
};

enum
{
  TEXT,
  COLUMNS
};

G_DEFINE_TYPE (CodeSlayerPreferencesTheme, codeslayer_preferences_theme, GTK_TYPE_VBOX)

static void
codeslayer_preferences_theme_class_init (CodeSlayerPreferencesThemeClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_preferences_theme_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPreferencesThemePrivate));
}

static void
codeslayer_preferences_theme_init (CodeSlayerPreferencesTheme *preferences_theme)
{
  gtk_box_set_homogeneous (GTK_BOX (preferences_theme), FALSE);
  gtk_box_set_spacing (GTK_BOX (preferences_theme), 15);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_theme), 5);
}

static void
codeslayer_preferences_theme_finalize (CodeSlayerPreferencesTheme *preferences_theme)
{
  G_OBJECT_CLASS (codeslayer_preferences_theme_parent_class)->finalize (G_OBJECT (preferences_theme));
}

/**
 * codeslayer_preferences_theme_new:
 * @preferences: a #CodeSlayerPreferences.
 *
 * Creates a new #CodeSlayerPreferencesTheme.
 *
 * Returns: a new #CodeSlayerPreferencesTheme. 
 */
GtkWidget*
codeslayer_preferences_theme_new (CodeSlayerPreferences *preferences)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GtkWidget *preferences_theme;
  
  preferences_theme = g_object_new (codeslayer_preferences_theme_get_type (), NULL);
  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);
  
  priv->preferences = preferences;
  
  add_font (CODESLAYER_PREFERENCES_THEME (preferences_theme));
  add_theme (CODESLAYER_PREFERENCES_THEME (preferences_theme));
  add_tab_positions (CODESLAYER_PREFERENCES_THEME (preferences_theme));
  
  return preferences_theme;
}

static void
add_font (CodeSlayerPreferencesTheme *preferences_theme)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GtkBox *content_area;
  GtkWidget *label;
  GtkWidget *font;
  gchar *fontname;
  
  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);
  
  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_theme), 
                                                            _("Font"));

  label = gtk_label_new (_("Editor: "));
  gtk_box_pack_start (content_area, label, FALSE, FALSE, 0);

  font = gtk_font_button_new ();
  priv->font = GTK_FONT_BUTTON (font);
  fontname = codeslayer_preferences_get_string (priv->preferences,
                                                CODESLAYER_PREFERENCES_EDITOR_FONT);
  gtk_font_button_set_font_name (GTK_FONT_BUTTON (font), fontname);
  g_free (fontname);

  g_signal_connect_swapped (G_OBJECT (font), "font-set", 
                            G_CALLBACK (font_action), preferences_theme);
                    
  gtk_box_pack_start (content_area, font, FALSE, FALSE, 0);
}

static void
add_tab_positions (CodeSlayerPreferencesTheme *preferences_theme)
{
  GtkBox *content_area;
  GtkWidget *table;
  
  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_theme), 
                                                            _("Tab Positions"));

  table = gtk_table_new (2, 2, FALSE);

  add_editor_tab_position (preferences_theme, table);
  add_side_pane_tab_position (preferences_theme, table);
  add_bottom_pane_tab_position (preferences_theme, table);
  
  gtk_box_pack_start (content_area, table, FALSE, FALSE, 0);
}

static void
add_editor_tab_position (CodeSlayerPreferencesTheme *preferences_theme, 
                         GtkWidget                  *table)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GtkWidget *label;
  GtkWidget *combo_box;

  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);

  label = gtk_label_new (_("Editor: "));
  combo_box = gtk_combo_box_text_new ();
  priv->editor_tab_position = combo_box;
  
  populate_combo_box (preferences_theme, 
                      combo_box, 
                      CODESLAYER_PREFERENCES_EDITOR_TAB_POSITION, 
                      GTK_POS_TOP);

  g_signal_connect_swapped (G_OBJECT (combo_box), "changed", 
                            G_CALLBACK (editor_tab_position_action), preferences_theme);

  gtk_misc_set_alignment (GTK_MISC (label), 1, .5);
  gtk_table_attach (GTK_TABLE (table), label, 
                    0, 1, 0, 1, GTK_FILL, GTK_SHRINK, 0, 2);  
  
  gtk_table_attach (GTK_TABLE (table), combo_box, 
                    1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 2);  
}

static void
add_side_pane_tab_position (CodeSlayerPreferencesTheme *preferences_theme, 
                            GtkWidget                  *table)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GtkWidget *label;
  GtkWidget *combo_box;

  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);

  label = gtk_label_new (_("Side Pane: "));
  combo_box = gtk_combo_box_text_new ();
  priv->side_pane_tab_position = combo_box;

  populate_combo_box (preferences_theme, 
                      combo_box, 
                      CODESLAYER_PREFERENCES_SIDE_PANE_TAB_POSITION, 
                      GTK_POS_TOP);

 g_signal_connect_swapped (G_OBJECT (combo_box), "changed", 
                           G_CALLBACK (side_pane_tab_position_action), preferences_theme);

  gtk_misc_set_alignment (GTK_MISC (label), 1, .5);
  gtk_table_attach (GTK_TABLE (table), label, 
                    0, 1, 1, 2, GTK_FILL, GTK_SHRINK, 0, 2);  
  
  gtk_table_attach (GTK_TABLE (table), combo_box, 
                    1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 2);  
}

static void
add_bottom_pane_tab_position (CodeSlayerPreferencesTheme *preferences_theme, 
                              GtkWidget                  *table)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GtkWidget *label;
  GtkWidget *combo_box;

  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);

  label = gtk_label_new (_("Bottom Pane: "));
  combo_box = gtk_combo_box_text_new ();
  priv->bottom_pane_tab_position = combo_box;

  populate_combo_box (preferences_theme, 
                      combo_box, 
                      CODESLAYER_PREFERENCES_BOTTOM_PANE_TAB_POSITION, 
                      GTK_POS_LEFT);

 g_signal_connect_swapped (G_OBJECT (combo_box), "changed", 
                           G_CALLBACK (bottom_pane_tab_position_action), preferences_theme);

  gtk_misc_set_alignment (GTK_MISC (label), 1, .5);
  gtk_table_attach (GTK_TABLE (table), label, 
                    0, 1, 2, 3, GTK_FILL, GTK_SHRINK, 0, 2);  
  
  gtk_table_attach (GTK_TABLE (table), combo_box, 
                    1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 2);  
}

static void
populate_combo_box (CodeSlayerPreferencesTheme *preferences_theme,
                    GtkWidget                  *combo_box,
                    gchar                      *preferences_key,
                    GtkPositionType             default_position_type)
{
  CodeSlayerPreferencesThemePrivate *priv;
  gchar *editor_value;

  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);
  
  editor_value = codeslayer_preferences_get_string (priv->preferences, preferences_key);

  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_box), _("Left"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_box), _("Right"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_box), _("Top"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_box), _("Bottom"));  

  if (g_strcmp0 (editor_value, "left") == 0)
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), GTK_POS_LEFT);
  else if (g_strcmp0 (editor_value, "right") == 0)
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), GTK_POS_RIGHT);
  else if (g_strcmp0 (editor_value, "top") == 0)
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), GTK_POS_TOP);
  else if (g_strcmp0 (editor_value, "bottom") == 0)
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), GTK_POS_BOTTOM); 
  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), default_position_type); 
    
  if (editor_value)                                             
    g_free (editor_value);
}

static void
add_theme (CodeSlayerPreferencesTheme *preferences_theme)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GtkBox *content_area;
  GtkWidget *themes_tree;
  GtkListStore *themes_store;
  GtkTreeSortable *sortable;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkWidget *scrolled_window;
  gchar *theme;
  GtkTreeRowReference *tree_row_reference = NULL;
  GtkSourceStyleSchemeManager *style_scheme_manager;
  const gchar *const *style_schemes;
  GtkTreeIter iter;
  
  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);
  
  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (preferences_theme), 
                                                            _("Theme"));

  themes_tree = gtk_tree_view_new ();
  
  themes_store = gtk_list_store_new (COLUMNS, G_TYPE_STRING);
  priv->themes_store = themes_store;

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (themes_tree), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (themes_tree),
                           GTK_TREE_MODEL (themes_store));

  sortable = GTK_TREE_SORTABLE (themes_store);
  gtk_tree_sortable_set_sort_func (sortable, TEXT, sort_iter_compare_func,
                                   GINT_TO_POINTER (TEXT), NULL);
  gtk_tree_sortable_set_sort_column_id (sortable, TEXT, GTK_SORT_ASCENDING);

  column = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_text_new ();

  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer, "text", TEXT, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (themes_tree), column);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (themes_tree));
  gtk_widget_set_size_request (scrolled_window, -1, 120);

  theme = codeslayer_preferences_get_string (priv->preferences,
                                             CODESLAYER_PREFERENCES_EDITOR_THEME);
                                             
  style_scheme_manager = gtk_source_style_scheme_manager_get_default ();
  style_schemes = gtk_source_style_scheme_manager_get_scheme_ids (style_scheme_manager);
  while (*style_schemes)
    {
      gtk_list_store_append (themes_store, &iter);
      gtk_list_store_set (themes_store, &iter, TEXT, *style_schemes, -1);

      if (g_ascii_strcasecmp (theme, *style_schemes) == 0)
        {
          GtkTreePath *tree_path;
          tree_path = gtk_tree_model_get_path (GTK_TREE_MODEL (themes_store), 
                                               &iter);
          tree_row_reference = gtk_tree_row_reference_new (GTK_TREE_MODEL (themes_store), tree_path);
          gtk_tree_path_free (tree_path);
        }

      style_schemes++;
    }
    
  if (theme)                                             
    g_free (theme);

  if (tree_row_reference)
    {
      GtkTreePath *tree_path;
      tree_path = gtk_tree_row_reference_get_path (tree_row_reference);
      gtk_tree_view_set_cursor (GTK_TREE_VIEW (themes_tree), tree_path, 
                                NULL, FALSE);
      gtk_tree_path_free (tree_path);
    }

  g_signal_connect (G_OBJECT (themes_tree), "cursor-changed",
                    G_CALLBACK (theme_action), preferences_theme);

  gtk_box_pack_start (content_area, scrolled_window, TRUE, TRUE, 0);
}

static gint
sort_iter_compare_func (GtkTreeModel *model, 
                        GtkTreeIter  *a,
                        GtkTreeIter  *b, 
                        gpointer      userdata)
{
  gint sortcol;
  gint ret = 0;

  sortcol = GPOINTER_TO_INT (userdata);
  
  switch (sortcol)
    {
    case TEXT:
      {
        gchar *text1, *text2;

        gtk_tree_model_get (model, a, TEXT, &text1, -1);
        gtk_tree_model_get (model, b, TEXT, &text2, -1);

        ret = g_strcmp0 (text1, text2);

        g_free (text1);
        g_free (text2);
      }
      break;
    }

  return ret;
}

static void
font_action (CodeSlayerPreferencesTheme *preferences_theme)
{
  CodeSlayerPreferencesThemePrivate *priv;
  const gchar *value;
  
  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);

  value = gtk_font_button_get_font_name (priv->font);
  codeslayer_preferences_set_string (priv->preferences,
                                     CODESLAYER_PREFERENCES_EDITOR_FONT,
                                     value);

  codeslayer_preferences_save (priv->preferences);
  codeslayer_preferences_utils_notify_editors (priv->preferences);
}

static void                
editor_tab_position_action (CodeSlayerPreferencesTheme *preferences_theme)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GString *string;
  gchar *value;
  
  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);

  value = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (priv->editor_tab_position));
  string = g_string_new (value);
  g_free (value);
  string = g_string_ascii_down (string);
  value = g_string_free (string, FALSE);
  
  codeslayer_preferences_set_string (priv->preferences,
                                     CODESLAYER_PREFERENCES_EDITOR_TAB_POSITION,
                                     value);
  codeslayer_preferences_save (priv->preferences);
  codeslayer_preferences_utils_notify_notebook (priv->preferences);

  g_free (value);
}                            

static void
side_pane_tab_position_action (CodeSlayerPreferencesTheme *preferences_theme)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GString *string;
  gchar *value;
  
  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);

  value = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (priv->side_pane_tab_position));
  string = g_string_new (value);
  g_free (value);
  string = g_string_ascii_down (string);
  value = g_string_free (string, FALSE);
  
  codeslayer_preferences_set_string (priv->preferences,
                                     CODESLAYER_PREFERENCES_SIDE_PANE_TAB_POSITION,
                                     value);
  codeslayer_preferences_save (priv->preferences);
  codeslayer_preferences_utils_notify_side_pane (priv->preferences);

  g_free (value);
}                            

static void
bottom_pane_tab_position_action (CodeSlayerPreferencesTheme *preferences_theme)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GString *string;
  gchar *value;
  
  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);

  value = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (priv->bottom_pane_tab_position));
  string = g_string_new (value);
  g_free (value);
  string = g_string_ascii_down (string);
  value = g_string_free (string, FALSE);
  
  codeslayer_preferences_set_string (priv->preferences,
                                     CODESLAYER_PREFERENCES_BOTTOM_PANE_TAB_POSITION,
                                     value);
  codeslayer_preferences_save (priv->preferences);
  codeslayer_preferences_utils_notify_bottom_pane (priv->preferences);

  g_free (value);
}                            

static gboolean
theme_action (GtkTreeView                *treeview, 
              CodeSlayerPreferencesTheme *preferences_theme)
{
  CodeSlayerPreferencesThemePrivate *priv;
  GtkTreeModel *model;
  GtkTreeSelection *treeselection;
  GtkTreeIter iter;
  
  priv = CODESLAYER_PREFERENCES_THEME_GET_PRIVATE (preferences_theme);

  treeselection = gtk_tree_view_get_selection (treeview);
  if (gtk_tree_selection_get_selected (treeselection, &model, &iter))
    {
      gchar *value = NULL;
      gtk_tree_model_get (GTK_TREE_MODEL (priv->themes_store), &iter, 
                          TEXT, &value, -1);
      codeslayer_preferences_set_string (priv->preferences,
                                         CODESLAYER_PREFERENCES_EDITOR_THEME,
                                         value);
      g_free (value);
    }

  codeslayer_preferences_save (priv->preferences);
  codeslayer_preferences_utils_notify_editors (priv->preferences);
  return FALSE;
}
