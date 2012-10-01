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

#ifndef __CODESLAYER_PROCESSES_H__
#define	__CODESLAYER_PROCESSES_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-process.h>

G_BEGIN_DECLS

#define CODESLAYER_PROCESSES_TYPE            (codeslayer_processes_get_type ())
#define CODESLAYER_PROCESSES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROCESSES_TYPE, CodeSlayerProcesses))
#define CODESLAYER_PROCESSES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROCESSES_TYPE, CodeSlayerProcessesClass))
#define IS_CODESLAYER_PROCESSES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROCESSES_TYPE))
#define IS_CODESLAYER_PROCESSES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROCESSES_TYPE))

typedef struct _CodeSlayerProcesses CodeSlayerProcesses;
typedef struct _CodeSlayerProcessesClass CodeSlayerProcessesClass;

struct _CodeSlayerProcesses
{
  GObject parent_instance;
};

struct _CodeSlayerProcessesClass
{
  GObjectClass parent_class;
  
  void (*process_started) (CodeSlayerProcesses *processes);  
  void (*process_finished) (CodeSlayerProcesses *processes);  
};

GType codeslayer_processes_get_type (void) G_GNUC_CONST;

CodeSlayerProcesses*  codeslayer_processes_new     (void);

void                  codeslayer_processes_add     (CodeSlayerProcesses *processes, 
                                                    CodeSlayerProcess   *process);
void                  codeslayer_processes_remove  (CodeSlayerProcesses *processes, 
                                                    CodeSlayerProcess   *process);

G_END_DECLS

#endif /* _CODESLAYER_PROCESSES_H */
