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

#include <codeslayer/codeslayer-process.h>

/**
 * SECTION:codeslayer-process
 * @short_description: Contains the documents.
 * @title: CodeSlayerProcess
 * @include: codeslayer/codeslayer-process.h
 */

static void codeslayer_process_class_init    (CodeSlayerProcessClass *klass);
static void codeslayer_process_init          (CodeSlayerProcess      *process);
static void codeslayer_process_finalize      (CodeSlayerProcess      *process);
static void codeslayer_process_get_property  (GObject                *object, 
                                              guint                   prop_id,
                                              GValue                 *value,
                                              GParamSpec             *pspec);
static void codeslayer_process_set_property  (GObject                *object, 
                                              guint                   prop_id,
                                              const GValue           *value,
                                              GParamSpec             *pspec);

#define CODESLAYER_PROCESS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_PROCESS_TYPE, CodeSlayerProcessPrivate))

typedef struct _CodeSlayerProcessPrivate CodeSlayerProcessPrivate;

struct _CodeSlayerProcessPrivate
{
  gint             id;
  gchar           *name;
  StopProcessFunc  func;
  gpointer         data;
};

enum
{
  STOPPED,
  LAST_SIGNAL
};

static guint codeslayer_process_signals[LAST_SIGNAL] = { 0 };

enum
{
  PROP_0,
  PROP_NAME,
  PROP_FUNC,
  PROP_DATA
};

G_DEFINE_TYPE (CodeSlayerProcess, codeslayer_process, G_TYPE_OBJECT)
     
static void 
codeslayer_process_class_init (CodeSlayerProcessClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  /**
   * CodeSlayerProcess::stopped
   * @codeslayerprocess: the process that received the signal
   *
   * Note: for internal use only.
   *
   * The ::stopped signal is invoked when a thread is finished 
   */
  codeslayer_process_signals[STOPPED] =
    g_signal_new ("stopped", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CodeSlayerProcessClass, stopped),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  gobject_class->finalize = (GObjectFinalizeFunc) codeslayer_process_finalize;

  gobject_class->get_property = codeslayer_process_get_property;
  gobject_class->set_property = codeslayer_process_set_property;

  g_type_class_add_private (klass, sizeof (CodeSlayerProcessPrivate));

  /**
   * CodeSlayerProcess:name:
   *
   * The displayed name for the process.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_NAME,
                                   g_param_spec_string ("name", 
                                                        "Name",
                                                        "Name Object", "",
                                                        G_PARAM_READWRITE));

  /**
   * CodeSlayerProcess:func:
   *
   * The callback function for the process.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_FUNC,
                                   g_param_spec_pointer ("func", 
                                                         "func",
                                                         "Func",
                                                         G_PARAM_READWRITE));

  /**
   * CodeSlayerProcess:data:
   *
   * The thread data for the process.
   */
  g_object_class_install_property (gobject_class, 
                                   PROP_DATA,
                                   g_param_spec_pointer ("data", 
                                                         "data",
                                                         "Data",
                                                         G_PARAM_READWRITE));
}

static void
codeslayer_process_init (CodeSlayerProcess *process)
{
  CodeSlayerProcessPrivate *priv;
  priv = CODESLAYER_PROCESS_GET_PRIVATE (process);
  priv->name = NULL;
  priv->func = NULL;
  priv->data = NULL;
}

static void
codeslayer_process_finalize (CodeSlayerProcess *process)
{
  CodeSlayerProcessPrivate *priv;
  priv = CODESLAYER_PROCESS_GET_PRIVATE (process);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  G_OBJECT_CLASS (codeslayer_process_parent_class)->finalize (G_OBJECT (process));
}

