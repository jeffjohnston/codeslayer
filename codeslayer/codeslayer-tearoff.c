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
#include <codeslayer/codeslayer-tearoff.h>
#include <codeslayer/codeslayer-abstract-pane.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  GtkNotebook *source_notebook;
  GtkNotebook *notebook;
  GtkWidget   *page;
  gint         x;
  gint         y;
  gint         width;
  gint         height;
} CodeSlayerTearOff;
                                                 
static CodeSlayerTearOff* create_tearoff  (GtkNotebook        *source_notebook,
                                           GtkNotebook        *notebook,
                                           GtkWidget          *widget,
                                           gint                x,
                                           gint                y);
static void add_notebook                  (GtkWidget          *window,
                                           GtkWidget          *notebook,
                                           CodeSlayerTearOff  *tearoff);
static gboolean window_delete_action      (GtkWidget          *window, 
                                           GdkEvent           *event, 
                                           CodeSlayerTearOff  *tearoff);
static gboolean window_destroy_action     (CodeSlayerTearOff  *tearoff);

GtkNotebook*
codeslayer_tearoff_window (GtkNotebook *source_notebook,
                           GtkWidget   *widget,
                           gint         x,
                           gint         y,
                           gpointer     data)
{
  GtkWidget *window;
  GtkWidget *notebook;
  CodeSlayerTearOff *tearoff = NULL;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);  
  gtk_window_set_title (GTK_WINDOW (window), 
                        gtk_notebook_get_tab_label_text (source_notebook, widget));

  notebook = gtk_notebook_new ();
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);

  tearoff = create_tearoff (source_notebook, GTK_NOTEBOOK (notebook), widget, x, y);
                                         
  add_notebook (window, notebook, tearoff);

  if (gtk_notebook_get_n_pages (source_notebook) == 1)
    {
      GtkWidget *pane;
      pane = gtk_widget_get_ancestor(GTK_WIDGET (source_notebook), 
                                     CODESLAYER_ABSTRACT_PANE_TYPE);      
      codeslayer_abstract_pane_close (CODESLAYER_ABSTRACT_PANE (pane));
    }

  return GTK_NOTEBOOK (notebook);
}

static CodeSlayerTearOff*
create_tearoff (GtkNotebook *source_notebook,
                GtkNotebook *notebook,
                GtkWidget   *widget,
                gint         x,
                gint         y)
{
  CodeSlayerTearOff *tearoff = NULL;
  
  tearoff = g_malloc (sizeof (CodeSlayerTearOff));
  tearoff->source_notebook = source_notebook;
  tearoff->notebook = GTK_NOTEBOOK (notebook);
  tearoff->page = widget;
                                                        
  tearoff->x = x;
  tearoff->y = y;
  tearoff->width = 500;
  tearoff->height = 500;
  
  return tearoff;
}

static void
add_notebook (GtkWidget         *window,
              GtkWidget         *notebook,
              CodeSlayerTearOff *tearoff)
{
  GtkWidget *toplevel;

  gtk_container_add (GTK_CONTAINER (window), notebook);
  gtk_window_set_default_size (GTK_WINDOW (window), tearoff->width, tearoff->height);
  gtk_window_move (GTK_WINDOW (window), tearoff->x, tearoff->y);
  gtk_widget_show_all (window);
  
  toplevel = gtk_widget_get_toplevel(GTK_WIDGET (tearoff->source_notebook));  
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (toplevel));
  gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
  
  g_signal_connect (G_OBJECT (window), "delete-event",
                    G_CALLBACK (window_delete_action), tearoff);

  g_signal_connect_swapped (G_OBJECT (window), "destroy",
                    G_CALLBACK (window_destroy_action), tearoff);
}

static gboolean
window_delete_action (GtkWidget         *window,
                      GdkEvent          *event, 
                      CodeSlayerTearOff *tearoff)
{
  GtkWidget *pane;
  GtkWidget *label;  

  pane = gtk_widget_get_ancestor(GTK_WIDGET (tearoff->source_notebook), 
                                 CODESLAYER_ABSTRACT_PANE_TYPE);      
  label = gtk_label_new (gtk_window_get_title (GTK_WINDOW (window)));
  
  g_object_ref (tearoff->page);
  
  gtk_notebook_remove_page (GTK_NOTEBOOK (tearoff->notebook), -1);
  gtk_notebook_append_page (GTK_NOTEBOOK (tearoff->source_notebook),
                            tearoff->page, label);

  codeslayer_abstract_pane_open (CODESLAYER_ABSTRACT_PANE (pane), 
                                 tearoff->page);
  
  return FALSE;
}

static gboolean
window_destroy_action (CodeSlayerTearOff *tearoff)
{
  g_free (tearoff);
  return FALSE;
}                       
