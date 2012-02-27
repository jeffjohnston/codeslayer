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

#ifndef __CODESLAYER_COMPLETION_PROPOSALS_H__
#define	__CODESLAYER_COMPLETION_PROPOSALS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-completion-proposal.h>

G_BEGIN_DECLS

#define CODESLAYER_COMPLETION_PROPOSALS_TYPE            (codeslayer_completion_proposals_get_type ())
#define CODESLAYER_COMPLETION_PROPOSALS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_COMPLETION_PROPOSALS_TYPE, CodeSlayerCompletionProposals))
#define CODESLAYER_COMPLETION_PROPOSALS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_COMPLETION_PROPOSALS_TYPE, CodeSlayerCompletionProposalsClass))
#define IS_CODESLAYER_COMPLETION_PROPOSALS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_COMPLETION_PROPOSALS_TYPE))
#define IS_CODESLAYER_COMPLETION_PROPOSALS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_COMPLETION_PROPOSALS_TYPE))

typedef struct _CodeSlayerCompletionProposals CodeSlayerCompletionProposals;
typedef struct _CodeSlayerCompletionProposalsClass CodeSlayerCompletionProposalsClass;

struct _CodeSlayerCompletionProposals
{
  GObject parent_instance;
};
  
struct _CodeSlayerCompletionProposalsClass
{
  GObjectClass parent_class;
};

GType codeslayer_completion_proposals_get_type (void) G_GNUC_CONST;

CodeSlayerCompletionProposals*  codeslayer_completion_proposals_new           (void);

GList*                          codeslayer_completion_proposals_get_list      (CodeSlayerCompletionProposals *proposals);
void                            codeslayer_completion_proposals_set_list      (CodeSlayerCompletionProposals *proposals, 
                                                                               GList                         *list);
GtkTextMark*                    codeslayer_completion_proposals_get_mark      (CodeSlayerCompletionProposals *proposals);
void                            codeslayer_completion_proposals_set_mark      (CodeSlayerCompletionProposals *proposals, 
                                                                               GtkTextMark                   *mark);
void                            codeslayer_completion_proposals_add_proposal  (CodeSlayerCompletionProposals *proposals, 
                                                                               CodeSlayerCompletionProposal  *proposal);

G_END_DECLS

#endif /* __CODESLAYER_COMPLETION_PROPOSALS_H__ */
