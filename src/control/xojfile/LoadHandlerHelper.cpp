#include "LoadHandlerHelper.h"

#include "LoadHandler.h"

#include <config.h>
#include <i18n.h>

#define error(...) if (loadHandler->error == nullptr) { loadHandler->error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__); }

typedef struct
{
	const char* name;
	const int rgb;
} PredefinedColor;

static PredefinedColor PREDEFINED_COLORS[] =
{
	{ "black",      0x000000 },
	{ "blue",       0x3333cc },
	{ "red",        0xff0000 },
	{ "green",      0x008000 },
	{ "gray",       0x808080 },
	{ "lightblue",  0x00c0ff },
	{ "lightgreen", 0x00ff00 },
	{ "magenta",    0xff00ff },
	{ "orange",     0xff8000 },
	{ "yellow",     0xffff00 },
	{ "white",      0xffffff }
};

const int COLOR_COUNT = sizeof(PREDEFINED_COLORS) / sizeof(PredefinedColor);

int LoadHandlerHelper::parseBackgroundColor(LoadHandler* loadHandler)
{
	const char* sColor = LoadHandlerHelper::getAttrib("color", false, loadHandler);

	int color = 0xffffff;
	if (strcmp("blue", sColor) == 0)
	{
		color = 0xa0e8ff;
	}
	else if (strcmp("pink", sColor) == 0)
	{
		color = 0xffc0d4;
	}
	else if (strcmp("green", sColor) == 0)
	{
		color = 0x80FFC0;
	}
	else if (strcmp("orange", sColor) == 0)
	{
		color = 0xFFC080;
	}
	else if (strcmp("yellow", sColor) == 0)
	{
		color = 0xFFFF80;
	}
	else
	{
		LoadHandlerHelper::parseColor(sColor, color, loadHandler);
	}

	return color;
}

bool LoadHandlerHelper::parseColor(const char* text, int& color, LoadHandler* loadHandler)
{
	if (text == nullptr)
	{
		error("%s", _("Attribute color not set!"));
		return false;
	}

	if (text[0] == '#')
	{
		gchar* ptr = nullptr;
		int c = g_ascii_strtoull(&text[1], &ptr, 16);
		if (ptr != text + strlen(text))
		{
			error("%s", FC(_F("Unknown color value \"{1}\"") % text));
			return false;
		}

		color = c >> 8;

		return true;
	}
	else
	{
		for (int i = 0; i < COLOR_COUNT; i++)
		{
			if (!strcmp(text, PREDEFINED_COLORS[i].name))
			{
				color = PREDEFINED_COLORS[i].rgb;
				return true;
			}
		}
		error("%s", FC(_F("Color \"{1}\" unknown (not defined in default color list)!") % text));
		return false;
	}
}


const char* LoadHandlerHelper::getAttrib(const char* name, bool optional, LoadHandler* loadHandler)
{
	const char** aName = loadHandler->attributeNames;
	const char** aValue = loadHandler->attributeValues;

	while (*aName != nullptr)
	{
		if (!strcmp(*aName, name))
		{
			return *aValue;
		}
		aName++;
		aValue++;
	}

	if (!optional)
	{
		g_warning("Parser: attribute %s not found!", name);
	}
	return nullptr;
}

double LoadHandlerHelper::getAttribDouble(const char* name, LoadHandler* loadHandler)
{
	const char* attrib = getAttrib(name, false, loadHandler);

	if (attrib == nullptr)
	{
		error("%s", FC(_F("Attribute \"{1}\" could not be parsed as double, the value is nullptr") % name));
		return 0;
	}

	char* ptr = nullptr;
	double val = g_ascii_strtod(attrib, &ptr);
	if (ptr == attrib)
	{
		error("%s", FC(_F("Attribute \"{1}\" could not be parsed as double, the value is \"{2}\"") % name % attrib));
	}

	return val;
}

int LoadHandlerHelper::getAttribInt(const char* name, LoadHandler* loadHandler)
{
	const char* attrib = getAttrib(name, false, loadHandler);

	if (attrib == nullptr)
	{
		error("%s", FC(_F("Attribute \"{1}\" could not be parsed as int, the value is nullptr") % name));
		return 0;
	}

	char* ptr = nullptr;
	int val = strtol(attrib, &ptr, 10);
	if (ptr == attrib)
	{
		error("%s", FC(_F("Attribute \"{1}\" could not be parsed as int, the value is \"{2}\"") % name % attrib));
	}

	return val;
}

bool LoadHandlerHelper::getAttribInt(const char* name, bool optional, LoadHandler* loadHandler, int& rValue)
{
	const char* attrib = getAttrib(name, optional, loadHandler);

	if (attrib == nullptr)
	{
		if (!optional)
		{
			g_warning("Parser: attribute %s not found!", name);
		}
		return false;
	}

	char* ptr = nullptr;
	int val = strtol(attrib, &ptr, 10);
	if (ptr == attrib)
	{
		error("%s", FC(_F("Attribute \"{1}\" could not be parsed as int, the value is \"{2}\"") % name % attrib));
	}

	rValue = val;

	return true;
}

size_t LoadHandlerHelper::getAttribSizeT(const char* name, LoadHandler* loadHandler)
{
	const char* attrib = getAttrib(name, false, loadHandler);

	if (attrib == nullptr)
	{
		error("%s", FC(_F("Attribute \"{1}\" could not be parsed as size_t, the value is nullptr") % name));
		return 0;
	}

	char* ptr = nullptr;
	size_t val = g_ascii_strtoull(attrib, &ptr, 10);
	if (ptr == attrib)
	{
		error("%s", FC(_F("Attribute \"{1}\" could not be parsed as size_t, the value is \"{2}\"") % name % attrib));
	}

	return val;
}

bool LoadHandlerHelper::getAttribSizeT(const char* name, bool optional, LoadHandler* loadHandler, size_t& rValue)
{
	const char* attrib = getAttrib(name, optional, loadHandler);

	if (attrib == nullptr)
	{
		if (!optional)
		{
			g_warning("Parser: attribute %s not found!", name);
		}
		return false;
	}

	char* ptr = nullptr;
	size_t val = strtoull(attrib, &ptr, 10);
	if (ptr == attrib)
	{
		error("%s", FC(_F("Attribute \"{1}\" could not be parsed as size_t, the value is \"{2}\"") % name % attrib));
	}

	rValue = val;

	return true;
}