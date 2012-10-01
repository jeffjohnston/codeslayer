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

#ifndef __CODESLAYER_PROCESS_H__
#define	__CODESLAYER_PROCESS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CODESLAYER_PROCESS_TYPE            (codeslayer_process_get_type ())
#define CODESLAYER_PROCESS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CODESLAYER_PROCESS_TYPE, CodeSlayerProcess))
#define CODESLAYER_PROCESS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CODESLAYER_PROCESS_TYPE, CodeSlayerProcessClass))
#define IS_CODESLAYER_PROCESS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CODESLAYER_PROCESS_TYPE))
#define IS_CODESLAYER_PROCESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CODESLAYER_PROCESS_TYPE))

typedef struct _CodeSlayerProcess CodeSlayerProcess;
typedef struct _CodeSlayerProcessClass CodeSlayerProcessClass;

struct _CodeSlayerProcess
{
  GObject parent_instance;
};

struct _CodeSlayerProcessClass
{
  GObjectClass parent_class;
  
  void (*stopped) (CodeSlayerProcess *process);  
};

GType codeslayer_process_get_type (void) G_GNUC_CONST;

CodeSlayerProcess  *codeslayer_process_new                     (void);

const gchar*       codeslayer_process_get_name                 (CodeSlayerProcess *process);
void               codeslayer_process_set_name                 (CodeSlayerProcess *process,
                                                                const gchar       *name);
const gchar*       codeslayer_process_get_key                  (CodeSlayerProcess *process);
void               codeslayer_process_set_key                  (CodeSlayerProcess *process,
                                                                const gchar       *key);
GThreadFunc        codeslayer_process_get_func                 (CodeSlayerProcess *process);
void               codeslayer_process_set_func                 (CodeSlayerProcess *process,
                                                                GThreadFunc        func);
gpointer           codeslayer_process_get_data                 (CodeSlayerProcess *process);
void               codeslayer_process_set_data                 (CodeSlayerProcess *process,
                                                                gpointer           data);
void               codeslayer_process_start                    (CodeSlayerProcess *process);
void               codeslayer_process_stop                     (CodeSlayerProcess *process);

G_END_DECLS

#endif /* __CODESLAYER_PROCESS_H__ */