static void
codeslayer_process_get_property (GObject    *object, 
                                 guint       prop_id,
                                 GValue     *value, 
                                 GParamSpec *pspec)
{
  CodeSlayerProcess *process;
  CodeSlayerProcessPrivate *priv;
  
  process = CODESLAYER_PROCESS (object);
  priv = CODESLAYER_PROCESS_GET_PRIVATE (process);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_FUNC:
      g_value_set_pointer (value, priv->func);
      break;
    case PROP_DATA:
      g_value_set_pointer (value, priv->data);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_process_set_property (GObject      *object, 
                                 guint         prop_id,
                                 const GValue *value, 
                                 GParamSpec   *pspec)
{
  CodeSlayerProcess *process;
  process = CODESLAYER_PROCESS (object);

  switch (prop_id)
    {
    case PROP_NAME:
      codeslayer_process_set_name (process, g_value_get_string (value));
      break;
    case PROP_FUNC:
      codeslayer_process_set_func (process, g_value_get_pointer (value));
      break;
    case PROP_DATA:
      codeslayer_process_set_data (process, g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * codeslayer_process_new:
 * @id: the process identifier.
 *
 * Creates a new #CodeSlayerProcess.
 *
 * Returns: a new #CodeSlayerProcess. 
 */
CodeSlayerProcess*
codeslayer_process_new (gint id)
{
  CodeSlayerProcess *process;
  CodeSlayerProcessPrivate *priv;
  
  process = CODESLAYER_PROCESS (g_object_new (codeslayer_process_get_type (), NULL));
  priv = CODESLAYER_PROCESS_GET_PRIVATE (process);
  priv->id = id;

  return process;
}

/**
 * codeslayer_process_get_id:
 * @process: a #CodeSlayerProcess.
 *
 * Returns: the identifier for the process.
 */
const gint
codeslayer_process_get_id (CodeSlayerProcess *process)
{
  return CODESLAYER_PROCESS_GET_PRIVATE (process)->id;
}

/**
 * codeslayer_process_get_name:
 * @process: a #CodeSlayerProcess.
 *
 * Returns: the text to display for the process.
 */
const gchar *
codeslayer_process_get_name (CodeSlayerProcess *process)
{
  return CODESLAYER_PROCESS_GET_PRIVATE (process)->name;
}

/**
 * codeslayer_process_set_name:
 * @process: a #CodeSlayerProcess.
 * @name: the text to display for the process.
 */
void
codeslayer_process_set_name (CodeSlayerProcess *process, 
                             const gchar       *name)
{
  CodeSlayerProcessPrivate *priv;
  priv = CODESLAYER_PROCESS_GET_PRIVATE (process);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

/**
 * codeslayer_process_get_func:
 * @process: a #CodeSlayerProcess.
 *
 * Returns: the callback function for when the process is stopped
 */
StopProcessFunc
codeslayer_process_get_func (CodeSlayerProcess *process)
{
  return CODESLAYER_PROCESS_GET_PRIVATE (process)->func;
}

/**
 * codeslayer_process_set_func:
 * @process: a #CodeSlayerProcess.
 * @func: the callback function for when the process is stopped
 */
void
codeslayer_process_set_func (CodeSlayerProcess *process, 
                             StopProcessFunc    func)
{
  CodeSlayerProcessPrivate *priv;
  priv = CODESLAYER_PROCESS_GET_PRIVATE (process);
  priv->func = func;
}

/**
 * codeslayer_process_get_data:
 * @process: a #CodeSlayerProcess.
 *
 * Returns: the data for the stopped callback function
 */
gpointer
codeslayer_process_get_data (CodeSlayerProcess *process)
{
  return CODESLAYER_PROCESS_GET_PRIVATE (process)->data;
}

/**
 * codeslayer_process_set_data:
 * @process: a #CodeSlayerProcess.
 * @data: the data for the stopped callback function
 */
void
codeslayer_process_set_data (CodeSlayerProcess *process, 
                             gpointer           data)
{
  CodeSlayerProcessPrivate *priv;
  priv = CODESLAYER_PROCESS_GET_PRIVATE (process);
  priv->data = data;
}

/**
 * codeslayer_process_stop:
 * @process: a #CodeSlayerProcess.
 *
 * Stop the running thread
 */
void               
codeslayer_process_stop (CodeSlayerProcess *process)
{
  g_signal_emit_by_name ((gpointer) process, "stopped", process);
}
