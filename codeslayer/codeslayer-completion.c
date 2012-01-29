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
#include <codeslayer/codeslayer-completion.h>
#include <codeslayer/codeslayer-completion-provider.h>
#include <codeslayer/codeslayer-completion-proposal.h>

static void codeslayer_completion_class_init  (CodeSlayerCompletionClass    *klass);
static void codeslayer_completion_init        (CodeSlayerCompletion         *completion);
static void codeslayer_completion_finalize    (CodeSlayerCompletion         *completion);

static void process_proposals                 (CodeSlayerCompletionProvider *provider, 
                                               GtkTextIter                   iter);
                                       
#define CODESLAYER_COMPLETION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_COMPLETION_TYPE, CodeSlayerCompletionPrivate))

typedef struct _CodeSlayerCompletionPrivate CodeSlayerCompletionPrivate;

struct _CodeSlayerCompletionPrivate
{
  GList *providers;
};

G_DEFINE_TYPE (CodeSlayerCompletion, codeslayer_completion, G_TYPE_OBJECT)
     
static void 
codeslayer_completion_class_init (CodeSlayerCompletionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_completion_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerCompletionPrivate));
}

static void
codeslayer_completion_init (CodeSlayerCompletion *completion)
{
}

static void
codeslayer_completion_finalize (CodeSlayerCompletion *completion)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);  
  priv->providers = NULL;
  G_OBJECT_CLASS (codeslayer_completion_parent_class)->finalize (G_OBJECT (completion));
}

CodeSlayerCompletion*
codeslayer_completion_new ()
{
  CodeSlayerCompletion *completion;
  completion = CODESLAYER_COMPLETION (g_object_new (codeslayer_completion_get_type (), NULL));
  return completion;
}

void
codeslayer_completion_add_provider  (CodeSlayerCompletion         *completion, 
                                     CodeSlayerCompletionProvider *provider)
{
  CodeSlayerCompletionPrivate *priv;
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  priv->providers = g_list_append (priv->providers, provider);
}                                     

void
codeslayer_completion_invoke (CodeSlayerCompletion *completion, 
                              GtkTextIter           iter)
{
  CodeSlayerCompletionPrivate *priv;
  GList *list;
  
  priv = CODESLAYER_COMPLETION_GET_PRIVATE (completion);
  
  list = priv->providers;
  while (list != NULL)
    {
      CodeSlayerCompletionProvider *provider = list->data;
      if (codeslayer_completion_provider_has_match (provider, iter))
        process_proposals (provider, iter);
      list = g_list_next (list);
    }
}

static void
process_proposals (CodeSlayerCompletionProvider *provider, 
                   GtkTextIter                   iter)
{
  GList *proposals;
  GList *list;

  proposals = codeslayer_completion_provider_get_proposals (provider, iter);
  list = proposals;
  
  g_print ("*****************************************************************\n");

  while (list != NULL)
    {
      CodeSlayerCompletionProposal *proposal = list->data;
      const gchar *label;
      label = codeslayer_completion_proposal_get_label (proposal);
      
      g_print ("label %s \n", label);

      list = g_list_next (list);
    }
    
  g_list_foreach (proposals, (GFunc)g_object_unref, NULL);
  g_list_free (proposals);
}
