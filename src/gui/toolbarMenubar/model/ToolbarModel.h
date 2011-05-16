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

#ifndef __TOOLBARMODEL_H__
#define __TOOLBARMODEL_H__

#include <glib.h>
#include <ListIterator.h>
#include <XournalType.h>

class ToolbarData;

class ToolbarModel {
public:
	ToolbarModel();
	virtual ~ToolbarModel();

public:
	ListIterator<ToolbarData *> iterator();
	bool parse(const char * file, bool predefined);
	const char * getColorName(const char * color);
	void add(ToolbarData * data);
	void remove(ToolbarData * data);

private:
	void parseGroup(GKeyFile * config, const char * group, bool predefined);
	void parseColors(GKeyFile * config, const char * group);

private:
	XOJ_TYPE_ATTRIB;

	GList * toolbars;
	GHashTable * colorNameTable;
};

#endif /* __TOOLBARMODEL_H__ */
