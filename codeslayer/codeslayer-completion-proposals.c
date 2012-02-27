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

#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-completion-proposals.h>

/**
 * SECTION:codeslayer-proposals
 * @short_description: Contains the list of proposals.
 * @title: CodeSlayerCompletionProposals
 * @include: codeslayer/codeslayer-completion-proposals.h
 */

static void codeslayer_completion_proposals_class_init    (CodeSlayerCompletionProposalsClass *klass);
static void codeslayer_completion_proposals_init          (CodeSlayerCompletionProposals      *proposals);
static void codeslayer_completion_proposals_finalize      (CodeSlayerCompletionProposals      *proposals);
static void codeslayer_completion_proposals_get_property  (GObject                            *object, 
                                                           guint                                prop_id, 
                                                           GValue                             *value, 
                                                           GParamSpec                         *pspec);
static void codeslayer_completion_proposals_set_property  (GObject                            *object, 
                                                           guint                               prop_id, 
                                                           const GValue                       *value, 
                                                           GParamSpec                         *pspec);

#define CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_COMPLETION_PROPOSALS_TYPE, CodeSlayerCompletionProposalsPrivate))

typedef struct _CodeSlayerCompletionProposalsPrivate CodeSlayerCompletionProposalsPrivate;

struct _CodeSlayerCompletionProposalsPrivate
{
  GList       *list;
  GtkTextMark *mark;
};

enum
{
  PROP_0,
  PROP_LIST,
  PROP_MARK
};

G_DEFINE_TYPE (CodeSlayerCompletionProposals, codeslayer_completion_proposals, G_TYPE_OBJECT)
     
static void
codeslayer_completion_proposals_class_init (CodeSlayerCompletionProposalsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_completion_proposals_finalize;

  gobject_class->get_property = codeslayer_completion_proposals_get_property;
  gobject_class->set_property = codeslayer_completion_proposals_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerCompletionProposalsPrivate));

  /**
	 * CodeSlayerCompletionProposals:list:
	 *
	 * The list of #CodeSlayerCompletionProposal objects.
	 */
  g_object_class_install_property (gobject_class, 
                                   PROP_LIST,
                                   g_param_spec_pointer ("list", 
                                                         "List", 
                                                         "List Object",
                                                         G_PARAM_READWRITE));

  /**
	 * CodeSlayerCompletionProposals:mark:
	 *
	 * The GtkTextMark.
	 */
  g_object_class_install_property (gobject_class, 
                                   PROP_MARK,
                                   g_param_spec_pointer ("mark", 
                                                         "Mark", 
                                                         "Mark Object",
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_completion_proposals_init (CodeSlayerCompletionProposals *proposals)
{
  CodeSlayerCompletionProposalsPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE (proposals);
  priv->list = NULL;
  priv->mark = NULL;
}

static void
codeslayer_completion_proposals_finalize (CodeSlayerCompletionProposals *proposals)
{
  CodeSlayerCompletionProposalsPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE (proposals);
  if (priv->list)
    {
      g_list_foreach (priv->list, (GFunc) g_object_unref, NULL);
      g_list_free (priv->list);
    }
  G_OBJECT_CLASS (codeslayer_completion_proposals_parent_class)->finalize (G_OBJECT (proposals));
}

static void
codeslayer_completion_proposals_get_property (GObject    *object, 
                                              guint       prop_id,
                                              GValue     *value, 
                                              GParamSpec *pspec)
{
  CodeSlayerCompletionProposalsPrivate *priv;
  CodeSlayerCompletionProposals *proposals;

  proposals = CODESLAYER_COMPLETION_PROPOSALS (object);
  priv = CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE (proposals);

  switch (prop_id)
    {
    case PROP_LIST:
      g_value_set_pointer (value, priv->list);
      break;
    case PROP_MARK:
      g_value_set_pointer (value, priv->mark);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_completion_proposals_set_property (GObject      *object, 
                                              guint         prop_id,
                                              const GValue *value,
                                              GParamSpec   *pspec)
{
  CodeSlayerCompletionProposals *proposals; 
  proposals = CODESLAYER_COMPLETION_PROPOSALS (object);

  switch (prop_id)
    {
    case PROP_LIST:
      codeslayer_completion_proposals_set_list (proposals, g_value_get_pointer (value));
      break;
    case PROP_MARK:
      codeslayer_completion_proposals_set_mark (proposals, g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_completion_proposals_new:
 *
 * Creates a new #CodeSlayerCompletionProposals.
 *
 * Returns: a new #CodeSlayerCompletionProposals. 
 */
CodeSlayerCompletionProposals*
codeslayer_completion_proposals_new (void)
{
  return CODESLAYER_COMPLETION_PROPOSALS (g_object_new (codeslayer_completion_proposals_get_type (), NULL));
}

/**
 * codeslayer_completion_proposals_get_list:
 * @proposals: a #CodeSlayerCompletionProposals.
 *
 * Returns: The list of #CodeSlayerCompletionProposal objects.
 */
GList*
codeslayer_completion_proposals_get_list (CodeSlayerCompletionProposals *proposals)
{
  return CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE (proposals)->list;
}

/**
 * codeslayer_completion_proposals_set_list:
 * @proposals: a #CodeSlayerCompletionProposals.
 * @list: the list of #CodeSlayerCompletionProposal objects to add.
 */
void
codeslayer_completion_proposals_set_list (CodeSlayerCompletionProposals *proposals, 
                                          GList                         *list)
{
  CodeSlayerCompletionProposalsPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE (proposals);
  priv->list = list;
  g_list_foreach (priv->list, (GFunc) g_object_ref_sink, NULL);
}

/**
 * codeslayer_completion_proposals_get_mark:
 * @proposals: a #CodeSlayerCompletionProposals.
 *
 * Returns: The GtkTextMark that starts the completion list.
 */
GtkTextMark*
codeslayer_completion_proposals_get_mark (CodeSlayerCompletionProposals *proposals)
{
  CodeSlayerCompletionProposalsPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE (proposals);
  return priv->mark;
}

/**
 * codeslayer_completion_proposals_set_mark:
 * @proposals: a #CodeSlayerCompletionProposals.
 * @mark: The GtkTextMark that starts the completion list.
 */
void
codeslayer_completion_proposals_set_mark (CodeSlayerCompletionProposals *proposals, 
                                          GtkTextMark                   *mark)
{
  CodeSlayerCompletionProposalsPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE (proposals);
  priv->mark = mark;
}                                          

/**
 * codeslayer_completion_proposals_add_proposal:
 * @proposals: a #CodeSlayerCompletionProposals.
 * @proposal: the #CodeSlayerCompletionProposal to add.
 */
void
codeslayer_completion_proposals_add_proposal (CodeSlayerCompletionProposals *proposals,
                                              CodeSlayerCompletionProposal  *proposal)
{
  CodeSlayerCompletionProposalsPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSALS_GET_PRIVATE (proposals);
  priv->list = g_list_prepend (priv->list, proposal);
  g_object_ref_sink (G_OBJECT (proposal));
}
