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
#include <codeslayer/codeslayer-completion-proposal.h>

static void codeslayer_completion_proposal_class_init  (CodeSlayerCompletionProposalClass *klass);
static void codeslayer_completion_proposal_init        (CodeSlayerCompletionProposal      *proposal);
static void codeslayer_completion_proposal_finalize    (CodeSlayerCompletionProposal      *proposal);

#define CODESLAYER_COMPLETION_PROPOSAL_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_COMPLETION_PROPOSAL_TYPE, CodeSlayerCompletionProposalPrivate))

typedef struct _CodeSlayerCompletionProposalPrivate CodeSlayerCompletionProposalPrivate;

struct _CodeSlayerCompletionProposalPrivate
{
  gchar       *label;
  gchar       *text;
  GtkTextMark *mark;
};

G_DEFINE_TYPE (CodeSlayerCompletionProposal, codeslayer_completion_proposal, G_TYPE_OBJECT)

static void 
codeslayer_completion_proposal_class_init (CodeSlayerCompletionProposalClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_completion_proposal_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerCompletionProposalPrivate));
}

static void
codeslayer_completion_proposal_init (CodeSlayerCompletionProposal *proposal)
{
}

static void
codeslayer_completion_proposal_finalize (CodeSlayerCompletionProposal *proposal)
{
  CodeSlayerCompletionProposalPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSAL_GET_PRIVATE (proposal);

  if (priv->label != NULL)
    g_free (priv->label);
    
  if (priv->text != NULL)
    g_free (priv->text);

  G_OBJECT_CLASS (codeslayer_completion_proposal_parent_class)->finalize (G_OBJECT (proposal));
}

CodeSlayerCompletionProposal*
codeslayer_completion_proposal_new (gchar       *label, 
                                    gchar       *text, 
                                    GtkTextMark *mark)
{
  CodeSlayerCompletionProposalPrivate *priv;
  CodeSlayerCompletionProposal *proposal;

  proposal = CODESLAYER_COMPLETION_PROPOSAL (g_object_new (codeslayer_completion_proposal_get_type (), NULL));
  priv = CODESLAYER_COMPLETION_PROPOSAL_GET_PRIVATE (proposal);
  priv->label = g_strdup (label);
  priv->text = g_strdup (text);
  priv->mark = mark;

  return proposal;
}

const gchar*
codeslayer_completion_proposal_get_label (CodeSlayerCompletionProposal *proposal)
{
  CodeSlayerCompletionProposalPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSAL_GET_PRIVATE (proposal);
  return priv->label;
}

const gchar*
codeslayer_completion_proposal_get_text (CodeSlayerCompletionProposal *proposal)
{
  CodeSlayerCompletionProposalPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSAL_GET_PRIVATE (proposal);
  return priv->text;
}

GtkTextMark*
codeslayer_completion_proposal_get_mark (CodeSlayerCompletionProposal *proposal)
{
  CodeSlayerCompletionProposalPrivate *priv;
  priv = CODESLAYER_COMPLETION_PROPOSAL_GET_PRIVATE (proposal);
  return priv->mark;
}
