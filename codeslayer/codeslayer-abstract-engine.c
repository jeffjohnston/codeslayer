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

#include <stdlib.h>
#include <codeslayer/codeslayer-abstract-engine.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-document.h>
#include <codeslayer/codeslayer-projects.h>
#include <codeslayer/codeslayer-projects-search.h>
#include <codeslayer/codeslayer-menubar.h>
#include <codeslayer/codeslayer-profile.h>
#include <codeslayer/codeslayer-side-pane.h>
#include <codeslayer/codeslayer-bottom-pane.h>
#include <codeslayer/codeslayer-notebook.h>
#include <codeslayer/codeslayer-notebook-tab.h>
#include <codeslayer/codeslayer-notebook-page.h>
#include <codeslayer/codeslayer-notebook-pane.h>
#include <codeslayer/codeslayer-editor.h>
#include <codeslayer/codeslayer-plugins.h>

/**
 * SECTION:codeslayer-engine
 * @short_description: Central delegation class.
 * @title: CodeSlayerAbstractEngine
 * @include: codeslayer/codeslayer-engine.h
 *
 * The engine sits in the middle of the framework and delegates calls out to 
 * the rest of the API. By having such a central engine we can keep various 
 * parts of the application very decoupled.
 */

static void codeslayer_abstract_engine_class_init   (CodeSlayerAbstractEngineClass *klass);
static void codeslayer_abstract_engine_init         (CodeSlayerAbstractEngine      *engine);
static void codeslayer_abstract_engine_finalize     (CodeSlayerAbstractEngine      *engine);
static void codeslayer_abstract_engine_get_property (GObject                       *object, 
                                                     guint                          prop_id,
                                                     GValue                        *value,
                                                     GParamSpec                    *pspec);
static void codeslayer_abstract_engine_set_property (GObject                       *object,
                                                     guint                          prop_id,
                                                     const GValue                  *value,
                                                     GParamSpec                    *pspec);

static void save_document_settings                 (CodeSlayerAbstractEngine      *engine);
static void save_window_settings                   (CodeSlayerAbstractEngine      *engine);
                                                   
#define CODESLAYER_ABSTRACT_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CODESLAYER_ABSTRACT_ENGINE_TYPE, CodeSlayerAbstractEnginePrivate))

typedef struct _CodeSlayerAbstractEnginePrivate CodeSlayerAbstractEnginePrivate;

struct _CodeSlayerAbstractEnginePrivate
{
  GtkWindow          *window;
  CodeSlayerProfiles *profiles;
  GtkWidget          *menubar;
  GtkWidget          *notebook;
  GtkWidget          *notebook_pane;
  GtkWidget          *side_pane;
  GtkWidget          *bottom_pane;
  GtkWidget          *hpaned;
  GtkWidget          *vpaned;
};

G_DEFINE_ABSTRACT_TYPE (CodeSlayerAbstractEngine, codeslayer_abstract_engine, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_WINDOW,
  PROP_CONFIG_HANDLER,
  PROP_MENUBAR,
  PROP_NOTEBOOK,
  PROP_NOTEBOOK_PANE,
  PROP_SIDE_PANE,
  PROP_BOTTOM_PANE,
  PROP_HPANED, 
  PROP_VPANED  
};

