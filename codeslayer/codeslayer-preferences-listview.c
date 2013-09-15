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

#include <codeslayer/codeslayer-preferences-listview.h>
#include <codeslayer/codeslayer-preferences-utils.h>
#include <codeslayer/codeslayer-preferences.h>
#include <codeslayer/codeslayer-listview.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-preferences-listview
 * @short_description: Common list view widget for the preferences.
 * @title: CodeSlayerPreferencesListView
 * @include: codeslayer/codeslayer-preferences-listview.h
 */

static void codeslayer_preferences_list_view_class_init  (CodeSlayerPreferencesListViewClass *klass);
static void codeslayer_preferences_list_view_init        (CodeSlayerPreferencesListView      *project);
static void codeslayer_preferences_list_view_finalize    (CodeSlayerPreferencesListView      *project);

static void add_list_view                                (CodeSlayerPreferencesListView      *preferences_listview);
static void load_list                                    (CodeSlayerPreferencesListView      *preferences_listview,
                                                          GtkWidget                          *list_view);
static void list_changed_action                          (CodeSlayerPreferencesListView      *preferences_listview, 
                                                          GList                              *list);

#define CODESLAYER_PREFERENCES_LIST_VIEW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PREFERENCES_LIST_VIEW_TYPE, CodeSlayerPreferencesListViewPrivate))

typedef struct _CodeSlayerPreferencesListViewPrivate CodeSlayerPreferencesListViewPrivate;

struct _CodeSlayerPreferencesListViewPrivate
{
  CodeSlayerPreferences *preferences;
  CodeSlayerRegistry    *registry;
  gchar                 *key;
  GtkWidget             *tab;
  const gchar           *title;
};

enum
{
  TEXT,
  COLUMNS
};

enum
{
  LIST_CHANGED,
  LAST_SIGNAL
};

static guint codeslayer_preferences_list_view_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerPreferencesListView, codeslayer_preferences_list_view, G_TYPE_OBJECT)
     
static void 
codeslayer_preferences_list_view_class_init (CodeSlayerPreferencesListViewClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  /**
   * CodeSlayerPreferencesListView::list-changed
   * @codeslayerpreferenceslistview: the list view that received the signal
   *
   * The ::list-changed signal is emitted when the list values are edited, added, or removed.
   */
  codeslayer_preferences_list_view_signals[LIST_CHANGED] =
    g_signal_new ("list-changed", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerPreferencesListViewClass, list_changed), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_preferences_list_view_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerPreferencesListViewPrivate));
}

static void
codeslayer_preferences_list_view_init (CodeSlayerPreferencesListView *project) {}

static void
codeslayer_preferences_list_view_finalize (CodeSlayerPreferencesListView *project)
{
  G_OBJECT_CLASS (codeslayer_preferences_list_view_parent_class)->finalize (G_OBJECT (project));
}

/**
 * codeslayer_preferences_list_view_new:
 * @preferences: a #CodeSlayerPreferences.
 * @key: the preferences key.
 * @tab: a #GtkWidget.
 * @title: the title to give the section.
 *
 * Creates a new #CodeSlayerPreferencesListView.
 *
 * Returns: a new #CodeSlayerPreferencesListView. 
 */
GObject*
codeslayer_preferences_list_view_new (CodeSlayerPreferences *preferences, 
                                      CodeSlayerRegistry    *registry,
                                      gchar                 *key,
                                      GtkWidget             *tab, 
                                      const gchar           *title)
{
  CodeSlayerPreferencesListViewPrivate *priv;
  GObject *preferences_listview;
  
  preferences_listview = g_object_new (codeslayer_preferences_list_view_get_type (), NULL);
  priv = CODESLAYER_PREFERENCES_LIST_VIEW_GET_PRIVATE (preferences_listview);
  priv->preferences = preferences;
  priv->registry = registry;
  priv->title = title;
  priv->key = key;
  priv->tab = tab;
  
  add_list_view (CODESLAYER_PREFERENCES_LIST_VIEW (preferences_listview));
  
  return preferences_listview;
}

static void
add_list_view (CodeSlayerPreferencesListView *preferences_listview)
{
  CodeSlayerPreferencesListViewPrivate *priv;
  GtkBox *content_area;
  GtkWidget *list_view;  
  
  priv = CODESLAYER_PREFERENCES_LIST_VIEW_GET_PRIVATE (preferences_listview);
  
  content_area = codeslayer_preferences_utils_content_area (GTK_BOX (priv->tab), 
                                                            _(priv->title));
  list_view = codeslayer_list_view_new ();
  gtk_widget_set_size_request (list_view, -1, 140);
  
  g_signal_connect_swapped (G_OBJECT (list_view), "list-changed",
                            G_CALLBACK (list_changed_action), preferences_listview);
                            
  load_list (preferences_listview, list_view);

  gtk_box_pack_start (content_area, list_view, TRUE, TRUE, 0);
}

static void
load_list (CodeSlayerPreferencesListView *preferences_listview,
           GtkWidget                     *list_view)
{
  CodeSlayerPreferencesListViewPrivate *priv;
  gchar *include_types;
  gchar **split, **tmp;

  priv = CODESLAYER_PREFERENCES_LIST_VIEW_GET_PRIVATE (preferences_listview);

  include_types = codeslayer_registry_get_string (priv->registry, priv->key);
  split = g_strsplit (include_types, ",", 0);
  g_free (include_types);
  
  if (split != NULL)
    {
      tmp = split;
      while (*tmp != NULL)
        {
          codeslayer_list_view_add_text (CODESLAYER_LIST_VIEW (list_view), *tmp);
          tmp++;
        }
      g_strfreev (split);
    }
}

static void
list_changed_action (CodeSlayerPreferencesListView *preferences_listview, 
                     GList                         *list)
{
  CodeSlayerPreferencesListViewPrivate *priv;
  gchar *value;
  GString *gs;
    
  priv = CODESLAYER_PREFERENCES_LIST_VIEW_GET_PRIVATE (preferences_listview);

  gs = g_string_new ("");

  if (list != NULL)
    {
      gchar *value = list->data;
      gs = g_string_append (gs, value);
      list = g_list_next (list);
    }
  
  while (list != NULL)
    {
      gchar *value = list->data;
      gs = g_string_append (gs, ",");
      gs = g_string_append (gs, value);
      list = g_list_next (list);
    }

  value = g_string_free (gs, FALSE);

  codeslayer_registry_set_string (priv->registry, priv->key, value);
  codeslayer_preferences_utils_notify (priv->registry);
  
  g_signal_emit_by_name ((gpointer) preferences_listview, "list-changed");
  g_free (value);
}
