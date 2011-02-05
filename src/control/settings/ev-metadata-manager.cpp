/*
 * ev-metadata-manager.c
 * This file is part of ev
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

/*
 * Modified by the ev Team, 2003. See the AUTHORS file for a 
 * list of people on the ev Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <stdlib.h>

#include <libxml/xmlreader.h>

#include "ev-metadata-manager.h"
#include "../../cfg.h"

#define FILENAME() g_build_filename(g_get_home_dir(), CONFIG_DIR, METADATA_FILE, NULL)

typedef struct _EvMetadataManager EvMetadataManager;

typedef struct _MetaItem MetaItem;

struct _MetaItem {
	time_t atime; /* time of last access */

	GHashTable *values;
};

struct _EvMetadataManager {
	gboolean values_loaded; /* It is true if the file
	 has been read */

	guint timeout_id;

	GHashTable *items;
};

static gboolean ev_metadata_manager_save(gpointer data);

static EvMetadataManager *ev_metadata_manager = NULL;

/**
 * item_free:
 * @data: a pointer to a #Item data
 *
 * It does free the values on the #GHashTable where data points.
 */
static void item_free(gpointer data) {
	MetaItem *item = (MetaItem *) data;

	if (item->values != NULL) {
		g_hash_table_destroy(item->values);
	}

	g_slice_free(MetaItem, item);
}

/**
 * ev_metadata_arm_timeout
 *
 * Setup a timeout for saving the metadata to disk.
 */
static void ev_metadata_arm_timeout(void) {
	if (ev_metadata_manager->timeout_id)
		return;

	ev_metadata_manager->timeout_id = g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE, 2,
			(GSourceFunc) ev_metadata_manager_save, NULL, NULL);
}

/**
 * ev_metadata_manager_init:
 *
 * Creates an EvMetadataManager with default values.
 *
 *  values_loaded   ->  %FALSE.
 *  timeout_id      ->  the id of the event source.
 *  items           ->  a new full empty #GHashTable.
 */
void ev_metadata_manager_init(void) {
	ev_metadata_manager = g_slice_new0(EvMetadataManager);

	ev_metadata_manager->values_loaded = FALSE;

	ev_metadata_manager->items = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, item_free);
}

/* This function must be called before exiting */
void ev_metadata_manager_shutdown(void) {
	if (ev_metadata_manager == NULL) {
		return;
	}

	if (ev_metadata_manager->timeout_id) {
		g_source_remove(ev_metadata_manager->timeout_id);
		ev_metadata_manager->timeout_id = 0;
		ev_metadata_manager_save(NULL);
	}

	if (ev_metadata_manager->items != NULL) {
		g_hash_table_destroy(ev_metadata_manager->items);
	}

	g_slice_free(EvMetadataManager, ev_metadata_manager);
	ev_metadata_manager = NULL;
}

static void value_free(gpointer data) {
	GValue *value = (GValue *) data;

	g_value_unset(value);
	g_slice_free(GValue, value);
}

static GValue *
parse_value(xmlChar *value, xmlChar *type) {
	GType ret_type;
	GValue *ret;

	ret_type = g_type_from_name((char *) type);
	ret = g_slice_new0(GValue);
	g_value_init(ret, ret_type);

	switch (ret_type) {
	case G_TYPE_STRING:
		g_value_set_string(ret, (char *) value);
		break;
	case G_TYPE_INT:
		g_value_set_int(ret, g_ascii_strtoull((char *) value, NULL, 0));
		break;
	case G_TYPE_DOUBLE:
		g_value_set_double(ret, g_ascii_strtod((char *) value, NULL));
		break;
	case G_TYPE_BOOLEAN:
		g_value_set_boolean(ret, g_ascii_strtoull((char *) value, NULL, 0));
		break;
	}

	return ret;
}

