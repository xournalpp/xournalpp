#include <utility>

#include "PageTypeHandler.h"

#include "gui/GladeSearchpath.h"

#include <i18n.h>
#include <XojMsgBox.h>


PageTypeHandler::PageTypeHandler(GladeSearchpath* gladeSearchPath)
{
	XOJ_INIT_TYPE(PageTypeHandler);

	string file = gladeSearchPath->findFile("", "pagetemplates.ini");

	if (!parseIni(file) || this->types.size() < 5)
	{

		string msg = FS(_F("Could not load pagetemplates.ini file"));
		XojMsgBox::showErrorToUser(nullptr, msg);

		// On failure load the hardcoded and predefined values
		addPageTypeInfo(_("Plain"), PageTypeFormat::PLAIN, "");
		addPageTypeInfo(_("Lined"), PageTypeFormat::LINED, "");
		addPageTypeInfo(_("Lined with vertical line"), PageTypeFormat::LINED_VLINE, "");
		addPageTypeInfo(_("Staves"), PageTypeFormat::STAVES, "");
		addPageTypeInfo(_("Staves with vertical line"), PageTypeFormat::STAVES_VLINE, "");
		addPageTypeInfo(_("Graph"), PageTypeFormat::GRAPH, "");
		addPageTypeInfo(_("Dotted"), PageTypeFormat::DOTTED, "");
	}

	// Special types
	addPageTypeInfo(_("Copy current"), PageTypeFormat::COPY, "");
	addPageTypeInfo(_("With PDF background"), PageTypeFormat::PDF, "");
	addPageTypeInfo(_("Image"), PageTypeFormat::IMAGE, "");
}

PageTypeHandler::~PageTypeHandler()
{
	XOJ_CHECK_TYPE(PageTypeHandler);

	for (PageTypeInfo* t : types)
	{
		delete t;
	}
	types.clear();

	XOJ_RELEASE_TYPE(PageTypeHandler);
}

bool PageTypeHandler::parseIni(const string& filename)
{
	XOJ_CHECK_TYPE(PageTypeHandler);

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
	XOJ_CHECK_TYPE(PageTypeHandler);

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
	XOJ_CHECK_TYPE(PageTypeHandler);

	return this->types;
}

PageTypeFormat PageTypeHandler::getPageTypeFormatForString(const string& format)
{
	if (format == "plain")
	{
		return PageTypeFormat::PLAIN;
	}
	else if (format == "lined")
	{
		return PageTypeFormat::LINED;
	}
	else if (format == "lined_vline")
	{
		return PageTypeFormat::LINED_VLINE;
	}
	else if (format == "staves")
	{
		return PageTypeFormat::STAVES;
	}
	else if (format == "staves_vline")
	{
		return PageTypeFormat::STAVES_VLINE;
	}
	else if (format == "graph")
	{
		return PageTypeFormat::GRAPH;
	}
	else if (format == "dotted")
	{
		return PageTypeFormat::DOTTED;
	}
	else if (format == ":pdf")
	{
		return PageTypeFormat::PDF;
	}
	else if (format == ":image")
	{
		return PageTypeFormat::IMAGE;
	}
	else if (format == ":copy")
	{
		return PageTypeFormat::COPY;
	}
		// Add for compatibility sake
	else if (format == "ruled")
	{
		return PageTypeFormat::LINED_VLINE;
	}

	return PageTypeFormat::LINED_VLINE;
}

string PageTypeHandler::getStringForPageTypeFormat(const PageTypeFormat& format)
{
	switch (format)
	{
	case PageTypeFormat::PLAIN:
		return "plain";
	case PageTypeFormat::LINED:
		return "lined";
	case PageTypeFormat::LINED_VLINE:
		return "lined_vline";
	case PageTypeFormat::STAVES:
		return "staves";
	case PageTypeFormat::STAVES_VLINE:
		return "staves_vline";
	case PageTypeFormat::GRAPH:
		return "graph";
	case PageTypeFormat::DOTTED:
		return "dotted";
	case PageTypeFormat::PDF:
		return ":pdf";
	case PageTypeFormat::IMAGE:
		return ":image";
	case PageTypeFormat::COPY:
		return ":copy";
	}
	return "lined_vline";
}