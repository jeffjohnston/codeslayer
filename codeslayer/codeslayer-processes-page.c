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

#include <gtksourceview/gtksourceview.h>
#include <codeslayer/codeslayer-processes-page.h>

static void codeslayer_processes_page_class_init  (CodeSlayerProcessesPageClass *klass);
static void codeslayer_processes_page_init        (CodeSlayerProcessesPage      *page);
static void codeslayer_processes_page_finalize    (CodeSlayerProcessesPage      *page);


#define CODESLAYER_PROCESSES_PAGE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROCESSES_PAGE_TYPE, CodeSlayerProcessesPagePrivate))

typedef struct _CodeSlayerProcessesPagePrivate CodeSlayerProcessesPagePrivate;

struct _CodeSlayerProcessesPagePrivate
{
  CodeSlayerProcesses *processes;
  GtkWidget           *textview;
};

G_DEFINE_TYPE (CodeSlayerProcessesPage, codeslayer_processes_page, GTK_TYPE_VBOX)

static void
codeslayer_processes_page_class_init (CodeSlayerProcessesPageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_processes_page_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerProcessesPagePrivate));
}

static void
codeslayer_processes_page_init (CodeSlayerProcessesPage *page) 
{
  CodeSlayerProcessesPagePrivate *priv;
  GtkWidget *scrolled_window;
  GtkTextBuffer *buffer;
  
  priv = CODESLAYER_PROCESSES_PAGE_GET_PRIVATE (page);
  
  priv->textview = gtk_source_view_new ();
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview));
  gtk_text_buffer_create_tag (buffer, "header", "weight", PANGO_WEIGHT_BOLD, NULL);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), priv->textview);

  gtk_box_pack_start (GTK_BOX (page), scrolled_window, TRUE, TRUE, 0);
}

static void
codeslayer_processes_page_finalize (CodeSlayerProcessesPage *page)
{
  G_OBJECT_CLASS (codeslayer_processes_page_parent_class)->finalize (G_OBJECT(page));
}

GtkWidget*
codeslayer_processes_page_new (CodeSlayerProcesses *processes)
{
  CodeSlayerProcessesPagePrivate *priv;
  GtkWidget *page;

  page = g_object_new (codeslayer_processes_page_get_type (), NULL);
  priv = CODESLAYER_PROCESSES_PAGE_GET_PRIVATE (page);
  priv->processes = processes;

  return page;
}