static void parseItem(xmlDocPtr doc, xmlNodePtr cur) {
	MetaItem *item;

	xmlChar *uri;
	xmlChar *atime;

	if (xmlStrcmp(cur->name, (const xmlChar *) "document") != 0) {
		return;
	}

	uri = xmlGetProp(cur, (const xmlChar *) "uri");
	if (uri == NULL) {
		return;
	}

	atime = xmlGetProp(cur, (const xmlChar *) "atime");
	if (atime == NULL) {
		xmlFree(uri);
		return;
	}

	item = g_slice_new0(MetaItem);

	item->atime = g_ascii_strtoull((char*) atime, NULL, 0);

	item->values = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, value_free);

	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		if (xmlStrcmp(cur->name, (const xmlChar *) "entry") == 0) {
			xmlChar *key;
			xmlChar *xml_value;
			xmlChar *type;
			GValue *value;

			key = xmlGetProp(cur, (const xmlChar *) "key");
			xml_value = xmlGetProp(cur, (const xmlChar *) "value");
			type = xmlGetProp(cur, (const xmlChar *) "type");
			value = parse_value(xml_value, type);

			if ((key != NULL) && (value != NULL))
				g_hash_table_insert(item->values, xmlStrdup(key), value);

			if (key != NULL)
				xmlFree(key);
			if (type != NULL)
				xmlFree(type);
			if (xml_value != NULL)
				xmlFree(xml_value);
		}

		cur = cur->next;
	}

	g_hash_table_insert(ev_metadata_manager->items, xmlStrdup(uri), item);

	xmlFree(uri);
	xmlFree(atime);
}

static gboolean load_values() {
	xmlDocPtr doc;
	xmlNodePtr cur;
	gchar *file_name;

	g_return_val_if_fail(ev_metadata_manager != NULL, FALSE);
	g_return_val_if_fail(ev_metadata_manager->values_loaded == FALSE, FALSE);

	ev_metadata_manager->values_loaded = TRUE;

	xmlKeepBlanksDefault(0);

	file_name = FILENAME();
	if (!g_file_test(file_name, G_FILE_TEST_EXISTS)) {
		g_free(file_name);
		return FALSE;
	}

	doc = xmlParseFile(file_name);
	g_free(file_name);

	if (doc == NULL) {
		return FALSE;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		g_message("The metadata file \"%s\" is empty", METADATA_FILE);
		xmlFreeDoc(doc);

		return FALSE;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "metadata")) {
		g_message("File \"%s\" is of the wrong type", METADATA_FILE);
		xmlFreeDoc(doc);

		return FALSE;
	}

	cur = xmlDocGetRootElement(doc);
	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		parseItem(doc, cur);

		cur = cur->next;
	}

	xmlFreeDoc(doc);

	return TRUE;
}

#define LAST_URI "last-used-value"

static gboolean ev_metadata_manager_get_last(const gchar *key, GValue *value, gboolean ignore) {
	MetaItem *item;
	GValue *ret;

	g_assert(ev_metadata_manager->values_loaded);

	if (ignore)
		return FALSE;

	item = (MetaItem *) g_hash_table_lookup(ev_metadata_manager->items, LAST_URI);

	if (item == NULL)
		return FALSE;

	item->atime = time(NULL);

	if (item->values == NULL)
		return FALSE;

	ret = (GValue *) g_hash_table_lookup(item->values, key);

	if (ret != NULL) {
		g_value_init(value, G_VALUE_TYPE(ret));
		g_value_copy(ret, value);
		return TRUE;
	}

	return FALSE;
}

static void ev_metadata_manager_set_last(const gchar *key, const GValue *value) {
	MetaItem *item;

	g_assert(ev_metadata_manager->values_loaded);

	item = (MetaItem *) g_hash_table_lookup(ev_metadata_manager->items, LAST_URI);

	if (item == NULL) {
		item = g_slice_new0(MetaItem);

		g_hash_table_insert(ev_metadata_manager->items, g_strdup(LAST_URI), item);
	}

	if (item->values == NULL)
		item->values = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, value_free);
	if (value != NULL) {
		GValue *newVal;

		newVal = g_slice_new0(GValue);
		g_value_init(newVal, G_VALUE_TYPE(value));
		g_value_copy(value, newVal);

		g_hash_table_insert(item->values, g_strdup(key), newVal);
	} else {
		g_hash_table_remove(item->values, key);
	}

	item->atime = time(NULL);
	ev_metadata_arm_timeout();
	return;
}

