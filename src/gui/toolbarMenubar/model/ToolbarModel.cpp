#include "ToolbarModel.h"
#include "ToolbarData.h"

#include <string.h>

ToolbarModel::ToolbarModel() {
	XOJ_INIT_TYPE(ToolbarModel);

	this->toolbars = NULL;
	this->colorNameTable = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}

ToolbarModel::~ToolbarModel() {
	XOJ_CHECK_TYPE(ToolbarModel);

	for (GList * l = this->toolbars; l != NULL; l = l->next) {
		delete (ToolbarData *) l->data;
	}
	g_list_free(this->toolbars);
	this->toolbars = NULL;

	g_hash_table_destroy(this->colorNameTable);
	this->colorNameTable = NULL;

	XOJ_RELEASE_TYPE(ToolbarModel);
}

ListIterator<ToolbarData *> ToolbarModel::iterator() {
	XOJ_CHECK_TYPE(ToolbarModel);

	return ListIterator<ToolbarData *> (this->toolbars);
}

void ToolbarModel::parseGroup(GKeyFile * config, const char * group, bool predefined) {
	XOJ_CHECK_TYPE(ToolbarModel);

	ToolbarData * data = new ToolbarData(predefined);

	String name;
	if (predefined) {
		name = "predef_";
	} else {
		name = "custom_";
	}

	data->name = name;
	data->id = group;

	data->load(config, group);

	add(data);
}

void ToolbarModel::remove(ToolbarData * data) {
	XOJ_CHECK_TYPE(ToolbarModel);

	this->toolbars = g_list_remove(this->toolbars, data);
}

void ToolbarModel::add(ToolbarData * data) {
	XOJ_CHECK_TYPE(ToolbarModel);

	this->toolbars = g_list_append(this->toolbars, data);
}

const char * ToolbarModel::getColorName(const char * color) {
	return (char *) g_hash_table_lookup(this->colorNameTable, color);
}

bool ToolbarModel::parse(const char * file, bool predefined) {
	XOJ_CHECK_TYPE(ToolbarModel);

	GKeyFile * config = g_key_file_new();
	g_key_file_set_list_separator(config, ',');
	if (!g_key_file_load_from_file(config, file, G_KEY_FILE_NONE, NULL)) {
		g_key_file_free(config);
		return false;
	}

	gsize lenght = 0;
	gchar ** groups = g_key_file_get_groups(config, &lenght);

	for (gsize i = 0; i < lenght; i++) {
		if (groups[i][0] == '_') {
			if (strcmp("_ColorNames", groups[i]) == 0) {
				parseColors(config, groups[i]);
			}

			continue;
		}

		parseGroup(config, groups[i], predefined);
	}

	g_strfreev(groups);
	g_key_file_free(config);
	return true;
}

void ToolbarModel::parseColors(GKeyFile * config, const char * group) {
	XOJ_CHECK_TYPE(ToolbarModel);

	gsize length = 0;
	gchar ** keys = g_key_file_get_keys(config, group, &length, NULL);
	if (keys == NULL) {
		return;
	}

	for (gsize i = 0; i < length; i++) {
		if (strstr(keys[i], "[")) {
			// skip localized keys
			continue;
		}

		char * name = g_key_file_get_locale_string(config, group, keys[i], NULL, NULL);
		g_hash_table_insert(this->colorNameTable, g_strdup(keys[i]), name);
	}

	g_strfreev(keys);
}
