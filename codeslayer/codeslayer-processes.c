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

static void codeslayer_processes_class_init  (CodeSlayerProcessesClass *klass);
static void codeslayer_processes_init        (CodeSlayerProcesses      *processes);
static void codeslayer_processes_finalize    (CodeSlayerProcesses      *processes);
static void process_stopped                  (CodeSlayerProcesses      *processes, 
                                              CodeSlayerProcess        *process);
                                                   
#define CODESLAYER_PROCESSES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROCESSES_TYPE, CodeSlayerProcessesPrivate))

typedef struct _CodeSlayerProcessesPrivate CodeSlayerProcessesPrivate;

struct _CodeSlayerProcessesPrivate
{
  GList *list;
};

enum
{
  PROCESS_STARTED,
  PROCESS_FINISHED,
  LAST_SIGNAL
};

static guint codeslayer_processes_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CodeSlayerProcesses, codeslayer_processes, G_TYPE_OBJECT)

static void
codeslayer_processes_class_init (CodeSlayerProcessesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  /**
   * CodeSlayerProcesses::process-started
   * @codeslayerprocesses: the processes that received the signal
   *
   * Note: for internal use only.
   *
   * The ::process-started signal is invoked when a thread is created and
   * added to the list of processes.
   */
  codeslayer_processes_signals[PROCESS_STARTED] =
    g_signal_new ("process-started", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProcessesClass, process_started),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
  
  /**
   * CodeSlayerProcesses::process-finished
   * @codeslayerprocesses: the processes that received the signal
   *
   * Note: for internal use only.
   *
   * The ::process-finished signal is invoked when a thread finishes and
   * removed from the list of processes. 
   */
  codeslayer_processes_signals[PROCESS_FINISHED] =
    g_signal_new ("process-finished", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProcessesClass, process_finished),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
  
  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_processes_finalize;
  g_type_class_add_private (klass, sizeof (CodeSlayerProcessesPrivate));
}

static void
codeslayer_processes_init (CodeSlayerProcesses *processes) 
{
  CodeSlayerProcessesPrivate *priv;
  priv = CODESLAYER_PROCESSES_GET_PRIVATE (processes);
  priv->list = NULL;
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

void
codeslayer_processes_add (CodeSlayerProcesses *processes, 
                          CodeSlayerProcess   *process)
{
  CodeSlayerProcessesPrivate *priv;
  
  priv = CODESLAYER_PROCESSES_GET_PRIVATE (processes);
  
  g_object_ref_sink (process);
  
  g_signal_connect_swapped (G_OBJECT (process), "stopped",
                            G_CALLBACK (process_stopped), processes);
    
  priv->list = g_list_prepend (priv->list, process);
  g_signal_emit_by_name ((gpointer) processes, "process-started", process);
}   

void
codeslayer_processes_remove (CodeSlayerProcesses *processes, 
                             gint                 id)
{
  CodeSlayerProcessesPrivate *priv;
  GList *list;
  
  priv = CODESLAYER_PROCESSES_GET_PRIVATE (processes);
  
  list = priv->list;
  while (list != NULL)
    {
      CodeSlayerProcess *process = list->data;
      if (codeslayer_process_get_id (process) == id)
        {
          g_signal_emit_by_name ((gpointer) processes, "process-finished", process);
          priv->list = g_list_remove (priv->list, process);
          g_object_unref (process);
          break;
        }
      list = g_list_next (list);
    }
}

static void
process_stopped (CodeSlayerProcesses *processes, 
                 CodeSlayerProcess   *process)
{
  StopProcessFunc func;
  gpointer data;
  
  func = codeslayer_process_get_func (process);
  data = codeslayer_process_get_data (process);
  
  if (func != NULL)
    func (data);
}
