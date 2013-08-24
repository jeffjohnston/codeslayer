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

#include <string.h>
#include <codeslayer/codeslayer-notebook-tab.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-notebook-tab
 * @short_description: The notebook page label.
 * @title: CodeSlayerNotebookTab
 * @include: codeslayer/codeslayer-notebook-tab.h
 *
 * The tab is appended to the notebook page as a way to give the page a 
 * label. This label can also communicate whether or not the editor needs 
 * to be saved. The tab itself allows a page to be closed.
 */

static void codeslayer_notebook_tab_class_init    (CodeSlayerNotebookTabClass *klass);
static void codeslayer_notebook_tab_init          (CodeSlayerNotebookTab      *notebook_tab);
static void codeslayer_notebook_tab_finalize      (CodeSlayerNotebookTab      *notebook_tab);
static void codeslayer_notebook_tab_get_property  (GObject                    *object, 
                                                   guint                       prop_id, 
                                                   GValue                     *value, 
                                                   GParamSpec                 *pspec);
static void codeslayer_notebook_tab_set_property  (GObject                    *object, 
                                                   guint                       prop_id, 
                                                   const GValue               *value, 
                                                   GParamSpec                 *pspec);
static GtkWidget *decorate_popup_menu             (CodeSlayerNotebookTab      *notebook_tab, 
                                                   GtkWidget                  *label);
static gboolean button_press_action                   (CodeSlayerNotebookTab      *notebook_tab, 
                                                   GdkEventButton             *event);
static void close_editor_action                   (CodeSlayerNotebookTab      *notebook_tab);
static void close_all_editors_action              (CodeSlayerNotebookTab      *notebook_tab);
static void close_other_editors_action            (CodeSlayerNotebookTab      *notebook_tab);
static void close_right_editors_action            (CodeSlayerNotebookTab      *notebook_tab);
static void close_left_editors_action             (CodeSlayerNotebookTab      *notebook_tab);
static void set_tooltip                           (CodeSlayerNotebookTab      *notebook_tab);

#define CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_NOTEBOOK_TAB_TYPE, CodeSlayerNotebookTabPrivate))

typedef struct _CodeSlayerNotebookTabPrivate CodeSlayerNotebookTabPrivate;

struct _CodeSlayerNotebookTabPrivate
{
  GtkWidget *menu;
  GtkWidget *close_all_editors_menu_item;
  GtkWidget *close_other_editors_menu_item;
  GtkWidget *close_right_editors_menu_item;
  GtkWidget *close_left_editors_menu_item;
  GtkWidget *notebook_page;
  GtkWidget *notebook;
  GtkWidget *label;
  GtkWidget *button;
  gchar     *file_name;
};

enum
{
  PROP_0,
  PROP_NOTEBOOK_PAGE
};

enum
{
  SELECT_EDITOR,
  CLOSE_EDITOR,
  CLOSE_ALL_EDITORS,
  CLOSE_OTHER_EDITORS,
  CLOSE_RIGHT_EDITORS,
  CLOSE_LEFT_EDITORS,
  LAST_SIGNAL
};

static guint codeslayer_notebook_tab_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerNotebookTab, codeslayer_notebook_tab, GTK_TYPE_HBOX)
     
