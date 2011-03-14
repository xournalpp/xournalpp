/*
 * Xournal++
 *
 * Toolbar edit dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARDIALOG_H__
#define __TOOLBARDIALOG_H__

#include "../GladeGui.h"

class ToolbarModel;
class ToolbarData;

class ToolbarDialog: public GladeGui {
public:
	ToolbarDialog(GladeSearchpath * gladeSearchPath, ToolbarModel * model);
	virtual ~ToolbarDialog();

public:
	void show();

private:
	static void treeSelectionChangedCallback(GtkTreeSelection * selection, ToolbarDialog * dlg);
	static void treeCellEditedCallback(GtkCellRendererText * renderer, gchar * pathString, gchar * newText, ToolbarDialog * dlg);

	static void buttonNewCallback(GtkButton * button, ToolbarDialog * dlg);
	static void buttonDeleteCallback(GtkButton * button, ToolbarDialog * dlg);
	static void buttonCopyCallback(GtkButton * button, ToolbarDialog * dlg);

	void addToolbarData(ToolbarData * data);
	void entrySelected(ToolbarData * data);

private:
	ToolbarModel * tbModel;
	GtkListStore * model;

	ToolbarData * selected;
};

#endif /* __TOOLBARDIALOG_H__ */
