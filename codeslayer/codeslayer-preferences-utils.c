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

#include <codeslayer/codeslayer-preferences-utils.h>
#include <codeslayer/codeslayer-preferences.h>

GtkBox*
codeslayer_preferences_utils_content_area (GtkBox *tab, 
                                           gchar  *title)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *spacer;
  gchar *full_title;

  vbox = gtk_vbox_new (FALSE, 1);
  hbox = gtk_hbox_new (FALSE, 0);
  
  /* set the title */  
  full_title = g_strconcat ("<b>", title, "</b>", NULL);
  label = gtk_label_new (full_title);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_misc_set_padding (GTK_MISC (label), 12, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, .50);
  g_free (full_title);

  /* put in a spacer */
  spacer = gtk_label_new (NULL);
  gtk_misc_set_padding (GTK_MISC (spacer), 15, 0);
  gtk_misc_set_alignment (GTK_MISC (spacer), 0, .50);

  gtk_box_pack_start (GTK_BOX (tab), GTK_WIDGET (vbox), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spacer, FALSE, FALSE, 0);

  return GTK_BOX (hbox);
}

void
codeslayer_preferences_utils_notify_editors (CodeSlayerPreferences *preferences)
{
  g_signal_emit_by_name ((gpointer) preferences, "editor-preferences-changed");
}

void
codeslayer_preferences_utils_notify_notebook (CodeSlayerPreferences *preferences)
{
  g_signal_emit_by_name ((gpointer) preferences, "notebook-preferences-changed");
}

void
codeslayer_preferences_utils_notify_side_pane (CodeSlayerPreferences *preferences)
{
  g_signal_emit_by_name ((gpointer) preferences, "side-pane-preferences-changed");
}

void
codeslayer_preferences_utils_notify_bottom_pane (CodeSlayerPreferences *preferences)
{
  g_signal_emit_by_name ((gpointer) preferences, "bottom-pane-preferences-changed");
}