static void
codeslayer_notebook_tab_class_init (CodeSlayerNotebookTabClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  /**
   * CodeSlayerNotebookTab::select-editor
   * @codeslayernotebooktab: the tab that received the signal
   *
   * The ::select-editor signal is a request to close the active editor.
   */
  codeslayer_notebook_tab_signals[SELECT_EDITOR] =
    g_signal_new ("select-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookTabClass, select_editor),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerNotebookTab::close-editor
   * @codeslayernotebooktab: the tab that received the signal
   *
   * The ::close-editor signal is a request to close the active editor.
   */
  codeslayer_notebook_tab_signals[CLOSE_EDITOR] =
    g_signal_new ("close-editor", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookTabClass, close_editor),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerNotebookTab::close-all-editors
   * @codeslayernotebooktab: the tab that received the signal
   *
   * The ::close-all-editors signal is a request to close all the editors.
   */
  codeslayer_notebook_tab_signals[CLOSE_ALL_EDITORS] =
    g_signal_new ("close-all-editors", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookTabClass, close_all_editors), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerNotebookTab::close-other-editors
   * @codeslayernotebooktab: the tab that received the signal
   *
   * The ::close-other-editors signal is a request to close all editors except the 
   * active one.
   */
  codeslayer_notebook_tab_signals[CLOSE_OTHER_EDITORS] =
    g_signal_new ("close-other-editors", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookTabClass, close_other_editors), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerNotebookTab::close-right-editors
   * @codeslayernotebooktab: the tab that received the signal
   *
   * The ::close-right-editors signal is a request to close all editors to the right of 
   * the active one.
   */
  codeslayer_notebook_tab_signals[CLOSE_RIGHT_EDITORS] =
    g_signal_new ("close-right-editors", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookTabClass, close_right_editors), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * CodeSlayerNotebookTab::close-left-editors
   * @codeslayernotebooktab: the tab that received the signal
   *
   * The ::close-left-editors signal is a request to close all editors to the left of the 
   * active one.
   */
  codeslayer_notebook_tab_signals[CLOSE_LEFT_EDITORS] =
    g_signal_new ("close-left-editors", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerNotebookTabClass, close_left_editors), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_notebook_tab_finalize;

  gobject_class->get_property = codeslayer_notebook_tab_get_property;
  gobject_class->set_property = codeslayer_notebook_tab_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerNotebookTabPrivate));

  /**
   * CodeSlayerNotebookTab:notebook-page:
   *
   * A #CodeSlayerNotebookPage.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_NOTEBOOK_PAGE,
                                   g_param_spec_pointer ("notebook_page",
                                                         "Notebook Page",
                                                         "Notebook Page",
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_notebook_tab_init (CodeSlayerNotebookTab *notebook_tab)
{
  gtk_box_set_homogeneous (GTK_BOX (notebook_tab), FALSE);
  gtk_box_set_spacing (GTK_BOX (notebook_tab), 0);
}

static void
codeslayer_notebook_tab_finalize (CodeSlayerNotebookTab *notebook_tab)
{
  CodeSlayerNotebookTabPrivate *priv;
  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  g_free (priv->file_name);
  G_OBJECT_CLASS (codeslayer_notebook_tab_parent_class)->finalize ( G_OBJECT (notebook_tab));
}

static void
codeslayer_notebook_tab_get_property (GObject    *object, 
                                      guint       prop_id, 
                                      GValue     *value, 
                                      GParamSpec *pspec)
{
  CodeSlayerNotebookTabPrivate *priv;
  CodeSlayerNotebookTab *notebook_tab;
  
  notebook_tab = CODESLAYER_NOTEBOOK_TAB (object);
  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);

  switch (prop_id)
    {
    case PROP_NOTEBOOK_PAGE:
      g_value_set_pointer (value, priv->notebook_page);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_notebook_tab_set_property (GObject      *object, 
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  CodeSlayerNotebookTab *notebook_tab;
  
  notebook_tab = CODESLAYER_NOTEBOOK_TAB (object);

  switch (prop_id)
    {
    case PROP_NOTEBOOK_PAGE:
      codeslayer_notebook_tab_set_notebook_page (notebook_tab, 
                                                 g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_notebook_tab_new:
 * @notebook: a #GtkWidget.
 * @file_name: the display name for the tab is the file name.
 *
 * Creates a new #CodeSlayerNotebookTab.
 *
 * Returns: a new #CodeSlayerNotebookTab. 
 */
GtkWidget*
codeslayer_notebook_tab_new (GtkWidget *notebook, 
                             gchar     *file_name)
{
  CodeSlayerNotebookTabPrivate *priv;
  GtkWidget *notebook_tab;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *image;
  
  notebook_tab = g_object_new (codeslayer_notebook_tab_get_type (), NULL);
  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  priv->file_name = g_strdup (file_name);
  priv->notebook = notebook;

  label = gtk_label_new (file_name);
  priv->label = label;

  button = gtk_button_new ();
  priv->button = button;

  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (button), image);
  codeslayer_utils_style_close_button (button);

  g_signal_connect_swapped (G_OBJECT (button), "clicked",
                            G_CALLBACK (close_editor_action), notebook_tab);

  gtk_box_pack_start (GTK_BOX (notebook_tab), 
                      decorate_popup_menu (CODESLAYER_NOTEBOOK_TAB (notebook_tab), label), 
                      TRUE, TRUE, 1);
  gtk_box_pack_start (GTK_BOX (notebook_tab), button, FALSE, FALSE, 1);

  return notebook_tab;
}

