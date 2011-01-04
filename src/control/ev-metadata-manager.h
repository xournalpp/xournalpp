/*
 * ev-metadata-manager.h
 *
 * Copyright (C) 2003  Paolo Maggi 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

#ifndef __EV_METADATA_MANAGER_H__
#define __EV_METADATA_MANAGER_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

void ev_metadata_manager_init(void);
gboolean ev_metadata_manager_get(const gchar *uri, const gchar *key, GValue *value, gboolean ignore_last);
void ev_metadata_manager_set(const gchar *uri, const gchar *key, const GValue *value);
void ev_metadata_manager_set_int(const gchar *uri, const gchar *key, int value);
void ev_metadata_manager_set_double(const gchar *uri, const gchar *key, double value);
void ev_metadata_manager_set_string(const gchar *uri, const gchar *key, const gchar *value);
void ev_metadata_manager_set_boolean(const gchar *uri, const gchar *key, gboolean value);
void ev_metadata_manager_shutdown(void);

G_END_DECLS

#endif /* __EV_METADATA_MANAGER_H__ */
