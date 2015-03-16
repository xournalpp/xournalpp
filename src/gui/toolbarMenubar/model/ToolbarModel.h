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
#include <StringUtils.h>

class ToolbarData;

class ToolbarModel
{
public:
	ToolbarModel();
	virtual ~ToolbarModel();

public:
	ListIterator<ToolbarData*> iterator();
	bool parse(const char* file, bool predefined);
	void add(ToolbarData* data);
	void remove(ToolbarData* data);
	void save(const char* filename);
	bool existsId(String id);

private:
	void parseGroup(GKeyFile* config, const char* group, bool predefined);

private:
	XOJ_TYPE_ATTRIB;

	GList* toolbars;
};

#endif /* __TOOLBARMODEL_H__ */
