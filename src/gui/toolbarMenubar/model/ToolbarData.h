/*
 * Xournal++
 *
 * Toolbar definitions model
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARDATA_H__
#define __TOOLBARDATA_H__

#include <glib.h>
#include <String.h>
#include <XournalType.h>
#include <vector>

#include "ToolbarEntry.h"

class ToolbarData {
public:
	ToolbarData(bool predefined);
	ToolbarData(const ToolbarData & data);
	virtual ~ToolbarData();

public:
	String getName();
	void setName(String name);
	String getId();
	bool isPredefined();

	void load(GKeyFile * config, const char * group);
	void saveToKeyFile(GKeyFile * config);

	// Editing API
	void addItem(String toolbar, String item, int position);
	bool removeItem(String toolbar, int position);

private:
	XOJ_TYPE_ATTRIB;

	String id;
	String name;
	std::vector<ToolbarEntry> contents;

	bool predefined;

	friend class ToolbarModel;
	friend class ToolMenuHandler;
};

#endif /* __TOOLBARDATA_H__ */
