#include "ToolbarData.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <iostream>

using namespace std;

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

		ToolbarEntry e;
		gsize keyLen = 0;
		e.setName(keys[i]);
		gchar** list = g_key_file_get_string_list(config, group, keys[i], &keyLen,
												  NULL);

		for (gsize x = 0; x < keyLen; x++)
		{
			e.addItem(ba::trim_copy(string(list[x])));
		}

		this->contents.push_back(e);

		g_strfreev(list);
	}

	g_strfreev(keys);
}

void ToolbarData::saveToKeyFile(GKeyFile* config)
{
	XOJ_CHECK_TYPE(ToolbarData);

	const char* group = getId().c_str();

	for (ToolbarEntry& e : this->contents)
	{
		string line = "";

		ListIterator<ToolbarItem*> it = e.iterator();
		while (it.hasNext())
		{
			line += ",";
			line += *it.next();
		}

		if (line.length() > 2)
		{
			g_key_file_set_string(config, group, e.getName().c_str(),
								  line.substr(1).c_str());
		}
	}

	g_key_file_set_string(config, group, "name", this->name.c_str());
}

int ToolbarData::insertItem(string toolbar, string item, int position)
{
	XOJ_CHECK_TYPE(ToolbarData);

	cout << bl::format("ToolbarData::insertItem({1}, {2}, {3});") % toolbar % item % position << endl;

	g_return_val_if_fail(isPredefined() == false, -1);

	for (ToolbarEntry e : this->contents)
	{
		if (e.getName() == toolbar)
		{
			cout << bl::format("Toolbar found: {1}") % toolbar << endl;

			int id = e.insertItem(item, position);

			cout << bl::format("return {1}") % id << endl;
			return id;
		}
	}

	ToolbarEntry newEntry;
	newEntry.setName(toolbar);
	int id = newEntry.addItem(item);
	this->contents.push_back(newEntry);

	return id;
}

bool ToolbarData::removeItemByID(string toolbar, int id)
{
	XOJ_CHECK_TYPE(ToolbarData);

	g_return_val_if_fail(isPredefined() == false, false);

	for (ToolbarEntry& e : this->contents)
	{
		if (e.getName() == toolbar)
		{
			return e.removeItemById(id);
		}
	}

	return false;
}

