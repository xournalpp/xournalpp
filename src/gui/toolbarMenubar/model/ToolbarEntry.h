/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARENTRY_H__
#define __TOOLBARENTRY_H__
// TODO: AA: type check

class ToolbarEntry {
private:
	String name;
	std::vector<String> entries;

	friend class ToolMenuHandler;
	friend class ToolbarData;
};

#endif /* __TOOLBARENTRY_H__ */
