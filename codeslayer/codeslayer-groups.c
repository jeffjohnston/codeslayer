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

#include <codeslayer/codeslayer-groups.h>
#include <codeslayer/codeslayer-utils.h>

/**
 * SECTION:codeslayer-groups
 * @short_description: Contains the list of groups.
 * @title: CodeSlayerGroups
 * @include: codeslayer/codeslayer-groups.h
 */

static void codeslayer_groups_class_init         (CodeSlayerGroupsClass *klass);
static void codeslayer_groups_init               (CodeSlayerGroups      *groups);
static void codeslayer_groups_finalize           (CodeSlayerGroups      *groups);
static void codeslayer_groups_get_property       (GObject               *object, 
                                                  guint                  prop_id, 
                                                  GValue                *value, 
                                                  GParamSpec            *pspec);
static void codeslayer_groups_set_property       (GObject               *object, 
                                                  guint                  prop_id, 
                                                  const GValue          *value, 
                                                  GParamSpec            *pspec);
static void codeslayer_groups_remove_all_groups  (CodeSlayerGroups      *groups);

#define CODESLAYER_GROUPS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_GROUPS_TYPE, CodeSlayerGroupsPrivate))

typedef struct _CodeSlayerGroupsPrivate CodeSlayerGroupsPrivate;

struct _CodeSlayerGroupsPrivate
{
  GList           *list;
  CodeSlayerGroup *active;
};

enum
{
  PROP_0,
  PROP_LIST,
  PROP_ACTIVE
};

G_DEFINE_TYPE (CodeSlayerGroups, codeslayer_groups, G_TYPE_OBJECT)
     
static void
codeslayer_groups_class_init (CodeSlayerGroupsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_groups_finalize;

  gobject_class->get_property = codeslayer_groups_get_property;
  gobject_class->set_property = codeslayer_groups_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerGroupsPrivate));

  /**
	 * CodeSlayerGroups:list:
	 *
	 * The list of #CodeSlayerGroup objects in the manager.
	 */
  g_object_class_install_property (gobject_class, 
                                   PROP_LIST,
                                   g_param_spec_pointer ("list", 
                                                         "List", 
                                                         "List Object",
                                                         G_PARAM_READWRITE));

  /**
	 * CodeSlayerGroups:active:
	 *
	 * The currently selected group.
	 */
  g_object_class_install_property (gobject_class, 
                                   PROP_ACTIVE,
                                   g_param_spec_object ("active",
                                                        "Active",
                                                        "Active Group",
                                                        CODESLAYER_GROUP_TYPE,
                                                        G_PARAM_READWRITE));
}

static void
codeslayer_groups_init (CodeSlayerGroups *groups)
{
  CodeSlayerGroupsPrivate *priv;
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);
  priv->list = NULL;
}

static void
codeslayer_groups_finalize (CodeSlayerGroups *groups)
{
  CodeSlayerGroupsPrivate *priv;
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);
  if (priv->list)
    {
      codeslayer_groups_remove_all_groups (groups);
      g_list_free (priv->list);
    }
  G_OBJECT_CLASS (codeslayer_groups_parent_class)->finalize (G_OBJECT (groups));
}

