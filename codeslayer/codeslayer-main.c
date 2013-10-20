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

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-application.h>
#include <codeslayer/codeslayer-utils.h>

int
main (int   argc, 
      char *argv[])
{
  CodeSlayerApplication *application;
  int status;
  gchar *profile_name = NULL;
  
  if (argc > 2 && codeslayer_utils_has_text (argv[1]) && g_strcmp0 (argv[1], "--profile") == 0)
    {
      if (codeslayer_utils_has_text (argv[2]))
        profile_name = g_strdup (argv[2]);
    }
  
  application = codeslayer_application_new (profile_name);
  status = g_application_run (G_APPLICATION (application), argc, argv);
  g_object_unref (application);
  return status;
}