static GtkWidget *
decorate_popup_menu (CodeSlayerNotebookTab *notebook_tab, 
                     GtkWidget             *label)
{
  CodeSlayerNotebookTabPrivate *priv;
  GtkWidget *menu;
  GtkWidget *close_all_editors_menu_item;
  GtkWidget *close_other_editors_menu_item;
  GtkWidget *close_right_editors_menu_item;
  GtkWidget *close_left_editors_menu_item;
  GtkWidget *event_box;
  
  menu = gtk_menu_new ();
  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  priv->menu = menu;

  close_all_editors_menu_item = gtk_menu_item_new_with_label (_("Close All Editors"));
  close_other_editors_menu_item = gtk_menu_item_new_with_label (_("Close Other Editors"));
  close_right_editors_menu_item = gtk_menu_item_new_with_label (_("Close Right Editors"));
  close_left_editors_menu_item = gtk_menu_item_new_with_label (_("Close Left Editors"));

  priv->close_other_editors_menu_item = close_other_editors_menu_item;
  priv->close_all_editors_menu_item = close_all_editors_menu_item;
  priv->close_right_editors_menu_item = close_right_editors_menu_item;
  priv->close_left_editors_menu_item = close_left_editors_menu_item;

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), close_all_editors_menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), close_other_editors_menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), close_right_editors_menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), close_left_editors_menu_item);

  gtk_menu_attach_to_widget (GTK_MENU (menu), GTK_WIDGET (label), NULL);
  gtk_widget_show_all (menu);

  event_box = gtk_event_box_new ();
  gtk_event_box_set_above_child (GTK_EVENT_BOX (event_box), FALSE);
  gtk_container_add (GTK_CONTAINER (event_box), label);
  gtk_widget_set_events (event_box, GDK_BUTTON_PRESS_MASK);

  g_signal_connect_swapped (G_OBJECT (event_box), "button-press-event",
                            G_CALLBACK (button_press_action), notebook_tab);
  g_signal_connect_swapped (G_OBJECT (close_all_editors_menu_item), "activate",
                            G_CALLBACK (close_all_editors_action), notebook_tab);
  g_signal_connect_swapped (G_OBJECT (close_other_editors_menu_item), "activate",
                            G_CALLBACK (close_other_editors_action), notebook_tab);
  g_signal_connect_swapped (G_OBJECT (close_right_editors_menu_item), "activate",
                            G_CALLBACK (close_right_editors_action), notebook_tab);
  g_signal_connect_swapped (G_OBJECT (close_left_editors_menu_item), "activate",
                            G_CALLBACK (close_left_editors_action), notebook_tab);

  return event_box;
}

static gboolean
button_press_action (CodeSlayerNotebookTab *notebook_tab, 
                     GdkEventButton        *event)
{
  CodeSlayerNotebookTabPrivate *priv;

  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);

  if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS))
    {
      g_signal_emit_by_name ((gpointer) notebook_tab, "select-editor");
    }
  else if ((event->button == 3) && (event->type == GDK_BUTTON_PRESS))
    {
      gboolean sensitive = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) > 1;
      gtk_widget_set_sensitive (priv->close_all_editors_menu_item, sensitive);
      gtk_widget_set_sensitive (priv->close_other_editors_menu_item, sensitive);
      gtk_widget_set_sensitive (priv->close_right_editors_menu_item, sensitive);
      gtk_widget_set_sensitive (priv->close_left_editors_menu_item, sensitive);
      gtk_menu_popup (GTK_MENU (priv->menu), NULL, NULL, NULL, NULL,
                      event->button, event->time);
      return TRUE;
    }

  return FALSE;
}

static void
close_editor_action (CodeSlayerNotebookTab *notebook_tab)
{
  g_signal_emit_by_name ((gpointer) notebook_tab, "close-editor");
}

