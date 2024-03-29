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

#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-projects.h>

/**
 * SECTION:codeslayer-side-pane
 * @short_description: The left hand side pane.
 * @title: CodeSlayerSidePane
 * @include: codeslayer/codeslayer-side-pane.h
 */

static void    codeslayer_side_pane_class_init  (CodeSlayerSidePaneClass *klass);
static void    codeslayer_side_pane_init        (CodeSlayerSidePane      *side_pane);
static void    codeslayer_side_pane_finalize    (CodeSlayerSidePane      *side_pane);

static gchar*  tab_position_key                 (CodeSlayerAbstractPane  *abstract_pane);
static gint    tab_start_sort                   (CodeSlayerAbstractPane  *abstract_pane);

#define CODESLAYER_SIDE_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_SIDE_PANE_TYPE, CodeSlayerSidePanePrivate))

G_DEFINE_TYPE (CodeSlayerSidePane, codeslayer_side_pane, CODESLAYER_ABSTRACT_PANE_TYPE)

static void
codeslayer_side_pane_class_init (CodeSlayerSidePaneClass *klass)
{
  CodeSlayerAbstractPaneClass *abstract_pane_class = (CodeSlayerAbstractPaneClass*) klass;
  abstract_pane_class->abstract_tab_position_key = tab_position_key;
  abstract_pane_class->abstract_tab_start_sort = tab_start_sort;

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_side_pane_finalize;
}

static void
codeslayer_side_pane_init (CodeSlayerSidePane *side_pane) {}

static void
codeslayer_side_pane_finalize (CodeSlayerSidePane *side_pane)
{
  G_OBJECT_CLASS (codeslayer_side_pane_parent_class)->finalize (G_OBJECT (side_pane));
}

/**
 * codeslayer_side_pane_new:
 * @profile: a #CodeSlayerProfile.
 *
 * Creates a new #CodeSlayerSidePane.
 *
 * Returns: a new #CodeSlayerSidePane. 
 */
GtkWidget*
codeslayer_side_pane_new (CodeSlayerProfile *profile)
{
  GtkWidget *side_pane;
  CodeSlayerRegistry *registry; 
  
  side_pane = g_object_new (codeslayer_side_pane_get_type (), NULL);

  registry = codeslayer_profile_get_registry (profile);
  
  codeslayer_abstract_pane_set_profile (CODESLAYER_ABSTRACT_PANE (side_pane), profile);
  codeslayer_abstract_pane_create_notebook (CODESLAYER_ABSTRACT_PANE (side_pane));
  
  g_signal_connect_swapped (G_OBJECT (registry), "registry-initialized",
                            G_CALLBACK ( codeslayer_abstract_pane_sync_registry), CODESLAYER_ABSTRACT_PANE (side_pane));
  
  g_signal_connect_swapped (G_OBJECT (registry), "registry-changed",
                            G_CALLBACK (codeslayer_abstract_pane_sync_registry), side_pane);

  return side_pane;
}

static gchar* 
tab_position_key (CodeSlayerAbstractPane *abstract_pane)
{
  return CODESLAYER_REGISTRY_SIDE_PANE_TAB_POSITION;
}

static gint 
tab_start_sort (CodeSlayerAbstractPane *abstract_pane)
{
  return 1;
}
