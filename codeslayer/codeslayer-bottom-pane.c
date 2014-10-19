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

#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-registry.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-bottom-pane
 * @short_description: The bottom pane.
 * @title: CodeSlayerBottomPane
 * @include: codeslayer/codeslayer-bottom-pane.h
 */

static void    codeslayer_bottom_pane_class_init  (CodeSlayerBottomPaneClass *klass);
static void    codeslayer_bottom_pane_init        (CodeSlayerBottomPane      *bottom_pane);
static void    codeslayer_bottom_pane_finalize    (CodeSlayerBottomPane      *bottom_pane);

static void    create_close_button                (CodeSlayerBottomPane      *bottom_pane);
static gchar*  tab_position_key                   (CodeSlayerAbstractPane    *abstract_pane);
static gint    tab_start_sort                     (CodeSlayerAbstractPane    *abstract_pane);

#define CODESLAYER_BOTTOM_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_BOTTOM_PANE_TYPE, CodeSlayerBottomPanePrivate))

enum
{
  CLOSE_PANE,
  LAST_SIGNAL
};

G_DEFINE_TYPE (CodeSlayerBottomPane, codeslayer_bottom_pane, CODESLAYER_ABSTRACT_PANE_TYPE)

static void
codeslayer_bottom_pane_class_init (CodeSlayerBottomPaneClass *klass)
{
  CodeSlayerAbstractPaneClass *abstract_pane_class = (CodeSlayerAbstractPaneClass*) klass;
  abstract_pane_class->abstract_tab_position_key = tab_position_key;
  abstract_pane_class->abstract_tab_start_sort = tab_start_sort;

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_bottom_pane_finalize;
}

static void
codeslayer_bottom_pane_init (CodeSlayerBottomPane *bottom_pane) {}

static void
codeslayer_bottom_pane_finalize (CodeSlayerBottomPane *bottom_pane)
{
  G_OBJECT_CLASS (codeslayer_bottom_pane_parent_class)->finalize (G_OBJECT (bottom_pane));
}

/**
 * codeslayer_bottom_pane_new:
 * @profile: a #CodeSlayerProfile.
 *
 * Creates a new #CodeSlayerBottomPane.
 *
 * Returns: a new #CodeSlayerBottomPane. 
 */
GtkWidget*
codeslayer_bottom_pane_new (CodeSlayerProfile *profile)
{
  GtkWidget *bottom_pane;
  CodeSlayerRegistry *registry; 
  
  bottom_pane = g_object_new (codeslayer_bottom_pane_get_type (), NULL);
  
  codeslayer_abstract_pane_set_profile (CODESLAYER_ABSTRACT_PANE (bottom_pane), profile);
  
  create_close_button (CODESLAYER_BOTTOM_PANE (bottom_pane));
  codeslayer_abstract_pane_create_notebook (CODESLAYER_ABSTRACT_PANE (bottom_pane));

  registry = codeslayer_profile_get_registry (profile);

  g_signal_connect_swapped (G_OBJECT (registry), "registry-initialized",
                            G_CALLBACK (codeslayer_abstract_pane_sync_registry), CODESLAYER_ABSTRACT_PANE (bottom_pane));

  g_signal_connect_swapped (G_OBJECT (registry), "registry-changed",
                            G_CALLBACK (codeslayer_abstract_pane_sync_registry), bottom_pane);

  return bottom_pane;
}

static void
create_close_button (CodeSlayerBottomPane *bottom_pane)
{
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *image;
    
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);
  
  button = gtk_button_new ();
  image = gtk_image_new_from_icon_name (_("window-close"), GTK_ICON_SIZE_MENU);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  gtk_container_add (GTK_CONTAINER (button), image);
  codeslayer_utils_style_close_button (button);

  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 5);

  g_signal_connect_swapped (G_OBJECT (button), "button-press-event",
                            G_CALLBACK (codeslayer_abstract_pane_close), bottom_pane);
                            
  gtk_box_pack_start (GTK_BOX (bottom_pane), hbox, FALSE, FALSE, 0);  
}

static gchar* 
tab_position_key (CodeSlayerAbstractPane *abstract_pane)
{
  return CODESLAYER_REGISTRY_BOTTOM_PANE_TAB_POSITION;
}

static gint 
tab_start_sort (CodeSlayerAbstractPane *abstract_pane)
{
  return 0;
}