/**
 * ev_metadata_manager_get:
 * @uri: Uri to set data for, if @NULL, we return default value
 * @key: Key to set uri
 * @value: GValue struct filled up with value
 * @ignore_last: if @TRUE, default value is ignored
 * 
 * Retrieve value for uri in metadata database
 * 
 * Returns: @TRUE if value was taken.
 **/
gboolean ev_metadata_manager_get(const gchar *uri, const gchar *key, GValue *value, gboolean ignore_last) {
	MetaItem *item;
	GValue *ret;

	g_return_val_if_fail(key != NULL, FALSE);

	if (ev_metadata_manager == NULL)
		return FALSE;

	if (!ev_metadata_manager->values_loaded) {
		gboolean res;

		res = load_values();

		if (!res)
			return ev_metadata_manager_get_last(key, value, ignore_last);
	}

	if (uri == NULL)
		return ev_metadata_manager_get_last(key, value, ignore_last);

	item = (MetaItem *) g_hash_table_lookup(ev_metadata_manager->items, uri);

	if (item == NULL)
		return ev_metadata_manager_get_last(key, value, ignore_last);

	item->atime = time(NULL);

	if (item->values == NULL)
		return ev_metadata_manager_get_last(key, value, ignore_last);

	ret = (GValue *) g_hash_table_lookup(item->values, key);

	if (ret != NULL) {
		g_value_init(value, G_VALUE_TYPE(ret));
		g_value_copy(ret, value);
		return TRUE;
	}

	return ev_metadata_manager_get_last(key, value, ignore_last);
}

/**
 * ev_metadata_manager_set:
 * @uri: Uri to set data for, if @NULL, we set default value
 * @key: Key to set uri
 * @value: GValue struct containing value
 * 
 * Set value for key in metadata database
 **/
void ev_metadata_manager_set(const gchar *uri, const gchar *key, const GValue *value) {
	MetaItem *item;

	g_return_if_fail(key != NULL);

	if (ev_metadata_manager == NULL) {
		return;
	}

	if (!ev_metadata_manager->values_loaded) {
		gboolean res;

		res = load_values();

		if (!res)
			return;
	}

	if (uri == NULL) {
		ev_metadata_manager_set_last(key, value);
		return;
	}

	item = (MetaItem *) g_hash_table_lookup(ev_metadata_manager->items, uri);

	if (item == NULL) {
		item = g_slice_new0(MetaItem);

		g_hash_table_insert(ev_metadata_manager->items, g_strdup(uri), item);
	}

	if (item->values == NULL)
		item->values = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, value_free);
	if (value != NULL) {
		GValue *newVal;

		newVal = g_slice_new0(GValue);
		g_value_init(newVal, G_VALUE_TYPE(value));
		g_value_copy(value, newVal);

		g_hash_table_insert(item->values, g_strdup(key), newVal);
		ev_metadata_manager_set_last(key, value);
	} else {
		g_hash_table_remove(item->values, key);
	}

	item->atime = time(NULL);

	ev_metadata_arm_timeout();
}

static void save_values(const gchar *key, GValue *value, xmlNodePtr parent) {
	char *string_value;
	xmlNodePtr xml_node;

	g_return_if_fail(key != NULL);

	if (value == NULL)
		return;

	xml_node = xmlNewChild(parent, NULL, (const xmlChar *) "entry", NULL);

	xmlSetProp(xml_node, (const xmlChar *) "key", (const xmlChar *) key);
	xmlSetProp(xml_node, (const xmlChar *) "type", (const xmlChar *) g_type_name(G_VALUE_TYPE(value)));

	switch (G_VALUE_TYPE(value)) {
	case G_TYPE_STRING:
		string_value = g_value_dup_string(value);
		break;
	case G_TYPE_INT:
		string_value = g_strdup_printf("%d", g_value_get_int(value));
		break;
	case G_TYPE_DOUBLE: {
		gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
		g_ascii_dtostr(buf, G_ASCII_DTOSTR_BUF_SIZE, g_value_get_double(value));
		string_value = g_strdup(buf);
	}
		break;
	case G_TYPE_BOOLEAN:
		string_value = g_strdup_printf("%d", g_value_get_boolean(value));
		break;
	default:
		string_value = NULL;
		g_assert_not_reached();
	}

	xmlSetProp(xml_node, (const xmlChar *) "value", (const xmlChar *) string_value);

	g_free(string_value);
}