static void
close_all_editors_action (CodeSlayerNotebookTab *notebook_tab)
{
  g_signal_emit_by_name ((gpointer) notebook_tab, "close-all-editors");
}

static void
close_other_editors_action (CodeSlayerNotebookTab *notebook_tab)
{
  g_signal_emit_by_name ((gpointer) notebook_tab, "close-other-editors");
}

static void
close_right_editors_action (CodeSlayerNotebookTab *notebook_tab)
{
  g_signal_emit_by_name ((gpointer) notebook_tab, "close-right-editors");
}

static void
close_left_editors_action (CodeSlayerNotebookTab *notebook_tab)
{
  g_signal_emit_by_name ((gpointer) notebook_tab, "close-left-editors");
}

/**
 * codeslayer_notebook_tab_show_buffer_dirty:
 * @notebook_tab: a #CodeSlayerNotebookTab.
 * 
 * The tab label needs to be show that the editor needs to be saved.
 */
void
codeslayer_notebook_tab_show_buffer_dirty (CodeSlayerNotebookTab *notebook_tab)
{
  CodeSlayerNotebookTabPrivate *priv;
  gchar *text;
  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  text = g_strconcat ("*", priv->file_name, NULL);
  gtk_label_set_text (GTK_LABEL (priv->label), text);
  g_free (text);
}

/**
 * codeslayer_notebook_tab_show_buffer_clean:
 * @notebook_tab: a #CodeSlayerNotebookTab.
 * 
 * The tab label needs to be show that the editor no longer needs to be saved.
 */
void
codeslayer_notebook_tab_show_buffer_clean (CodeSlayerNotebookTab *notebook_tab)
{
  CodeSlayerNotebookTabPrivate *priv;
  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  gtk_label_set_text (GTK_LABEL (priv->label), priv->file_name);
}

/**
 * codeslayer_notebook_tab_get_notebook_page:
 * @notebook_tab: a #CodeSlayerNotebookTab.
 *
 * Returns: the notebook page contained with the tab.
 */
GtkWidget*
codeslayer_notebook_tab_get_notebook_page (CodeSlayerNotebookTab *notebook_tab)
{
  return CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab)->notebook_page;
}

/**
 * codeslayer_notebook_tab_set_notebook_page:
 * @notebook_tab: a #CodeSlayerNotebookTab.
 * @notebook_page: a #CodeSlayerNotebookPage.
 */
void
codeslayer_notebook_tab_set_notebook_page (CodeSlayerNotebookTab  *notebook_tab,
                                           GtkWidget              *notebook_page)
{
  CodeSlayerNotebookTabPrivate *priv;
  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  priv->notebook_page = notebook_page;
  set_tooltip (notebook_tab);
}

/**
 * codeslayer_notebook_tab_set_label_name:
 * @notebook_tab: a #CodeSlayerNotebookTab.
 * @file_name: the display name for the tab is the file name.
 *
 * Change the label name of the tab.
 */
void
codeslayer_notebook_tab_set_label_name (CodeSlayerNotebookTab *notebook_tab,
                                        gchar                 *file_name)
{
  CodeSlayerNotebookTabPrivate *priv;
  const gchar *label_text;
  
  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);

  label_text = gtk_label_get_text (GTK_LABEL (priv->label));

  if (g_ascii_strncasecmp (label_text, "*", 1) == 0)
    {
      gchar *text = g_strconcat ("*", file_name, NULL);
      gtk_label_set_text (GTK_LABEL (priv->label), text);
      g_free (text);
    }
  else
    {
      gtk_label_set_text (GTK_LABEL (priv->label), file_name);
    }

  set_tooltip (notebook_tab);

  g_free (priv->file_name);
  priv->file_name = g_strdup (file_name);
}

static void
set_tooltip (CodeSlayerNotebookTab *notebook_tab)
{
  CodeSlayerNotebookTabPrivate *priv;
  CodeSlayerDocument *document;
  const gchar *file_path;

  priv = CODESLAYER_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  
  document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (priv->notebook_page));
  file_path = codeslayer_document_get_file_path (document);

  if (file_path != NULL)
    gtk_widget_set_tooltip_text (priv->label, file_path);
}
