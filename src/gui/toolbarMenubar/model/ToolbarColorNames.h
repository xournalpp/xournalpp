/*
 * Xournal++
 *
 * Names for the toolbar color items (e.g. 0xff000 is called red)
 * Singleton
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARCOLORNAMES_H__
#define __TOOLBARCOLORNAMES_H__

#include <XournalType.h>
#include <glib.h>
#include <String.h>

class ToolbarColorNames {
private:
	ToolbarColorNames();
	virtual ~ToolbarColorNames();

public:
	static ToolbarColorNames & getInstance();
	static void freeInstance();

public:
	void loadFile(const char * file);
	void saveFile(const char * file);

	void adddColor(int color, String name, bool predefined);

	String getColorName(int color);

private:
	void initPredefinedColors();

private:
	XOJ_TYPE_ATTRIB;

	GKeyFile * config;
	GHashTable * predefinedColorNames;
};

#endif /* __TOOLBARCOLORNAMES_H__ */