static void save_item(const gchar *key, const gpointer *data, xmlNodePtr parent) {
	xmlNodePtr xml_node;
	const MetaItem *item = (const MetaItem *) data;
	gchar *atime;

	g_return_if_fail(key != NULL);

	if (item == NULL)
		return;

	xml_node = xmlNewChild(parent, NULL, (const xmlChar *) "document", NULL);

	xmlSetProp(xml_node, (const xmlChar *) "uri", (const xmlChar *) key);

	atime = g_strdup_printf("%ld", item->atime);
	xmlSetProp(xml_node, (const xmlChar *) "atime", (const xmlChar *) atime);
	g_free(atime);

	g_hash_table_foreach(item->values, (GHFunc) save_values, xml_node);
}

static void get_oldest(const gchar *key, const gpointer value, const gchar ** key_to_remove) {
	const MetaItem *item = (const MetaItem *) value;

	if (*key_to_remove == NULL) {
		*key_to_remove = key;
	} else {
		const MetaItem *item_to_remove = (MetaItem *)g_hash_table_lookup(ev_metadata_manager->items, *key_to_remove);

		g_return_if_fail(item_to_remove != NULL);

		if (item->atime < item_to_remove->atime) {
			*key_to_remove = key;
		}
	}
}

static void resize_items() {
	while (g_hash_table_size(ev_metadata_manager->items) > METADATA_MAX_ITEMS) {
		gpointer key_to_remove = NULL;

		g_hash_table_foreach(ev_metadata_manager->items, (GHFunc) get_oldest, &key_to_remove);

		g_return_if_fail(key_to_remove != NULL);

		g_hash_table_remove(ev_metadata_manager->items, key_to_remove);
	}
}

static gboolean ev_metadata_manager_save(gpointer data) {
	xmlDocPtr doc;
	xmlNodePtr root;
	gchar *file_name;

	ev_metadata_manager->timeout_id = 0;

	resize_items();

	xmlIndentTreeOutput = TRUE;

	doc = xmlNewDoc((const xmlChar *) "1.0");
	if (doc == NULL)
		return TRUE;

	/* Create metadata root */
	root = xmlNewDocNode(doc, NULL, (const xmlChar *) "metadata", NULL);
	xmlDocSetRootElement(doc, root);

	g_hash_table_foreach(ev_metadata_manager->items, (GHFunc) save_item, root);

	file_name = FILENAME();
	xmlSaveFormatFile(file_name, doc, 1);
	g_free(file_name);

	xmlFreeDoc(doc);

	return FALSE;
}

void ev_metadata_manager_set_int(const gchar *uri, const gchar *key, int value) {
	GValue val = { 0, };

	g_value_init(&val, G_TYPE_INT);
	g_value_set_int(&val, value);

	ev_metadata_manager_set(uri, key, &val);

	g_value_unset(&val);
}

void ev_metadata_manager_set_double(const gchar *uri, const gchar *key, double value) {
	GValue val = { 0, };

	g_value_init(&val, G_TYPE_DOUBLE);
	g_value_set_double(&val, value);

	ev_metadata_manager_set(uri, key, &val);

	g_value_unset(&val);
}

void ev_metadata_manager_set_string(const gchar *uri, const gchar *key, const gchar *value) {
	GValue val = { 0, };

	g_value_init(&val, G_TYPE_STRING);
	g_value_set_static_string(&val, value);

	ev_metadata_manager_set(uri, key, &val);

	g_value_unset(&val);
}

void ev_metadata_manager_set_boolean(const gchar *uri, const gchar *key, gboolean value) {
	GValue val = { 0, };

	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, value);

	ev_metadata_manager_set(uri, key, &val);

	g_value_unset(&val);
}