static void
codeslayer_abstract_engine_class_init (CodeSlayerAbstractEngineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);  
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) codeslayer_abstract_engine_finalize;
  
  gobject_class->get_property = codeslayer_abstract_engine_get_property;
  gobject_class->set_property = codeslayer_abstract_engine_set_property;
  
  g_type_class_add_private (klass, sizeof (CodeSlayerAbstractEnginePrivate));
  
  g_object_class_install_property (gobject_class, 
                                   PROP_WINDOW,
                                   g_param_spec_pointer ("window",
                                                         "GtkWindow",
                                                         "GtkWindow",
                                                         G_PARAM_READWRITE));
  
  g_object_class_install_property (gobject_class, 
                                   PROP_CONFIG_HANDLER,
                                   g_param_spec_pointer ("profiles",
                                                         "CodeSlayerProfiles",
                                                         "CodeSlayerProfiles",
                                                         G_PARAM_READWRITE));  
  
  g_object_class_install_property (gobject_class, 
                                   PROP_MENUBAR,
                                   g_param_spec_pointer ("menubar",
                                                         "CodeSlayerMenuBar",
                                                         "CodeSlayerMenuBar",
                                                         G_PARAM_READWRITE));  
  
  g_object_class_install_property (gobject_class, 
                                   PROP_NOTEBOOK,
                                   g_param_spec_pointer ("notebook",
                                                         "CodeSlayerNotebook",
                                                         "CodeSlayerNotebook",
                                                         G_PARAM_READWRITE));  
  
  g_object_class_install_property (gobject_class, 
                                   PROP_NOTEBOOK_PANE,
                                   g_param_spec_pointer ("notebook_pane",
                                                         "CodeSlayerNotebookPane",
                                                         "CodeSlayerNotebookPane",
                                                         G_PARAM_READWRITE));  
  
  g_object_class_install_property (gobject_class, 
                                   PROP_SIDE_PANE,
                                   g_param_spec_pointer ("side_pane",
                                                         "CodeSlayerSidePane",
                                                         "CodeSlayerSidePane",
                                                         G_PARAM_READWRITE));  
  
  g_object_class_install_property (gobject_class, 
                                   PROP_BOTTOM_PANE,
                                   g_param_spec_pointer ("bottom_pane",
                                                         "CodeSlayerBottomPane",
                                                         "CodeSlayerBottomPane",
                                                         G_PARAM_READWRITE));  
  
  g_object_class_install_property (gobject_class, 
                                   PROP_HPANED,
                                   g_param_spec_pointer ("hpaned",
                                                         "GtkWidget",
                                                         "GtkWidget",
                                                         G_PARAM_READWRITE));  
  
  g_object_class_install_property (gobject_class, 
                                   PROP_VPANED,
                                   g_param_spec_pointer ("vpaned",
                                                         "GtkWidget",
                                                         "GtkWidget",
                                                         G_PARAM_READWRITE));  
}

static void
codeslayer_abstract_engine_init (CodeSlayerAbstractEngine *engine) 
{
}

static void
codeslayer_abstract_engine_finalize (CodeSlayerAbstractEngine *engine)
{
  G_OBJECT_CLASS (codeslayer_abstract_engine_parent_class)->finalize (G_OBJECT (engine));
}

