/*
 * Xournal++
 *
 * Names for the toolbar color items (e.g. 0xff000 is called red)
 * Singleton
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class ToolbarColorNames
{
private:
	ToolbarColorNames();
	virtual ~ToolbarColorNames();

public:
	static ToolbarColorNames& getInstance();
	static void freeInstance();

public:
	void loadFile(const string file);
	void saveFile(const string file);

	void addColor(int color, string name, bool predefined);

	string getColorName(int color);

private:
	void initPredefinedColors();

private:
	GKeyFile* config;
	GHashTable* predefinedColorNames;
};
