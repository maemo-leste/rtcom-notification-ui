/*
 * nui-core.h
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

#ifndef __NUI_CORE_H__
#define __NUI_CORE_H__

G_BEGIN_DECLS

#define NUI_TYPE_CORE             (nui_core_get_type ())
#define NUI_CORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NUI_TYPE_CORE, NuiCore))
#define NUI_CORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NUI_TYPE_CORE, NuiCoreClass))
#define NUI_IS_CORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NUI_TYPE_CORE))
#define NUI_IS_CORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NUI_TYPE_CORE))
#define NUI_CORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NUI_TYPE_CORE, NuiCoreClass))

typedef struct _NuiCoreClass NuiCoreClass;
typedef struct _NuiCore NuiCore;

GType nui_core_get_type(void) G_GNUC_CONST;

gpointer nui_core_new();

G_END_DECLS

#endif /* __NUI_CORE_H__ */
