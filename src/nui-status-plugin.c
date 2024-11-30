/*
 * nui-status-plugin.c
 *
 * Copyright (C) 2024 Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "config.h"

#include <hildon/hildon.h>
#include <libhildondesktop/libhildondesktop.h>
#include <glib/gi18n-lib.h>

#include "nui-core.h"
#include "nui-call-monitor.h"

typedef struct _NuiStatusPlugin NuiStatusPlugin;
typedef struct _NuiStatusPluginClass NuiStatusPluginClass;
typedef struct _NuiStatusPluginPrivate NuiStatusPluginPrivate;

struct _NuiStatusPlugin
{
  HDStatusMenuItem parent;
};

struct _NuiStatusPluginClass
{
  HDStatusMenuItemClass parent;
};

struct _NuiStatusPluginPrivate
{
  NuiCore *core;
  NuiCallMonitor *call_monitor;
  GdkPixbuf *call_icon;
  gboolean disposed;
};

#define PRIVATE(o) \
  ((NuiStatusPluginPrivate *)nui_status_plugin_get_instance_private((NuiStatusPlugin *)(o)))

#define NUI_STATUS_TYPE_PLUGIN (nui_status_plugin_get_type())
#define NUI_STATUS_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NUI_STATUS_TYPE_PLUGIN, NuiStatusPlugin))
#define NUI_STATUS_IS_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  NUI_STATUS_TYPE_PLUGIN))

HD_DEFINE_PLUGIN_MODULE_EXTENDED(
    NuiStatusPlugin,
    nui_status_plugin,
    HD_TYPE_STATUS_MENU_ITEM,
    G_ADD_PRIVATE_DYNAMIC(NuiStatusPlugin), , );


static void
set_call_indicator(NuiStatusPlugin *plugin, gboolean set)
{
  g_return_if_fail(plugin != NULL);

  if (set)
  {
    NuiStatusPluginPrivate *priv = PRIVATE(plugin);

    g_return_if_fail(priv->call_icon != NULL);

    hd_status_plugin_item_set_status_area_icon(
          HD_STATUS_PLUGIN_ITEM(plugin), priv->call_icon);
  }
  else
  {
    hd_status_plugin_item_set_status_area_icon(
          HD_STATUS_PLUGIN_ITEM(plugin), NULL);
  }
}

static void
call_status_changed_cb(int a1, gboolean in_call, gpointer user_data)
{
  g_return_if_fail(NUI_STATUS_IS_PLUGIN(user_data));

  set_call_indicator(NUI_STATUS_PLUGIN(user_data), in_call);
}

static void
nui_status_plugin_class_finalize(NuiStatusPluginClass *klass)
{
}

static void
nui_status_plugin_dispose(GObject *object)
{
  NuiStatusPluginPrivate *priv = PRIVATE(object);

  if (priv->disposed)
    return;

  if (priv->core)
  {
    g_object_unref(priv->core);
    priv->core = NULL;
  }

  if (priv->call_monitor)
  {
    g_signal_handlers_disconnect_by_func(priv->call_monitor,
                                         call_status_changed_cb,
                                         object);
    g_object_unref(priv->call_monitor);
    priv->call_monitor = NULL;
  }

  if (priv->call_icon)
  {
    g_object_unref(priv->call_icon);
    priv->call_icon = NULL;
  }

  priv->disposed = TRUE;

  G_OBJECT_CLASS(nui_status_plugin_parent_class)->dispose(object);
}

static void
nui_status_plugin_class_init(NuiStatusPluginClass *klass)
{
  G_OBJECT_CLASS(klass)->dispose = nui_status_plugin_dispose;
}

static void
nui_status_plugin_init(NuiStatusPlugin *plugin)
{
  NuiStatusPluginPrivate *priv = PRIVATE(plugin);
  GError *error = NULL;
  GtkIconInfo *info;

  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain("rtcom-messaging-ui");

  priv->disposed = FALSE;
  //priv->core = NUI_CORE(nui_core_new());
  priv->call_monitor = NUI_CALL_MONITOR(nui_call_monitor_new());

  if (priv->call_monitor)
  {
    g_signal_connect(priv->call_monitor, "status-changed",
                     G_CALLBACK(call_status_changed_cb), plugin);
  }

  info = gtk_icon_theme_lookup_icon(gtk_icon_theme_get_default(),
                                    "general_call_status",
                                    HILDON_ICON_PIXEL_SIZE_XSMALL, 0);

  if (info)
  {
    const gchar *icon_file = gtk_icon_info_get_filename(info);

    if (icon_file)
    {
      priv->call_icon = gdk_pixbuf_new_from_file(icon_file, &error);

      if (error)
        g_error_free(error);
    }

    gtk_icon_info_free(info);
  }
}
