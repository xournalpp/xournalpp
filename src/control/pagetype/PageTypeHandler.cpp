#include "PageTypeHandler.h"

#include "gui/GladeSearchpath.h"

#include <i18n.h>
#include <Util.h>


PageTypeHandler::PageTypeHandler(GladeSearchpath* gladeSearchPath)
{
	XOJ_INIT_TYPE(PageTypeHandler);

	string file = gladeSearchPath->findFile("", "pagetemplates.ini");

	if (!parseIni(file) || this->types.size() < 5)
	{

		string msg = FS(_F("Could not load pagetemplates.ini file"));
		Util::showErrorToUser(NULL, msg);

		// On failure load the hardcoded and predefined values
		addPageTypeInfo(_("Plain"), "plain", "");
		addPageTypeInfo(_("Lined"), "lined", "");
		addPageTypeInfo(_("Ruled"), "ruled", "");
		addPageTypeInfo(_("Graph"), "graph", "");
		addPageTypeInfo(_("Dotted"), "dotted", "");
	}

	// Special types
	addPageTypeInfo(_("Copy current"), ":copy", "");
	addPageTypeInfo(_("With PDF background"), ":pdf", "");
	addPageTypeInfo(_("Image"), ":image", "");
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

bool PageTypeHandler::parseIni(string filename)
{
	XOJ_CHECK_TYPE(PageTypeHandler);

	GKeyFile* config = g_key_file_new();
	g_key_file_set_list_separator(config, ',');
	if (!g_key_file_load_from_file(config, filename.c_str(), G_KEY_FILE_NONE, NULL))
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
	gchar* name = g_key_file_get_locale_string(config, group, "name", NULL, NULL);
	if (name != NULL)
	{
		strName = name;
		g_free(name);
	}

	string strFormat;
	gchar* format = g_key_file_get_string(config, group, "format", NULL);
	if (format != NULL)
	{
		strFormat = format;
		g_free(format);
	}

	string strConfig;
	gchar* cconfig = g_key_file_get_string(config, group, "config", NULL);
	if (cconfig != NULL)
	{
		strConfig = cconfig;
		g_free(cconfig);
	}

	addPageTypeInfo(strName, strFormat, strConfig);
}

void PageTypeHandler::addPageTypeInfo(string name, string format, string config)
{
	PageTypeInfo* pt = new PageTypeInfo();
	pt->name = name;
	pt->page.format = format;
	pt->page.config = config;

	this->types.push_back(pt);
}

vector<PageTypeInfo*>& PageTypeHandler::getPageTypes()
{
	XOJ_CHECK_TYPE(PageTypeHandler);

	return this->types;
}
