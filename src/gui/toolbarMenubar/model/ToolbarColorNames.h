/*
 * Xournal++
 *
 * Names for the toolbar color items (e.g. 0xff000 is called red)
 * Singleton
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <XournalType.h>
#include <glib.h>
#include <StringUtils.h>

class ToolbarColorNames
{
private:
	ToolbarColorNames();
	virtual ~ToolbarColorNames();

public:
	static ToolbarColorNames& getInstance();
	static void freeInstance();

public:
	void loadFile(const char* file);
	void saveFile(const char* file);

	void adddColor(int color, string name, bool predefined);

	string getColorName(int color);

private:
	void initPredefinedColors();

private:
	XOJ_TYPE_ATTRIB;

	GKeyFile* config;
	GHashTable* predefinedColorNames;
};