static void
codeslayer_abstract_engine_get_property (GObject    *object, 
                                         guint       prop_id,
                                         GValue     *value, 
                                         GParamSpec *pspec)
{
  CodeSlayerAbstractEnginePrivate *priv;
  priv = CODESLAYER_ABSTRACT_ENGINE_GET_PRIVATE (object);

  switch (prop_id)
    {
    case PROP_WINDOW:
      g_value_set_pointer (value, priv->window);
      break;
    case PROP_CONFIG_HANDLER:
      g_value_set_pointer (value, priv->profiles);
      break;
    case PROP_MENUBAR:
      g_value_set_pointer (value, priv->menubar);
      break;
    case PROP_NOTEBOOK:
      g_value_set_pointer (value, priv->notebook);
      break;
    case PROP_NOTEBOOK_PANE:
      g_value_set_pointer (value, priv->notebook_pane);
      break;
    case PROP_SIDE_PANE:
      g_value_set_pointer (value, priv->side_pane);
      break;
    case PROP_BOTTOM_PANE:
      g_value_set_pointer (value, priv->bottom_pane);
      break;
    case PROP_HPANED:
      g_value_set_pointer (value, priv->hpaned);
      break;
    case PROP_VPANED:
      g_value_set_pointer (value, priv->vpaned);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
codeslayer_abstract_engine_set_property (GObject      *object, 
                                         guint         prop_id,
                                         const GValue *value, 
                                         GParamSpec   *pspec)
{
  CodeSlayerAbstractEnginePrivate *priv;
  priv = CODESLAYER_ABSTRACT_ENGINE_GET_PRIVATE (object);

  switch (prop_id)
    {
    case PROP_WINDOW:
      priv->window = g_value_get_pointer (value);
      break;
    case PROP_CONFIG_HANDLER:
      priv->profiles = g_value_get_pointer (value);
      break;
    case PROP_MENUBAR:
      priv->menubar = g_value_get_pointer (value);
      break;
    case PROP_NOTEBOOK:
      priv->notebook = g_value_get_pointer (value);
      break;
    case PROP_NOTEBOOK_PANE:
      priv->notebook_pane = g_value_get_pointer (value);
      break;
    case PROP_SIDE_PANE:
      priv->side_pane = g_value_get_pointer (value);
      break;
    case PROP_BOTTOM_PANE:
      priv->bottom_pane = g_value_get_pointer (value);
      break;
    case PROP_HPANED:
      priv->hpaned = g_value_get_pointer (value);
      break;
    case PROP_VPANED:
      priv->vpaned = g_value_get_pointer (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

gboolean
codeslayer_abstract_engine_save_profile (CodeSlayerAbstractEngine *engine)
{
  CodeSlayerAbstractEnginePrivate *priv;
  CodeSlayerProfile *profile;
  
  priv = CODESLAYER_ABSTRACT_ENGINE_GET_PRIVATE (engine);
  profile = codeslayer_profiles_get_current_profile (priv->profiles);

  if (profile == NULL)
    return TRUE;
  
  if (codeslayer_notebook_has_unsaved_editors (CODESLAYER_NOTEBOOK (priv->notebook)))
    return FALSE;

  save_window_settings (engine);
  save_document_settings (engine);
  codeslayer_profiles_save_profile (priv->profiles, profile);
  
  return TRUE;
}

static void
save_document_settings (CodeSlayerAbstractEngine *abstract_engine)
{
  CodeSlayerAbstractEnginePrivate *priv;
  CodeSlayerProfile *profile;
  GList *documents = NULL;
  gint pages;
  gint page;

  priv = CODESLAYER_ABSTRACT_ENGINE_GET_PRIVATE (abstract_engine);
  profile = codeslayer_profiles_get_current_profile (priv->profiles);
  
  if (codeslayer_profile_get_enable_projects (profile) == FALSE)
    return;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));
  for (page = 0; page < pages; page++)
    {
      GtkWidget *notebook_page;
      CodeSlayerDocument *document;
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), page);
      document = codeslayer_notebook_page_get_document (CODESLAYER_NOTEBOOK_PAGE (notebook_page));
      documents = g_list_append (documents, document);
    }
    
  codeslayer_profile_set_documents (profile, documents);
}

static void
save_window_settings (CodeSlayerAbstractEngine *abstract_engine)
{
  CodeSlayerAbstractEnginePrivate *priv;
  CodeSlayerProfile *profile;
  CodeSlayerRegistry *registry; 
  gint width;
  gint height;
  gint x;
  gint y;
  gint position;
  
  priv = CODESLAYER_ABSTRACT_ENGINE_GET_PRIVATE (abstract_engine);
  
  profile = codeslayer_profiles_get_current_profile (priv->profiles);
  registry = codeslayer_profile_get_registry (profile);

  gtk_window_get_size (GTK_WINDOW (priv->window), &width, &height);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_WINDOW_WIDTH,
                                   width);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_WINDOW_HEIGHT,
                                   height);

  gtk_window_get_position (GTK_WINDOW (priv->window), &x, &y);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_WINDOW_X, x);
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_WINDOW_Y, y);

  position = gtk_paned_get_position (GTK_PANED (priv->hpaned));
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_HPANED_POSITION,
                                   position);

  position = gtk_paned_get_position (GTK_PANED (priv->vpaned));
  codeslayer_registry_set_integer (registry,
                                   CODESLAYER_REGISTRY_VPANED_POSITION,
                                   position);
}

