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
#include "../../../util/String.h"
#include "../../../util/XournalType.h"
#include <vector>

#include "ToolbarEntry.h"

class ToolbarData {
public:
	ToolbarData(bool predefined);
	~ToolbarData();

public:
	String getName();
	void setName(String name);
	String getId();

	void load(GKeyFile * config, const char * group);

	bool isPredefined();
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
