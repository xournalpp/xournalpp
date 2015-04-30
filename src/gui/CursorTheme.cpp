#include "CursorTheme.h"

#include <glib.h>

CursorTheme::CursorTheme()
{
	XOJ_INIT_TYPE(CursorTheme);
}

CursorTheme::~CursorTheme()
{
	XOJ_RELEASE_TYPE(CursorTheme);
}

bool CursorTheme::loadTheme(String name)
{
	XOJ_CHECK_TYPE(CursorTheme);

	GKeyFile* config = g_key_file_new();
	g_key_file_set_list_separator(config, ',');
	if (!g_key_file_load_from_file(config, name.c_str(), G_KEY_FILE_NONE, NULL))
	{
		g_key_file_free(config);
		return false;
	}

	gchar* author = g_key_file_get_string(config, "general", "author", NULL);
	if (author != NULL)
	{
		this->author = author;
		g_free(author);
	}
	else
	{
		this->author = "Unknown";
	}

	gchar* themeName = g_key_file_get_locale_string(config, "general", "name", NULL, NULL);
	if (themeName != NULL)
	{
		this->name = themeName;
		g_free(themeName);
	}
	else
	{
		this->name = name;
	}

	gchar* description = g_key_file_get_locale_string(config, "general", "description", NULL, NULL);
	if (description != NULL)
	{
		this->description = description;
		g_free(description);
	}
	else
	{
		this->description = "Keine";
	}

	g_key_file_free(config);
	return true;
}
