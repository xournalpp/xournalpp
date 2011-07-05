#include "ToolbarData.h"
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>

ToolbarData::ToolbarData(bool predefined) {
	XOJ_INIT_TYPE(ToolbarData);

	this->predefined = predefined;
}

ToolbarData::~ToolbarData() {
	XOJ_RELEASE_TYPE(ToolbarData);
}

String ToolbarData::getName() {
	XOJ_CHECK_TYPE(ToolbarData);

	return this->name;
}

void ToolbarData::setName(String name) {
	XOJ_CHECK_TYPE(ToolbarData);

	this->name = name;
}

String ToolbarData::getId() {
	XOJ_CHECK_TYPE(ToolbarData);

	return this->id;
}

bool ToolbarData::isPredefined() {
	XOJ_CHECK_TYPE(ToolbarData);

	return this->predefined;
}

void ToolbarData::load(GKeyFile * config, const char * group) {
	XOJ_CHECK_TYPE(ToolbarData);

	gsize length = 0;
	gchar ** keys = g_key_file_get_keys(config, group, &length, NULL);
	if (keys == NULL) {
		return;
	}

	gchar * name = g_key_file_get_locale_string(config, group, "name", NULL, NULL);
	if (name != NULL) {
		this->name = name;
		g_free(name);
	}

	for (gsize i = 0; i < length; i++) {
		if (strcmp(keys[i], "name") == 0 || strncmp(keys[i], "name[", 5) == 0) {
			continue;
		}

		ToolbarEntry e;
		gsize keyLen = 0;
		e.name = keys[i];
		gchar ** list = g_key_file_get_string_list(config, group, keys[i], &keyLen, NULL);

		for (gsize x = 0; x < keyLen; x++) {
			String s = list[x];
			e.entries.push_back(s.trim());
		}

		this->contents.push_back(e);

		g_strfreev(list);
	}

	g_strfreev(keys);
}

void ToolbarData::saveToKeyFile(GKeyFile * config) {/* TODO: Debug
	const char * group = getId().c_str();

	for (it = this->contents.begin(); it != this->contents.end(); it++) {
		ToolbarEntry & e = *it;

		std::vector<String>::iterator itItem;
		for (itItem = e.entries.begin(); itItem != e.entries.end(); itItem++) {


		if (e.name.equals(toolbar)) {
			std::vector<String>::iterator it2 = e.entries.begin();
			it2 += position;
			e.entries.insert(it2, item);
		}
	}

	//	g_key_file_set_string(config, )
*/
}

void ToolbarData::addItem(String toolbar, String item, int position) {
	g_return_if_fail(isPredefined() == false);

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++) {
		ToolbarEntry & e = *it;

		if (e.name.equals(toolbar)) {
			std::vector<String>::iterator it2 = e.entries.begin();
			it2 += position;
			e.entries.insert(it2, item);
		}
	}
}

void ToolbarData::removeItem(String toolbar, int position) {
	g_return_if_fail(isPredefined() == false);

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++) {
		ToolbarEntry & e = *it;

		if (e.name.equals(toolbar)) {
			std::vector<String>::iterator it2 = e.entries.begin();
			it2 += position;
			e.entries.erase(it2);
		}
	}
}

