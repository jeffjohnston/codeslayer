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

#include <codeslayer/codeslayer-group-properties.h>
#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-plugin.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-group-properties
 * @short_description: The groups properties.
 * @title: CodeSlayerGroupProperties
 * @include: codeslayer/codeslayer-groups-properties.h
 */

static void codeslayer_group_properties_class_init  (CodeSlayerGroupPropertiesClass *klass);
static void codeslayer_group_properties_init        (CodeSlayerGroupProperties      *group_properties);
static void codeslayer_group_properties_finalize    (CodeSlayerGroupProperties      *group_properties);

static void add_grid                               (CodeSlayerGroupProperties      *group_properties);

#define CODESLAYER_GROUP_PROPERTIES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_GROUP_PROPERTIES_TYPE, CodeSlayerGroupPropertiesPrivate))

typedef struct _CodeSlayerGroupPropertiesPrivate CodeSlayerGroupPropertiesPrivate;

struct _CodeSlayerGroupPropertiesPrivate
{
  CodeSlayerGroup *group;
  GtkWidget       *name_entry;
};

G_DEFINE_TYPE (CodeSlayerGroupProperties, codeslayer_group_properties, GTK_TYPE_VBOX)

static void
codeslayer_group_properties_class_init (CodeSlayerGroupPropertiesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_group_properties_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerGroupPropertiesPrivate));
}

static void
codeslayer_group_properties_init (CodeSlayerGroupProperties *group_properties)
{
  gtk_box_set_homogeneous (GTK_BOX (group_properties), FALSE);
  gtk_box_set_spacing (GTK_BOX (group_properties), 0);
}

static void
codeslayer_group_properties_finalize (CodeSlayerGroupProperties *group_properties)
{
  G_OBJECT_CLASS (codeslayer_group_properties_parent_class)->finalize (G_OBJECT (group_properties));
}

/**
 * codeslayer_group_properties_new:
 * @group: a #CodeSlayerGroup.
 *
 * Creates a new #CodeSlayerGroupProperties.
 *
 * Returns: a new #CodeSlayerGroupProperties. 
 */
GtkWidget*
codeslayer_group_properties_new (CodeSlayerGroup *group)
{
  CodeSlayerGroupPropertiesPrivate *priv;
  GtkWidget *group_properties;
  
  group_properties = g_object_new (codeslayer_group_properties_get_type (), NULL);
  priv = CODESLAYER_GROUP_PROPERTIES_GET_PRIVATE (group_properties);
  priv->group = group;
  
  add_grid (CODESLAYER_GROUP_PROPERTIES (group_properties));

  return group_properties;
}

static void
add_grid (CodeSlayerGroupProperties *group_properties)
{
  CodeSlayerGroupPropertiesPrivate *priv;
  GtkWidget *grid;
  GtkWidget *name_label;
  GtkWidget *name_entry;
  GtkWidget *folder_label;
  GtkWidget *folder_entry;
  gchar *file_path;
  const gchar *group_name;
  
  priv = CODESLAYER_GROUP_PROPERTIES_GET_PRIVATE (group_properties);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 2);

  name_label = gtk_label_new (_("Name:"));
  gtk_misc_set_alignment (GTK_MISC (name_label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (name_label), 4, 0);
  gtk_grid_attach (GTK_GRID (grid), name_label, 0, 0, 1, 1);

  name_entry = gtk_entry_new ();
  priv->name_entry = name_entry;
  gtk_entry_set_activates_default (GTK_ENTRY (name_entry), TRUE);
  gtk_entry_set_width_chars (GTK_ENTRY (name_entry), 50);  
  gtk_grid_attach_next_to (GTK_GRID (grid), name_entry, name_label, 
                           GTK_POS_RIGHT, 1, 1);

  folder_label = gtk_label_new (_("Folder:"));
  gtk_misc_set_alignment (GTK_MISC (folder_label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (folder_label), 4, 0);
  gtk_grid_attach (GTK_GRID (grid), folder_label, 0, 1, 1, 1);

  folder_entry = gtk_entry_new ();
  gtk_entry_set_width_chars (GTK_ENTRY (folder_entry), 50);
  gtk_widget_set_sensitive (folder_entry, FALSE);
  gtk_grid_attach_next_to (GTK_GRID (grid), folder_entry, folder_label, 
                           GTK_POS_RIGHT, 1, 1);
                    
  group_name = codeslayer_group_get_name (priv->group);
  
  gtk_entry_set_text (GTK_ENTRY (name_entry), group_name);                    
                    
  file_path = g_build_filename (g_get_home_dir (), 
                                CODESLAYER_HOME, 
                                CODESLAYER_GROUPS_DIR, 
                                group_name,
                                NULL);
                    
  gtk_entry_set_text (GTK_ENTRY (folder_entry), file_path);
  g_free (file_path);
                    
  gtk_box_pack_start (GTK_BOX (group_properties), grid, FALSE, FALSE, 2);   
}

const gchar*
codeslayer_group_properties_get_name (CodeSlayerGroupProperties *group_properties)
{
  CodeSlayerGroupPropertiesPrivate *priv;
  priv = CODESLAYER_GROUP_PROPERTIES_GET_PRIVATE (group_properties);
  return gtk_entry_get_text (GTK_ENTRY (priv->name_entry));
}
