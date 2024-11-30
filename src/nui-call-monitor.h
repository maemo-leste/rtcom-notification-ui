/*
 * nui-call-monitor.h
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

#ifndef __NUI_CALL_MONITOR_H__
#define __NUI_CALL_MONITOR_H__

G_BEGIN_DECLS

#define NUI_TYPE_CALL_MONITOR             (nui_call_monitor_get_type ())
#define NUI_CALL_MONITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NUI_TYPE_CALL_MONITOR, NuiCallMonitor))
#define NUI_CALL_MONITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NUI_TYPE_CALL_MONITOR, NuiCallMonitorClass))
#define NUI_IS_CALL_MONITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NUI_TYPE_CALL_MONITOR))
#define NUI_IS_CALL_MONITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NUI_TYPE_CALL_MONITOR))
#define NUI_CALL_MONITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NUI_TYPE_CALL_MONITOR, NuiCallMonitorClass))

typedef struct _NuiCallMonitorClass NuiCallMonitorClass;
typedef struct _NuiCallMonitor NuiCallMonitor;

GType nui_call_monitor_get_type(void) G_GNUC_CONST;

gpointer nui_call_monitor_new();

G_END_DECLS

#endif /* __NUI_CALL_MONITOR_H__ */
