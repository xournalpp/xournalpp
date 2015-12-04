#include "ToolbarModel.h"

#include "ToolbarData.h"

#include <fstream>

ToolbarModel::ToolbarModel()
{
	XOJ_INIT_TYPE(ToolbarModel);
}

ToolbarModel::~ToolbarModel()
{
	XOJ_CHECK_TYPE(ToolbarModel);

	for (ToolbarData* data : this->toolbars)
	{
		delete data;
	}
	this->toolbars.clear();

	XOJ_RELEASE_TYPE(ToolbarModel);
}

ToolbarDataVector* ToolbarModel::getToolbars()
{
	XOJ_CHECK_TYPE(ToolbarModel);

	return &this->toolbars;
}

void ToolbarModel::parseGroup(GKeyFile* config, const char* group, bool predefined)
{
	XOJ_CHECK_TYPE(ToolbarModel);

	ToolbarData* data = new ToolbarData(predefined);

	data->name = (predefined ? "predef_" : "custom_");
	data->id = group;

	data->load(config, group);

	add(data);
}

void ToolbarModel::remove(ToolbarData* data)
{
	XOJ_CHECK_TYPE(ToolbarModel);

	for (size_t i = 0; i < this->toolbars.size(); i++)
	{
		if (this->toolbars[i] == data)
		{
			this->toolbars.erase(this->toolbars.begin() + i);
			break;
		}
	}
}

void ToolbarModel::add(ToolbarData* data)
{
	XOJ_CHECK_TYPE(ToolbarModel);

	this->toolbars.push_back(data);
}

bool ToolbarModel::parse(string filename, bool predefined)
{
	XOJ_CHECK_TYPE(ToolbarModel);

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
		parseGroup(config, groups[i], predefined);
	}

	g_strfreev(groups);
	g_key_file_free(config);
	return true;
}

bool ToolbarModel::existsId(string id)
{
	XOJ_CHECK_TYPE(ToolbarModel);

	for (ToolbarData* data : this->toolbars)
	{
		if (data->getId() == id) return true;
	}
	return false;
}


const char* TOOLBAR_INI_HEADER =
		"# Xournal++ Toolbar configuration\n"
		"# Here you can customize the Toolbars\n"
		" Delte this file to generate a new config file with default values\n"
		"\n"
		" Available buttons:\n"
		" File: NEW,SAVE,OPEN\n"
		"\n"
		" Edit: CUT,COPY,PASTE,SEARCH,UNDO,REDO,INSERT_NEW_PAGE\n"
		"\n"
		" Navigation: GOTO_FIRST,GOTO_BACK,GOTO_NEXT,GOTO_LAST,GOTO_NEXT_ANNOTATED_PAGE\n"
		"\n"
		" Zoom: ZOOM_OUT,ZOOM_IN,ZOOM_FIT,ZOOM_100,FULLSCREEN,TWO_PAGES\n"
		"\n"
		" Color: COLOR(0xffffff),COLOR(0xffff00),COLOR(0xff8000),COLOR(0xff00ff),COLOR(0x00ff00),COLOR(0x00c0ff),COLOR(0x808080),COLOR(0x008000),COLOR(0xff0000),COLOR(0x3333cc),COLOR(0x000000),COLOR_SELECT\n"
		"  Notice: This are the default Xournal colors, each other color in HEX can also be used, eg COLOR(0x12ABCF);\n"
		"\n"
		" Tools: ERASER,PEN,HILIGHTER,IMAGE,TEXT,IMAGE,SELECT,SELECT_REGION,SELECT_RECTANGLE,VERTICAL_SPACE,HAND\n"
		"  Notice: ERASER also has a drop down menu to select the eraser type, SELECT are all selection tools, with drop down menu\n"
		"\n"
		" Tool settings: SHAPE_RECOGNIZER,RULER,FINE,MEDIUM,THICK,SELECT_FONT\n"
		"\n"
		" Components: PAGE_SPIN,ZOOM_SLIDER,LAYER\n"
		"  PAGE_SPIN: The page spiner, incl. current page label\n"
		"  ZOOM_SLIDER: The zoom slider\n"
		"  LAYER: The layer dropdown menu\n"
		"\n";

void ToolbarModel::save(string filename)
{
	GKeyFile* config = g_key_file_new();
	g_key_file_set_list_separator(config, ',');

	g_key_file_set_comment(config, NULL, NULL, TOOLBAR_INI_HEADER, NULL);

	for (ToolbarData* data : this->toolbars)
	{
		if (!data->isPredefined())
		{
			data->saveToKeyFile(config);
		}
	}

	gsize len = 0;
	char* data = g_key_file_to_data(config, &len, NULL);

	std::ofstream f(filename);
	f.write(data, len);
	f.close();
	
	g_free(data);
}
