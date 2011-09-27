#include "ToolbarData.h"
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>

ToolbarData::ToolbarData(bool predefined) {
	XOJ_INIT_TYPE(ToolbarData);

	this->predefined = predefined;
}

ToolbarData::ToolbarData(const ToolbarData & data) {
	XOJ_INIT_TYPE(ToolbarData);

	*this = data;
	this->predefined = false;
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

void ToolbarData::setId(String id) {
	XOJ_CHECK_TYPE(ToolbarData);

	this->id = id;
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

void ToolbarData::saveToKeyFile(GKeyFile * config) {
	const char * group = getId().c_str();

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++) {
		ToolbarEntry & e = *it;

		String line = "";

		std::vector<ToolbarItem>::iterator itItem;
		for (itItem = e.entries.begin(); itItem != e.entries.end(); itItem++) {
			line += ",";
			line += *itItem;
		}

		if (line.length() < 2) {
			g_key_file_set_string(config, group, e.name.c_str(), line.substring(1).c_str());
		}
	}

	g_key_file_set_string(config, group, "name", this->name.c_str());
}

void ToolbarData::addItem(String toolbar, String item, int position) {
	g_return_if_fail(isPredefined() == false);

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++) {
		ToolbarEntry & e = *it;

		if (e.name.equals(toolbar)) {
			std::vector<ToolbarItem>::iterator it2 = e.entries.begin();
			it2 += position;
			e.entries.insert(it2, ToolbarItem(item));
			return;
		}
	}

	ToolbarEntry newEntry;
	newEntry.name = toolbar;
	newEntry.entries.push_back(ToolbarItem(item));
	this->contents.push_back(newEntry);
}

bool ToolbarData::removeItemByID(String toolbar, int id) {
	g_return_val_if_fail(isPredefined() == false, false);

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++) {
		ToolbarEntry & e = *it;

		if (e.name.equals(toolbar)) {
			std::vector<ToolbarItem>::iterator it2 = e.entries.begin();

			for (; it2 != e.entries.end(); it2++) {
				if ((*it2).getId() == id) {
					String erased = *it2;
					e.entries.erase(it2);

					printf("removeItemByID %s from Toolbar %s\n", erased.c_str(), toolbar.c_str());
					return true;
				}
			}
		}
	}

	return false;
}

bool ToolbarData::removeItem(String toolbar, int position) {
	g_return_val_if_fail(isPredefined() == false, false);

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++) {
		ToolbarEntry & e = *it;

		if (e.name.equals(toolbar)) {
			std::vector<ToolbarItem>::iterator it2 = e.entries.begin();
			it2 += position;
			String erased = *it2;
			e.entries.erase(it2);

			printf("removeItem %s from Toolbar %s\n", erased.c_str(), toolbar.c_str());
			return true;
		}
	}

	return false;
}

