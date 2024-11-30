/*
 * nui-call-monitor.c
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

#include "org.ofono.Manager.h"
#include "org.ofono.Modem.h"
#include "org.ofono.VoiceCallManager.h"
#include "org.ofono.VoiceCall.h"
#include "nui-call-monitor.h"

#define OFONO_BUS_TYPE G_BUS_TYPE_SYSTEM
#define OFONO_SERVICE "org.ofono"

#define OFONO_(interface) OFONO_SERVICE "." interface
#define OFONO_VOICECALL_MANAGER_INTERFACE_NAME OFONO_("VoiceCallManager")

#define OFONO_MODEM_PROPERTY_INTERFACES "Interfaces"
#define OFONO_VOICE_CALL_PROPERTY_STATE "State"

struct _NuiCallMonitor
{
  GObject parent;
};

struct _NuiCallMonitorClass
{
  GObjectClass parent_class;
};

struct _NuiCallMonitorPrivate
{
  NuiOfonoManager *manager;
  GHashTable *modems;
  GHashTable *calls;
  guint active;
  gboolean disposed;
};

typedef struct _NuiCallMonitorPrivate NuiCallMonitorPrivate;

#define PRIVATE(o) \
    ((NuiCallMonitorPrivate *)nui_call_monitor_get_instance_private( \
      (NuiCallMonitor *)(o)))

G_DEFINE_TYPE_WITH_PRIVATE(
  NuiCallMonitor,
  nui_call_monitor,
  G_TYPE_OBJECT
);

enum
{
  STATUS_CHAGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
_call_state_changed(NuiCallMonitor *monitor, NuiOfonoVoiceCall *proxy,
                    GVariant *v)
{
  NuiCallMonitorPrivate *priv = PRIVATE(monitor);
  const char *state = g_variant_get_string(v, NULL);
  gint was_active;
  gint active;

  g_debug("Call state changed %s", state ? state : "unknown");

  active = !g_strcmp0(state, "active") || !g_strcmp0(state, "held");
  was_active = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(proxy), "active"));

  if (was_active == -1)
  {
    was_active = FALSE;
    g_object_set_data(G_OBJECT(proxy), "active", GINT_TO_POINTER(active));
  }

  if (active != was_active)
  {
    if (active)
    {
      priv->active++;

      if (priv->active == 1)
        g_signal_emit(monitor, signals[STATUS_CHAGED], 0, TRUE);
    }
    else
    {
      priv->active--;

      if (!priv->active)
        g_signal_emit(monitor, signals[STATUS_CHAGED], 0, FALSE);
    }

    g_object_set_data(G_OBJECT(proxy), "active", GINT_TO_POINTER(active));
  }
}

static void
_call_property_changed_cb(NuiOfonoVoiceCall *proxy, const gchar *name,
                          GVariant *value, gpointer user_data)
{
  NuiCallMonitor *monitor = user_data;

  if (!strcmp(name, OFONO_VOICE_CALL_PROPERTY_STATE))
  {
    GVariant *v = g_variant_get_variant(value);

    g_debug("Call properties changed");

    _call_state_changed(monitor, proxy, v);
    g_variant_unref(v);
  }
}

static void
_call_destroy(gpointer data)
{
  NuiOfonoVoiceCall *call = data;
  NuiCallMonitor *monitor = g_object_get_data(G_OBJECT(call), "monitor");
  NuiCallMonitorPrivate *priv = PRIVATE(monitor);
  gboolean active;

  g_signal_handlers_disconnect_by_func(
        G_OBJECT(call), _call_property_changed_cb, monitor);

  active = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(call), "active"));

  if (active)
  {
    g_warn_if_fail(priv->active > 0);

    if (priv->active)
    {
      priv->active--;

      if (!priv->active)
        g_signal_emit(monitor, signals[STATUS_CHAGED], 0, FALSE);
    }
  }

  g_object_unref(call);
}

static void
_call_properties_ready_cb(GObject *object, GAsyncResult *res,
                          gpointer user_data)
{
  NuiCallMonitor *monitor = user_data;
  NuiOfonoVoiceCall *call = NUI_OFONO_VOICE_CALL(object);
  GVariant *properties;
  GError *error = NULL;

  if (nui_ofono_voice_call_call_get_properties_finish(call, &properties, res,
                                                      &error))
  {
    if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(call), "active")) == -1)
    {
      GVariant *v = g_variant_lookup_value(properties,
                                           OFONO_VOICE_CALL_PROPERTY_STATE,
                                           NULL);
      if (v)
      {
        _call_state_changed(monitor, call, v);
        g_variant_unref(v);
      }
    }

    g_variant_unref(properties);
  }
  else
  {
    g_warning("Error getting OFONO voice call properties [%s]", error->message);
    g_error_free(error);
  }

  g_object_unref(monitor);
}

static void
_call_ready_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
  NuiCallMonitor *monitor = user_data;
  NuiCallMonitorPrivate *priv = PRIVATE(monitor);
  NuiOfonoVoiceCall *call;
  GError *error = NULL;

  call = nui_ofono_voice_call_proxy_new_for_bus_finish(res, &error);

  if (call)
  {
    gchar *path;

    g_object_get(G_OBJECT(call), "g-object-path", &path, NULL);
    g_object_set_data(G_OBJECT(call), "monitor", monitor);
    g_hash_table_insert(priv->calls, path, call);
    g_signal_connect(call, "property-changed",
                     G_CALLBACK(_call_property_changed_cb), monitor);
    /* in case property-changed is emitted while we are wating for the
     * get_properties async call
     */
    g_object_set_data(G_OBJECT(call), "active", GINT_TO_POINTER(-1));
    nui_ofono_voice_call_call_get_properties(
          call, NULL, _call_properties_ready_cb, monitor);
  }
  else
  {
    g_warning("Error creating OFONO voice call proxy [%s]", error->message);
    g_error_free(error);
    g_object_unref(monitor);
  }
}

