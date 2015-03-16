#include "ToolbarData.h"
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>

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

String ToolbarData::getName()
{
	XOJ_CHECK_TYPE(ToolbarData);

	return this->name;
}

void ToolbarData::setName(String name)
{
	XOJ_CHECK_TYPE(ToolbarData);

	this->name = name;
}

String ToolbarData::getId()
{
	XOJ_CHECK_TYPE(ToolbarData);

	return this->id;
}

void ToolbarData::setId(String id)
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
			String s = list[x];
			e.addItem(s.trim());
		}

		this->contents.push_back(e);

		g_strfreev(list);
	}

	g_strfreev(keys);
}

void ToolbarData::saveToKeyFile(GKeyFile* config)
{
	XOJ_CHECK_TYPE(ToolbarData);

	const char* group = CSTR(getId());

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++)
	{
		ToolbarEntry& e = *it;

		String line = "";

		ListIterator<ToolbarItem*> it = e.iterator();
		while (it.hasNext())
		{
			line += ",";
			line += *it.next();
		}

		if (line.length() > 2)
		{
			g_key_file_set_string(config, group, CSTR(e.getName()),
			                      CSTR(String(line).retainBetween(1)));
		}
	}

	g_key_file_set_string(config, group, "name", CSTR(this->name));
}

int ToolbarData::insertItem(String toolbar, String item, int position)
{
	XOJ_CHECK_TYPE(ToolbarData);

	printf("ToolbarData::insertItem(%s, %s, %i);\n", CSTR(toolbar), CSTR(item),
	       position);

	g_return_val_if_fail(isPredefined() == false, -1);

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++)
	{
		ToolbarEntry& e = *it;

		if (e.getName() == toolbar)
		{
			printf("Toolbar found: %s\n", CSTR(toolbar));

			int id = e.insertItem(item, position);

			printf("return %i\n", id);
			return id;
		}
	}

	ToolbarEntry newEntry;
	newEntry.setName(toolbar);
	int id = newEntry.addItem(item);
	this->contents.push_back(newEntry);

	return id;
}

bool ToolbarData::removeItemByID(String toolbar, int id)
{
	XOJ_CHECK_TYPE(ToolbarData);

	g_return_val_if_fail(isPredefined() == false, false);

	std::vector<ToolbarEntry>::iterator it;
	for (it = this->contents.begin(); it != this->contents.end(); it++)
	{
		ToolbarEntry& e = *it;

		if (e.getName() == toolbar)
		{
			return e.removeItemById(id);
		}
	}

	return false;
}

