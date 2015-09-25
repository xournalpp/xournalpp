#include "ToolbarColorNames.h"

#include <config.h>
#include <i18n.h>
#include <StringUtils.h>

#include <stdio.h>
#include <fstream>
#include <iostream>
using std::cout;
using std::endl;

ToolbarColorNames::ToolbarColorNames()
{
	XOJ_INIT_TYPE(ToolbarColorNames);
	this->predefinedColorNames = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, g_free);
	this->config = g_key_file_new();
	g_key_file_set_string(this->config, "info", "about", "Xournalpp custom color names");
	initPredefinedColors();
}

ToolbarColorNames::~ToolbarColorNames()
{
	XOJ_CHECK_TYPE(ToolbarColorNames);

	g_hash_table_destroy(this->predefinedColorNames);
	this->predefinedColorNames = NULL;
	g_key_file_free(this->config);

	XOJ_RELEASE_TYPE(ToolbarColorNames);
}

static ToolbarColorNames* instance = NULL;

ToolbarColorNames& ToolbarColorNames::getInstance()
{
	if (instance == NULL)
	{
		instance = new ToolbarColorNames();
	}

	return *instance;
}

void ToolbarColorNames::freeInstance()
{
	delete instance;
	instance = NULL;
}

void ToolbarColorNames::loadFile(const string file)
{
	XOJ_CHECK_TYPE(ToolbarColorNames);

	GError* error = NULL;
	if (!g_key_file_load_from_file(config, file.c_str(), G_KEY_FILE_NONE, &error))
	{
		g_warning("Failed to load \"colornames.ini\" (%s): %s\n", file.c_str(), error->message);
		g_error_free(error);
		return;
	}

	g_key_file_set_string(this->config, "info", "about", "Xournalpp custom color names");
}

void ToolbarColorNames::saveFile(const string file)
{
	XOJ_CHECK_TYPE(ToolbarColorNames);

	gsize len = 0;
	char* data = g_key_file_to_data(this->config, &len, NULL);

	std::ofstream f(file, std::fstream::out);
	f << data;
	f.close();
	
	g_free(data);
}

void ToolbarColorNames::addColor(int color, string name, bool predefined)
{
	XOJ_CHECK_TYPE(ToolbarColorNames);

	int* key = g_new(int, 1);
	*key = color;

	if (predefined)
	{
		g_hash_table_insert(this->predefinedColorNames, key, g_strdup(name.c_str()));
	}
	else
	{
		const gchar* colorHex = StringUtils::format("%06x", color).c_str();
		g_key_file_set_string(this->config, "custom", colorHex, name.c_str());
	}
}

string ToolbarColorNames::getColorName(int color)
{
	XOJ_CHECK_TYPE(ToolbarColorNames);

	string colorName;

	string colorHex = StringUtils::format("%06x", color);

	char* name = g_key_file_get_string(this->config, "custom", colorHex.c_str(), NULL);
	if (name != NULL)
	{
		colorName = name;
	}
	g_free(name);

	if (!colorName.empty())
	{
		return colorName;
	}

	char* value = (char*) g_hash_table_lookup(this->predefinedColorNames, &color);
	if (value)
	{
		return value;
	}

	return colorHex;
}

void ToolbarColorNames::initPredefinedColors()
{
	XOJ_CHECK_TYPE(ToolbarColorNames);

	// Here you can add predefined color names
	// this ordering fixes #2
	addColor(0x000000, _("Black"),		 true);
	addColor(0x008000, _("Green"),		 true);
	addColor(0x00c0ff, _("Light Blue"),  true);
	addColor(0x00ff00, _("Light Green"), true);
	addColor(0x3333cc, _("Blue"),		 true);
	addColor(0x808080, _("Gray"),		 true);
	addColor(0xff0000, _("Red"),		 true);
	addColor(0xff00ff, _("Mangenta"),	 true);
	addColor(0xff8000, _("Orange"),		 true);
	addColor(0xffff00, _("Yellow"),		 true);
	addColor(0xffffff, _("White"),		 true);
}