static void
_vcm_call_added_cb(NuiOfonoVoiceCallManager *proxy, const gchar *path,
                   GVariant *properties, gpointer user_data)
{
  NuiCallMonitor *monitor = user_data;

  g_debug("call added %s", path);

  nui_ofono_voice_call_proxy_new_for_bus(
        OFONO_BUS_TYPE, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
        OFONO_SERVICE, path, NULL, _call_ready_cb, g_object_ref(monitor));
}

static void
_vcm_call_removed_cb(NuiOfonoVoiceCallManager *proxy, const gchar *path,
                     gpointer user_data)
{
  NuiCallMonitor *monitor = user_data;
  NuiCallMonitorPrivate *priv = PRIVATE(monitor);

  g_debug("call removed %s", path);

  g_hash_table_remove(priv->calls, path);
}

static void
_vcm_destroy(gpointer data)
{
  NuiOfonoVoiceCallManager *vcm = data;
  NuiCallMonitor *monitor = g_object_get_data(G_OBJECT(vcm), "monitor");

  g_signal_handlers_disconnect_by_func(
        G_OBJECT(vcm), _vcm_call_added_cb, monitor);
  g_signal_handlers_disconnect_by_func(
        G_OBJECT(vcm), _vcm_call_removed_cb, monitor);
  g_object_unref(vcm);
}

static void
_modem_parse_interfaces(NuiCallMonitor *monitor, NuiOfonoModem *modem,
                        GVariant *interfaces)
{
  GVariantIter i;
  const gchar *iface;
  gboolean has_vcm = FALSE;

  g_variant_iter_init(&i, interfaces);

  while (g_variant_iter_loop(&i, "s", &iface))
  {
    if (!strcmp(iface, OFONO_VOICECALL_MANAGER_INTERFACE_NAME))
    {
      has_vcm = TRUE;
      break;
    }
  }

  if (has_vcm)
  {
    if (!g_object_get_data(G_OBJECT(modem), "vcm"))
    {
      gchar *path;
      GError *error = NULL;
      NuiOfonoVoiceCallManager *vcm;

      g_object_get(G_OBJECT(modem), "g-object-path", &path, NULL);

      g_return_if_fail(path != NULL);

      vcm = nui_ofono_voice_call_manager_proxy_new_for_bus_sync(
            OFONO_BUS_TYPE, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
            OFONO_SERVICE, path, NULL, &error);

      if (error)
      {
        g_warning("Error creating OFONO voice call manager proxy for %s [%s]",
                  path, error->message);
        g_error_free(error);
      }

      g_free(path);

      if (vcm)
      {
        g_object_set_data_full(G_OBJECT(modem), "vcm", vcm, _vcm_destroy);
        g_object_set_data(G_OBJECT(vcm), "monitor", monitor);
        g_signal_connect(vcm, "call-added",
                         G_CALLBACK(_vcm_call_added_cb), monitor);
        g_signal_connect(vcm, "call-removed",
                         G_CALLBACK(_vcm_call_removed_cb), monitor);
      }
    }
  }
  else
    g_object_set_data(G_OBJECT(modem), "vcm", NULL);
}

static void
_modem_property_changed_cb(NuiOfonoModem *proxy, const gchar *name,
                           GVariant *value, gpointer user_data)
{
  if (!strcmp(name, OFONO_MODEM_PROPERTY_INTERFACES))
  {
    GVariant *v = g_variant_get_variant(value);

    _modem_parse_interfaces(user_data, proxy, v);
    g_variant_unref(v);
  }
}

