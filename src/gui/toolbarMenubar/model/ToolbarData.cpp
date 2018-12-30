#include "ToolbarData.h"

#include <i18n.h>

#include <gtk/gtk.h>

#include <StringUtils.h>

ToolbarData::ToolbarData(bool predefined)
{
	XOJ_INIT_TYPE(ToolbarData);

	this->predefined = predefined;
}

ToolbarData::ToolbarData(const ToolbarData& data)
{
	XOJ_INIT_TYPE(ToolbarData);

	*this = data;
	this->predefined = false;
}

ToolbarData::~ToolbarData()
{
	XOJ_CHECK_TYPE(ToolbarData);

	for (ToolbarEntry* e : this->contents)
	{
		delete e;
	}
	contents.clear();

	XOJ_RELEASE_TYPE(ToolbarData);
}

string ToolbarData::getName()
{
	XOJ_CHECK_TYPE(ToolbarData);

	return this->name;
}

void ToolbarData::setName(string name)
{
	XOJ_CHECK_TYPE(ToolbarData);

	this->name = name;
}

string ToolbarData::getId()
{
	XOJ_CHECK_TYPE(ToolbarData);

	return this->id;
}

void ToolbarData::setId(string id)
{
	XOJ_CHECK_TYPE(ToolbarData);

	this->id = id;
}

bool ToolbarData::isPredefined()
{
	XOJ_CHECK_TYPE(ToolbarData);

	return this->predefined;
}

void ToolbarData::load(GKeyFile* config, const char* group)
{
	XOJ_CHECK_TYPE(ToolbarData);

	gsize length = 0;
	gchar** keys = g_key_file_get_keys(config, group, &length, NULL);
	if (keys == NULL)
	{
		return;
	}

	gchar* name = g_key_file_get_locale_string(config, group, "name", NULL, NULL);
	if (name != NULL)
	{
		this->name = name;
		g_free(name);
	}

	for (gsize i = 0; i < length; i++)
	{
		if (strcmp(keys[i], "name") == 0 || strncmp(keys[i], "name[", 5) == 0)
		{
			continue;
		}

		ToolbarEntry* e = new ToolbarEntry();
		gsize keyLen = 0;
		e->setName(keys[i]);
		gchar** list = g_key_file_get_string_list(config, group, keys[i], &keyLen, NULL);

		for (gsize x = 0; x < keyLen; x++)
		{
			e->addItem(StringUtils::trim(string(list[x])));
		}

		contents.push_back(e);

		g_strfreev(list);
	}

	g_strfreev(keys);
}

void ToolbarData::saveToKeyFile(GKeyFile* config)
{
	XOJ_CHECK_TYPE(ToolbarData);

	string group = getId();

	for (ToolbarEntry* e : this->contents)
	{
		string line = "";

		for (ToolbarItem* it : e->getItems())
		{
			line += ",";
			line += it->getName();
		}

		if (line.length() > 2)
		{
			g_key_file_set_string(config, group.c_str(), e->getName().c_str(), line.substr(1).c_str());
		}
	}

	g_key_file_set_string(config, group.c_str(), "name", this->name.c_str());
}

int ToolbarData::insertItem(string toolbar, string item, int position)
{
	XOJ_CHECK_TYPE(ToolbarData);

	g_message("%s", FC(FORMAT_STR("ToolbarData::insertItem({1}, {2}, {3});") % toolbar % item % position));

	g_return_val_if_fail(isPredefined() == false, -1);

	for (ToolbarEntry* e : this->contents)
	{
		if (e->getName() == toolbar)
		{
			g_message("%s", FC(_F("Toolbar found: {1}") % toolbar));

			int id = e->insertItem(item, position);

			g_message("%s", FC(FORMAT_STR("return {1}") % id));
			return id;
		}
	}

	ToolbarEntry* newEntry = new ToolbarEntry();
	newEntry->setName(toolbar);
	int id = newEntry->addItem(item);
	contents.push_back(newEntry);

	return id;
}

bool ToolbarData::removeItemByID(string toolbar, int id)
{
	XOJ_CHECK_TYPE(ToolbarData);

	g_return_val_if_fail(isPredefined() == false, false);

	for (ToolbarEntry* e : contents)
	{
		if (e->getName() == toolbar)
		{
			return e->removeItemById(id);
		}
	}

	return false;
}
