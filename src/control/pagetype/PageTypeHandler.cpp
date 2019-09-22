#include <utility>

#include "PageTypeHandler.h"

#include "gui/GladeSearchpath.h"

#include <i18n.h>
#include <XojMsgBox.h>

PageTypeHandler::PageTypeHandler(GladeSearchpath* gladeSearchPath)
{
	string file = gladeSearchPath->findFile("", "pagetemplates.ini");

	if (!parseIni(file) || this->types.size() < 5)
	{

		string msg = FS(_F("Could not load pagetemplates.ini file"));
		XojMsgBox::showErrorToUser(nullptr, msg);

		// On failure load the hardcoded and predefined values
		addPageTypeInfo(_("Plain"), PageTypeFormat::Plain, "");
		addPageTypeInfo(_("Ruled"), PageTypeFormat::Ruled, "");
		addPageTypeInfo(_("Ruled with vertical line"), PageTypeFormat::Lined, "");
		addPageTypeInfo(_("Staves"), PageTypeFormat::Staves, "");
		addPageTypeInfo(_("Graph"), PageTypeFormat::Graph, "");
		addPageTypeInfo(_("Dotted"), PageTypeFormat::Dotted, "");
	}

	// Special types
	addPageTypeInfo(_("Copy current"), PageTypeFormat::Copy, "");
	addPageTypeInfo(_("With PDF background"), PageTypeFormat::Pdf, "");
	addPageTypeInfo(_("Image"), PageTypeFormat::Image, "");
}

PageTypeHandler::~PageTypeHandler()
{
	for (PageTypeInfo* t : types)
	{
		delete t;
	}
	types.clear();
}

bool PageTypeHandler::parseIni(const string& filename)
{
	GKeyFile* config = g_key_file_new();
	g_key_file_set_list_separator(config, ',');
	if (!g_key_file_load_from_file(config, filename.c_str(), G_KEY_FILE_NONE, nullptr))
	{
		g_key_file_free(config);
		return false;
	}

	gsize lenght = 0;
	gchar** groups = g_key_file_get_groups(config, &lenght);

	for (gsize i = 0; i < lenght; i++)
	{
		loadFormat(config, groups[i]);
	}

	g_strfreev(groups);
	g_key_file_free(config);
	return true;
}

void PageTypeHandler::loadFormat(GKeyFile* config, const char* group)
{
	string strName;
	gchar* name = g_key_file_get_locale_string(config, group, "name", nullptr, nullptr);
	if (name != nullptr)
	{
		strName = name;
		g_free(name);
	}

	string strFormat;
	gchar* format = g_key_file_get_string(config, group, "format", nullptr);
	if (format != nullptr)
	{
		strFormat = format;
		g_free(format);
	}

	string strConfig;
	gchar* cconfig = g_key_file_get_string(config, group, "config", nullptr);
	if (cconfig != nullptr)
	{
		strConfig = cconfig;
		g_free(cconfig);
	}

	addPageTypeInfo(strName, getPageTypeFormatForString(strFormat), strConfig);
}

void PageTypeHandler::addPageTypeInfo(string name, PageTypeFormat format, string config)
{
	auto pt = new PageTypeInfo();
	pt->name = std::move(name);
	pt->page.format = format;
	pt->page.config = std::move(config);

	this->types.push_back(pt);
}

vector<PageTypeInfo*>& PageTypeHandler::getPageTypes()
{
	return this->types;
}

PageTypeFormat PageTypeHandler::getPageTypeFormatForString(const string& format)
{
	if (format == "plain")
	{
		return PageTypeFormat::Plain;
	}
	else if (format == "ruled")
	{
		return PageTypeFormat::Ruled;
	}
	else if (format == "lined")
	{
		return PageTypeFormat::Lined;
	}
	else if (format == "staves")
	{
		return PageTypeFormat::Staves;
	}
	else if (format == "graph")
	{
		return PageTypeFormat::Graph;
	}
	else if (format == "dotted")
	{
		return PageTypeFormat::Dotted;
	}
	else if (format == ":pdf")
	{
		return PageTypeFormat::Pdf;
	}
	else if (format == ":image")
	{
		return PageTypeFormat::Image;
	}
	else if (format == ":copy")
	{
		return PageTypeFormat::Copy;
	}

	return PageTypeFormat::Ruled;
}

string PageTypeHandler::getStringForPageTypeFormat(const PageTypeFormat& format)
{
	switch (format)
	{
	case PageTypeFormat::Plain:
		return "plain";
	case PageTypeFormat::Ruled:
		return "ruled";
	case PageTypeFormat::Lined:
		return "lined";
	case PageTypeFormat::Staves:
		return "staves";
	case PageTypeFormat::Graph:
		return "graph";
	case PageTypeFormat::Dotted:
		return "dotted";
	case PageTypeFormat::Pdf:
		return ":pdf";
	case PageTypeFormat::Image:
		return ":image";
	case PageTypeFormat::Copy:
		return ":copy";
	}
	return "lined_vline";
}