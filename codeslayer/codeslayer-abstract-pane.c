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

#include <codeslayer/codeslayer-abstract-pane.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-tearoff.h>

/**
 * SECTION:codeslayer-abstract-pane
 * @short_description: The abstract pane for the side and bottom pane.
 * @title: CodeSlayerAbstractPane
 * @include: codeslayer/codeslayer-abstract-pane.h
 */

static void    codeslayer_abstract_pane_class_init  (CodeSlayerAbstractPaneClass *klass);
static void    codeslayer_abstract_pane_init        (CodeSlayerAbstractPane      *abstract_pane);
static void    codeslayer_abstract_pane_finalize    (CodeSlayerAbstractPane      *abstract_pane);

static void    page_added_action                    (CodeSlayerAbstractPane      *abstract_pane,
                                                     GtkWidget                   *widget,
                                                     guint                        page_num);
static void    reorder_plugins                      (CodeSlayerAbstractPane      *abstract_pane);

static gchar*  tab_position_key                     (CodeSlayerAbstractPane      *abstract_pane);
static gint    tab_start_sort                       (CodeSlayerAbstractPane      *abstract_pane);

#define CODESLAYER_ABSTRACT_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_ABSTRACT_PANE_TYPE, CodeSlayerAbstractPanePrivate))

typedef struct _CodeSlayerAbstractPanePrivate CodeSlayerAbstractPanePrivate;

struct _CodeSlayerAbstractPanePrivate
{
  CodeSlayerProfiles *profiles;
  GtkWidget          *notebook;
  GList              *plugins;
};

enum
{
  OPEN_PANE,
  CLOSE_PANE,
  LAST_SIGNAL
};

static guint codeslayer_abstract_pane_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_ABSTRACT_TYPE (CodeSlayerAbstractPane, codeslayer_abstract_pane, GTK_TYPE_VBOX)

static void
codeslayer_abstract_pane_class_init (CodeSlayerAbstractPaneClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  /**
   * CodeSlayerAbstractPane::open-pane
   * @codeslayerabstractpane: the pane that received the signal.
   *
   * Note: for internal use only.
   *
   * The ::open-pane signal is a request for the pane to be opened.
   */
  codeslayer_abstract_pane_signals[OPEN_PANE] =
    g_signal_new ("open-pane", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerAbstractPaneClass, open_pane), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  
  /**
   * CodeSlayerAbstractPane::close-pane
   * @codeslayerabstractpane: the pane that received the signal.
   *
   * Note: for internal use only.
   *
   * The ::close-pane signal is a request for the pane to be closed.
   */
  codeslayer_abstract_pane_signals[CLOSE_PANE] =
    g_signal_new ("close-pane", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerAbstractPaneClass, close_pane), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_abstract_pane_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerAbstractPanePrivate));
}

static void
codeslayer_abstract_pane_init (CodeSlayerAbstractPane *abstract_pane)
{
  CodeSlayerAbstractPanePrivate *priv;
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);
  gtk_box_set_homogeneous (GTK_BOX (abstract_pane), FALSE);
  gtk_box_set_spacing (GTK_BOX (abstract_pane), 0);
  priv->plugins = NULL;
}

static void
codeslayer_abstract_pane_finalize (CodeSlayerAbstractPane *abstract_pane)
{
  CodeSlayerAbstractPanePrivate *priv;
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);
  g_list_free (priv->plugins);
  G_OBJECT_CLASS (codeslayer_abstract_pane_parent_class)->finalize (G_OBJECT (abstract_pane));
}

void
codeslayer_abstract_pane_create_notebook (CodeSlayerAbstractPane *abstract_pane)
{
  CodeSlayerAbstractPanePrivate *priv;
  GtkWidget *notebook;

  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);

  notebook = gtk_notebook_new ();
  priv->notebook = notebook;
  gtk_box_pack_start (GTK_BOX (abstract_pane), notebook, TRUE, TRUE, 1);
  
  g_signal_connect_swapped (G_OBJECT (priv->notebook), "page-added",
                            G_CALLBACK (page_added_action), abstract_pane);
                            
  g_signal_connect (G_OBJECT (priv->notebook), "create-window",
                    G_CALLBACK (codeslayer_tearoff_window), NULL);
}

void
codeslayer_abstract_pane_set_profiles (CodeSlayerAbstractPane *abstract_pane, 
                                       CodeSlayerProfiles     *profiles)
{
  CodeSlayerAbstractPanePrivate *priv;
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);
  priv->profiles = profiles;
}

/**
 * codeslayer_abstract_pane_add:
 * @abstract_pane: a #CodeSlayerAbstractPane.
 * @widget: a #GtkWidget.
 * @title: the title for the tab.
 *
 * Add the widget to the side pane.
 */
void
codeslayer_abstract_pane_add  (CodeSlayerAbstractPane *abstract_pane, 
                               GtkWidget              *widget, 
                               const gchar            *title)
{
  CodeSlayerAbstractPanePrivate *priv;
  GtkWidget *label;
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);
  label = gtk_label_new (title); 
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook), widget, label);
}

/**
 * codeslayer_abstract_pane_insert:
 * @abstract_pane: a #CodeSlayerAbstractPane.
 * @widget: a #GtkWidget.
 * @title: the title for the tab.
 * @position: the position to set.
 *
 * Add the widget to the side pane.
 */
void
codeslayer_abstract_pane_insert  (CodeSlayerAbstractPane *abstract_pane, 
                                  GtkWidget              *widget, 
                                  const gchar            *title, 
                                  gint                    position)
{
  CodeSlayerAbstractPanePrivate *priv;
  GtkWidget *label;
  gint page_num;
  
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);

  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (priv->notebook), widget);

  if (page_num == -1)
    {
      label = gtk_label_new (title); 
      gtk_notebook_insert_page (GTK_NOTEBOOK (priv->notebook), widget, label, position);
    }
}