static void
codeslayer_groups_get_property (GObject    *object, 
                                guint       prop_id,
                                GValue     *value, 
                                GParamSpec *pspec)
{
  CodeSlayerGroupsPrivate *priv;
  CodeSlayerGroups *groups;

  groups = CODESLAYER_GROUPS (object);
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);

  switch (prop_id)
    {
    case PROP_LIST:
      g_value_set_pointer (value, priv->list);
      break;
    case PROP_ACTIVE:
      g_value_set_pointer (value, priv->active);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_groups_set_property (GObject      *object, 
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  CodeSlayerGroups *groups; 
  groups = CODESLAYER_GROUPS (object);

  switch (prop_id)
    {
    case PROP_LIST:
      codeslayer_groups_set_list (groups, g_value_get_pointer (value));
      break;
    case PROP_ACTIVE:
      codeslayer_groups_set_active_group (groups, CODESLAYER_GROUP (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_groups_new:
 *
 * Creates a new #CodeSlayerGroups.
 *
 * Returns: a new #CodeSlayerGroups. 
 */
CodeSlayerGroups*
codeslayer_groups_new (void)
{
  return CODESLAYER_GROUPS (g_object_new (codeslayer_groups_get_type (), NULL));
}

/**
 * codeslayer_groups_get_list:
 * @groups: a #CodeSlayerGroups.
 *
 * Returns: The list of #CodeSlayerGroup objects within the manager.
 */
GList*
codeslayer_groups_get_list (CodeSlayerGroups *groups)
{
  return CODESLAYER_GROUPS_GET_PRIVATE (groups)->list;
}

/**
 * codeslayer_groups_set_list:
 * @groups: a #CodeSlayerGroups.
 * @list: the list of #CodeSlayerGroup objects to add to the manager.
 */
void
codeslayer_groups_set_list (CodeSlayerGroups *groups, 
                            GList            *list)
{
  CodeSlayerGroupsPrivate *priv;
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);
  priv->list = list;
}

/**
 * codeslayer_groups_add_group:
 * @groups: a #CodeSlayerGroups.
 * @group: the #CodeSlayerGroup to add to the manager.
 */
void
codeslayer_groups_add_group (CodeSlayerGroups *groups,
                             CodeSlayerGroup  *group)
{
  CodeSlayerGroupsPrivate *priv;
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);
  priv->list = g_list_prepend (priv->list, group);
  g_object_ref_sink (G_OBJECT (group));
}

/**
 * codeslayer_groups_remove_group:
 * @groups: a #CodeSlayerGroups.
 * @group: the #CodeSlayerGroup to remove from the manager.
 */
void
codeslayer_groups_remove_group (CodeSlayerGroups *groups,
                                CodeSlayerGroup  *group)
{
  CodeSlayerGroupsPrivate *priv;
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);
  priv->list = g_list_remove (priv->list, group);
  g_object_unref (group);
}

/**
 * codeslayer_groups_get_active_group:
 * @groups: a #CodeSlayerGroups.
 *
 * Returns: the active group in the manager.
 */
CodeSlayerGroup*
codeslayer_groups_get_active_group (CodeSlayerGroups *groups)
{
  CodeSlayerGroupsPrivate *priv;  
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);  
  if (priv->active == NULL) // for backwards compatibility
    codeslayer_groups_set_active_group (groups, priv->list->data);
  return priv->active;
}

/**
 * codeslayer_groups_set_active_group:
 * @groups: a #CodeSlayerGroups.
 * @group: the #CodeSlayerGroup to set as active in the manager.
 */
void
codeslayer_groups_set_active_group (CodeSlayerGroups *groups,
                                    CodeSlayerGroup  *group)
{
  CodeSlayerGroupsPrivate *priv;
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);
  priv->active = group;
}

/**
 * codeslayer_groups_find_group:
 * @groups: a #CodeSlayerGroups.
 * @name: the name of the #CodeSlayerGroup to find.
 *
 * Returns: the group found by name. Will return NULL if the 
 *                 group specified by name is not found.
 */
CodeSlayerGroup*
codeslayer_groups_find_group (CodeSlayerGroups *groups,
                              const gchar      *name)
{
  CodeSlayerGroupsPrivate *priv;
  GList *list;

  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);

  list = priv->list;
  while (list != NULL)
    {
      CodeSlayerGroup *group = list->data;
      if (g_strcmp0 (codeslayer_group_get_name (group), name) == 0)
        return group;
      list = g_list_next (list);
    }
  return NULL;
}

/**
 * codeslayer_groups_find_next_group:
 * @groups: a #CodeSlayerGroups.
 * @name: the name of the #CodeSlayerGroup to start the search from.
 *
 * Returns: the next logical group within the list. Will return NULL if the 
 *          group specified by name is not found.
 */
CodeSlayerGroup*
codeslayer_groups_find_next_group (CodeSlayerGroups *groups,
                                   const gchar      *name)
{
  CodeSlayerGroupsPrivate *priv;
  GList *list;

  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);

  list = priv->list;
  while (list != NULL)
    {
      CodeSlayerGroup *group = list->data;
      if (!g_strcmp0 (codeslayer_group_get_name (group), name) == 0)
        return group;
      list = g_list_next (list);
    }
  return NULL;
}

static void
codeslayer_groups_remove_all_groups (CodeSlayerGroups *groups)
{
  CodeSlayerGroupsPrivate *priv;
  priv = CODESLAYER_GROUPS_GET_PRIVATE (groups);
  g_list_foreach (priv->list, (GFunc) g_object_unref, NULL);
  priv->list = g_list_remove_all (priv->list, NULL);
}