void
codeslayer_abstract_engine_load_window_settings (CodeSlayerAbstractEngine *abstract_engine)
{
  CodeSlayerAbstractEnginePrivate *priv;
  CodeSlayerProfile *profile;
  CodeSlayerRegistry *registry; 
  gint window_width;
  gint window_height;
  gint window_x;
  gint window_y;
  gint hpaned_position;
  gint vpaned_position;
  gboolean show_side_pane;
  gboolean show_bottom_pane;
  
  priv = CODESLAYER_ABSTRACT_ENGINE_GET_PRIVATE (abstract_engine);
  
  profile = codeslayer_profiles_get_current_profile (priv->profiles);
  registry = codeslayer_profile_get_registry (profile);
    
  /* window specific settings */                                              

  window_width = codeslayer_registry_get_integer (registry,
                                                  CODESLAYER_REGISTRY_WINDOW_WIDTH);
  if (window_width < 0)
    {
      window_width = 800;
    }
  
  window_height = codeslayer_registry_get_integer (registry,
                                                   CODESLAYER_REGISTRY_WINDOW_HEIGHT);
  if (window_height < 0)
    {
      window_height = 600;
    }
    
  /*gtk_window_set_default_size (GTK_WINDOW (priv->window), window_width, window_height);*/
  gtk_window_resize (GTK_WINDOW (priv->window), window_width, window_height);

  window_x = codeslayer_registry_get_integer (registry,
                                              CODESLAYER_REGISTRY_WINDOW_X);
  if (window_x < 0)
    {
      window_x = 10;
    }
    
  window_y = codeslayer_registry_get_integer (registry,
                                              CODESLAYER_REGISTRY_WINDOW_Y);
  if (window_y < 0)
    {
      window_y = 10;
    }
    
  gtk_window_move (GTK_WINDOW (priv->window), window_x, window_y);
    
  /* side and bottom pane settings */
  
  hpaned_position = codeslayer_registry_get_integer (registry,
                                                     CODESLAYER_REGISTRY_HPANED_POSITION);
  if (hpaned_position == -1)
    hpaned_position = 250;

  gtk_paned_set_position (GTK_PANED (priv->hpaned), hpaned_position);
                                                
  vpaned_position = codeslayer_registry_get_integer (registry,
                                                     CODESLAYER_REGISTRY_VPANED_POSITION);
  if (vpaned_position == -1)
    vpaned_position = 250;

  gtk_paned_set_position (GTK_PANED (priv->vpaned), vpaned_position);
    
  /* we have to this before we show or hide the panes */
  gtk_widget_show_all (GTK_WIDGET (priv->window));
    
  /* show or hide panes */
  
  show_side_pane = codeslayer_registry_get_boolean (registry,
                                                    CODESLAYER_REGISTRY_SIDE_PANE_VISIBLE);
  gtk_widget_set_visible (gtk_paned_get_child1 (GTK_PANED(priv->hpaned)), 
                                                show_side_pane);
                                                
  show_bottom_pane = codeslayer_registry_get_boolean (registry,
                                                      CODESLAYER_REGISTRY_BOTTOM_PANE_VISIBLE);
  gtk_widget_set_visible (gtk_paned_get_child2 (GTK_PANED(priv->vpaned)), 
                                                show_bottom_pane);
}

void 
codeslayer_abstract_engine_sync_menu_bar (CodeSlayerAbstractEngine *abstract_engine)
{
  CodeSlayerAbstractEnginePrivate *priv;
  CodeSlayerProfile *profile;
  gboolean enable_projects;
  gboolean has_open_editors;
  gint pages;

  priv = CODESLAYER_ABSTRACT_ENGINE_GET_PRIVATE (abstract_engine);
  profile = codeslayer_profiles_get_current_profile (priv->profiles);

  enable_projects = codeslayer_profile_get_enable_projects (profile);

  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook));
  has_open_editors = pages > 0;
  
  codeslayer_notebook_pane_sync_with_notebook (CODESLAYER_NOTEBOOK_PANE (priv->notebook_pane));
  
  g_signal_emit_by_name ((gpointer) priv->menubar, "sync-engine", enable_projects, has_open_editors);
}