static void
_modem_add(NuiCallMonitor *monitor, const gchar *path, GVariant *properties)
{
  GError *error = NULL;
  NuiCallMonitorPrivate *priv = PRIVATE(monitor);
  NuiOfonoModem *proxy;

  if (g_hash_table_lookup(priv->modems, path))
    return;

  g_debug("Adding modem %s", path);

  proxy = nui_ofono_modem_proxy_new_for_bus_sync(
          OFONO_BUS_TYPE, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
          OFONO_SERVICE, path, NULL, &error);

  if (proxy)
  {
    GVariantIter i;
    GVariant *v;
    const gchar *name;

    g_hash_table_insert(priv->modems, g_strdup(path), proxy);
    g_variant_iter_init(&i, properties);

    while (g_variant_iter_loop(&i, "{&sv}", &name, &v))
    {
      if (!strcmp(name, OFONO_MODEM_PROPERTY_INTERFACES))
        _modem_parse_interfaces(monitor, proxy, v);
    }

    g_signal_connect(proxy, "property-changed",
                     G_CALLBACK(_modem_property_changed_cb), monitor);
  }
  else
  {
    g_warning("Error creating OFONO modem %s proxy [%s]", path,
              error->message);
    g_error_free(error);
  }
}

static void
_modem_added_cb(NuiOfonoManager *manager, const gchar *path,
                GVariant *properties, gpointer user_data)
{
  _modem_add(user_data, path, properties);
}

static void
_modem_removed_cb(NuiOfonoManager *manager, const gchar *path,
                  gpointer user_data)
{
  NuiCallMonitor *monitor = user_data;
  NuiCallMonitorPrivate *priv = PRIVATE(monitor);
  NuiOfonoModem *proxy;
  GHashTableIter iter;
  gpointer key, value;

  g_debug("Modem %s removed", path);

  g_hash_table_iter_init (&iter, priv->calls);

  while (g_hash_table_iter_next (&iter, &key, &value))
  {
    if (g_str_has_prefix(key, path))
      g_hash_table_iter_remove(&iter);
  }

  proxy = g_hash_table_lookup(priv->modems, path);

  if (proxy)
  {
    g_signal_handlers_disconnect_by_func(G_OBJECT(proxy),
                                         _modem_property_changed_cb, monitor);

    g_hash_table_remove(priv->modems, path);
  }
}

static void
_modems_ready_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
  NuiCallMonitor *monitor = user_data;
  NuiOfonoManager *proxy = NUI_OFONO_MANAGER(object);
  GVariant *modems;
  GError *error = NULL;

  if (nui_ofono_manager_call_get_modems_finish(proxy, &modems, res, &error))
  {
    GVariantIter i;
    GVariant *properties;
    const gchar *path;

    g_variant_iter_init(&i, modems);

    while (g_variant_iter_loop(&i, "(&o@a{sv})", &path, &properties))
      _modem_add(monitor, path, properties);

    g_variant_unref(modems);
  }
  else
  {
    g_warning("Error getting OFONO modems [%s]", error->message);
    g_error_free(error);
  }

  g_object_unref(monitor);
}

static void
_manager_ready_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
  NuiCallMonitor *monitor = user_data;
  NuiCallMonitorPrivate *priv = PRIVATE(monitor);
  GError *error = NULL;

  priv->manager = nui_ofono_manager_proxy_new_for_bus_finish(res, &error);

  if (priv->manager)
  {
    g_signal_connect(priv->manager, "modem-added",
                     G_CALLBACK(_modem_added_cb), monitor);
    g_signal_connect(priv->manager, "modem-removed",
                     G_CALLBACK(_modem_removed_cb), monitor);

    nui_ofono_manager_call_get_modems(priv->manager, NULL,
                                      _modems_ready_cb, monitor);
  }
  else
  {
    g_warning("Error creating OFONO manager [%s]", error->message);
    g_error_free(error);
    g_object_unref(monitor);
  }
}

static void
nui_call_monitor_init(NuiCallMonitor *monitor)
{
  NuiCallMonitorPrivate *priv = PRIVATE(monitor);

  priv->modems = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, g_object_unref);
  priv->calls = g_hash_table_new_full(
        g_str_hash, g_str_equal, g_free, _call_destroy);

  nui_ofono_manager_proxy_new_for_bus(
        OFONO_BUS_TYPE, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
        OFONO_SERVICE, "/", NULL, _manager_ready_cb, g_object_ref(monitor));
}

static void
nui_call_monitor_dispose(GObject *object)
{
  NuiCallMonitorPrivate *priv = PRIVATE(object);

  if (!priv->disposed)
  {
    g_hash_table_unref(priv->calls);
    g_hash_table_unref(priv->modems);

    if (priv->manager)
    {
      g_signal_handlers_disconnect_by_func(G_OBJECT(priv->manager),
                                           _modem_added_cb, object);
      g_signal_handlers_disconnect_by_func(G_OBJECT(priv->manager),
                                           _modem_removed_cb, object);
      g_object_unref(priv->manager);
    }

    priv->disposed = TRUE;
    G_OBJECT_CLASS(nui_call_monitor_parent_class)->dispose(object);
  }
}

static void
nui_call_monitor_class_init(NuiCallMonitorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = nui_call_monitor_dispose;

  signals[STATUS_CHAGED] =
      g_signal_new(
        "status-changed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_VOID__BOOLEAN,
        G_TYPE_NONE,
        1, G_TYPE_BOOLEAN);
}

gpointer nui_call_monitor_new()
{
  return g_object_new(NUI_TYPE_CALL_MONITOR, NULL);
}
