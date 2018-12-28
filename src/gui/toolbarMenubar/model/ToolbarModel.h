/*
 * Xournal++
 *
 * Toolbar definitions model
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <Path.h>
#include <XournalType.h>

class ToolbarData;

class ToolbarModel
{
public:
	ToolbarModel();
	virtual ~ToolbarModel();

public:
	vector<ToolbarData*>* getToolbars();
	bool parse(string filename, bool predefined);
	void add(ToolbarData* data);
	void remove(ToolbarData* data);
	void save(Path filename);
	bool existsId(string id);

private:
	void parseGroup(GKeyFile* config, const char* group, bool predefined);

private:
	XOJ_TYPE_ATTRIB;

	vector<ToolbarData*> toolbars;
};
