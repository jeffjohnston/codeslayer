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

#include <codeslayer/codeslayer-processes.h>

static void codeslayer_processes_class_init  (CodeSlayerProcessesClass  *klass);
static void codeslayer_processes_init        (CodeSlayerProcesses       *processes);
static void codeslayer_processes_finalize    (CodeSlayerProcesses       *processes);
                                                   
#define CODESLAYER_PROCESSES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROCESSES_TYPE, CodeSlayerProcessesPrivate))

typedef struct _CodeSlayerProcessesPrivate CodeSlayerProcessesPrivate;

struct _CodeSlayerProcessesPrivate
{
  GList *threads;
};

G_DEFINE_TYPE (CodeSlayerProcesses, codeslayer_processes, G_TYPE_OBJECT)

static void
codeslayer_processes_class_init (CodeSlayerProcessesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_processes_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerProcessesPrivate));
}

static void
codeslayer_processes_init (CodeSlayerProcesses *processes) 
{
  CodeSlayerProcessesPrivate *priv;
  priv = CODESLAYER_PROCESSES_GET_PRIVATE (processes);
  priv->threads = NULL;
}

static void
codeslayer_processes_finalize (CodeSlayerProcesses *processes)
{
  G_OBJECT_CLASS (codeslayer_processes_parent_class)->finalize (G_OBJECT(processes));
}

CodeSlayerProcesses*
codeslayer_processes_new ()
{
  CodeSlayerProcesses *processes;
  processes = CODESLAYER_PROCESSES (g_object_new (codeslayer_processes_get_type (), NULL));
  return processes;
}

GThread*
codeslayer_processes_add (CodeSlayerProcesses *processes, 
                          gchar               *name,
                          GThreadFunc          func, 
                          gpointer             data)
{
  CodeSlayerProcessesPrivate *priv;
  GThread *thread;

  priv = CODESLAYER_PROCESSES_GET_PRIVATE (processes);

  g_print ("added process %s\n", name);
  
  thread = g_thread_new (name, func, data);
  
  priv->threads = g_list_prepend (priv->threads, thread);
  
  return thread;
}                          
