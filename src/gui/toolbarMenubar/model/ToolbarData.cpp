#include "ToolbarData.h"
#include <string.h>

ToolbarData::ToolbarData(bool predefined) {
	this->predefined = predefined;
}

String ToolbarData::getName() {
	return this->name;
}

void ToolbarData::setName(String name) {
	this->name = name;
}

String ToolbarData::getId() {
	return this->id;
}

bool ToolbarData::isPredefined() {
	return this->predefined;
}

void ToolbarData::load(GKeyFile * config, const char * group) {
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

		contents.push_back(e);

		g_strfreev(list);
	}

	g_strfreev(keys);
}
