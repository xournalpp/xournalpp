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

#include "ToolbarEntry.h"

#include <XournalType.h>

class ToolbarData
{
public:
	ToolbarData(bool predefined);
	ToolbarData(const ToolbarData& data);
	virtual ~ToolbarData();

	void operator=(const ToolbarData& other);

public:
	string getName();
	void setName(string name);
	string getId();
	void setId(string id);
	bool isPredefined();

	void load(GKeyFile* config, const char* group);
	void saveToKeyFile(GKeyFile* config);

	// Editing API
	int insertItem(string toolbar, string item, int position);
	bool removeItemByID(string toolbar, int id);

private:
	string id;
	string name;
	std::vector<ToolbarEntry*> contents;

	bool predefined = false;

	friend class ToolbarModel;
	friend class ToolMenuHandler;
};