/**
 * codeslayer_abstract_pane_remove:
 * @abstract_pane: a #CodeSlayerAbstractPane.
 * @widget: a #GtkWidget.
 *
 * Remove the widget from the side pane.
 */
void
codeslayer_abstract_pane_remove (CodeSlayerAbstractPane *abstract_pane, 
                                 GtkWidget              *widget)
{
  CodeSlayerAbstractPanePrivate *priv;
  gint page_num;
    
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);

  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (priv->notebook), widget);

  if (page_num == -1)
    g_critical ("The widget you are trying to remove from the side pane does not exist.");
  
  gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook), page_num);
  priv->plugins = g_list_remove (priv->plugins, widget);  
  reorder_plugins (abstract_pane);
}

gboolean
codeslayer_abstract_pane_exists (CodeSlayerAbstractPane *abstract_pane,
                                 GtkWidget              *widget)
{
  CodeSlayerAbstractPanePrivate *priv;
  gint page_num;
    
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);

  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (priv->notebook), widget);

  if (page_num != -1)
    return TRUE;
    
  return FALSE;    
}                                 

static void 
page_added_action (CodeSlayerAbstractPane *abstract_pane,
                   GtkWidget              *widget,
                   guint                   page_num)
{
  CodeSlayerAbstractPanePrivate *priv;
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);
  
  gtk_notebook_set_tab_detachable (GTK_NOTEBOOK (priv->notebook), widget, TRUE);
  
  if (IS_CODESLAYER_PROJECTS (widget))
    {
      gtk_notebook_reorder_child (GTK_NOTEBOOK (priv->notebook), widget, 0);
      gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);
    }
  else
    {
      gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), page_num);
      priv->plugins = g_list_append (priv->plugins, widget);
      reorder_plugins (abstract_pane);
    }

  gtk_widget_show_all (widget);
}                        

/**
 * codeslayer_abstract_pane_open:
 * @abstract_pane: a #CodeSlayerAbstractPane.
 * @widget: a #GtkWidget.
 */
void 
codeslayer_abstract_pane_open (CodeSlayerAbstractPane *abstract_pane, 
                               GtkWidget              *widget)
{
  CodeSlayerAbstractPanePrivate *priv;
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);
  
  if (widget != NULL)
    {
      gint page_num;
      page_num = gtk_notebook_page_num (GTK_NOTEBOOK (priv->notebook), widget);
      if (page_num >= 0)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), page_num);
    }

  g_signal_emit_by_name (abstract_pane, "open-pane");
}                        

/**
 * codeslayer_abstract_pane_close:
 * @abstract_pane: a #CodeSlayerAbstractPane.
 */
void 
codeslayer_abstract_pane_close (CodeSlayerAbstractPane *abstract_pane)
{
  g_signal_emit_by_name (abstract_pane, "close-pane");
}                        

void
codeslayer_abstract_pane_sync_registry (CodeSlayerAbstractPane *abstract_pane)
{
  CodeSlayerAbstractPanePrivate *priv;
  CodeSlayerProfile *profile;
  CodeSlayerRegistry *registry; 
  gchar *editor_value;
  
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);
  
  profile = codeslayer_profiles_get_profile (priv->profiles);
  registry = codeslayer_profile_get_registry (profile);

  editor_value = codeslayer_registry_get_string (registry, tab_position_key (abstract_pane));
                                                    
  if (!codeslayer_utils_has_text (editor_value))
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (priv->notebook), GTK_POS_TOP);                                                    

  if (g_strcmp0 (editor_value, "left") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (priv->notebook), GTK_POS_LEFT);
  else if (g_strcmp0 (editor_value, "right") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (priv->notebook), GTK_POS_RIGHT);
  else if (g_strcmp0 (editor_value, "top") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (priv->notebook), GTK_POS_TOP);
  else if (g_strcmp0 (editor_value, "bottom") == 0)
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (priv->notebook), GTK_POS_BOTTOM);
    
  if (editor_value)                                             
    g_free (editor_value);
}

static gint
compare_plugins (GtkWidget *item1, 
                 GtkWidget *item2, 
                 GtkWidget *notebook)
{
  return g_strcmp0 (gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (notebook), item1),
                    gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (notebook), item2));
}

static void
reorder_plugins (CodeSlayerAbstractPane *abstract_pane)
{
  CodeSlayerAbstractPanePrivate *priv;
  GList *tmp;
  int pos;
  
  priv = CODESLAYER_ABSTRACT_PANE_GET_PRIVATE (abstract_pane);
  
  pos = tab_start_sort (abstract_pane);
  
  priv->plugins = g_list_sort_with_data (priv->plugins, 
                                         (GCompareDataFunc) compare_plugins,
                                         priv->notebook);
  tmp = priv->plugins;

  while (tmp != NULL)
    {
      GtkWidget *item = tmp->data;
      gtk_notebook_reorder_child (GTK_NOTEBOOK (priv->notebook), item, pos);
      pos++;
      tmp = g_list_next (tmp);
    }
}

static gchar*
tab_position_key (CodeSlayerAbstractPane *abstract_pane)
{
  return CODESLAYER_ABSTRACT_PANE_GET_CLASS(abstract_pane)->abstract_tab_position_key (abstract_pane);
}

static gint
tab_start_sort (CodeSlayerAbstractPane *abstract_pane)
{
  return CODESLAYER_ABSTRACT_PANE_GET_CLASS(abstract_pane)->abstract_tab_start_sort (abstract_pane);
}
