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

#include <XournalType.h>
#include <StringUtils.h>

class ToolbarData;
typedef std::vector<ToolbarData*> ToolbarDataVector;

class ToolbarModel
{
public:
	ToolbarModel();
	virtual ~ToolbarModel();

public:
	ToolbarDataVector* getToolbars();
	bool parse(const char* file, bool predefined);
	void add(ToolbarData* data);
	void remove(ToolbarData* data);
	void save(const char* filename);
	bool existsId(string id);

private:
	void parseGroup(GKeyFile* config, const char* group, bool predefined);

private:
	XOJ_TYPE_ATTRIB;

	ToolbarDataVector toolbars;
};
