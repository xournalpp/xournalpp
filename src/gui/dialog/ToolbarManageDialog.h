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

#ifndef __TOOLBARMANAGEDIALOG_H__
#define __TOOLBARMANAGEDIALOG_H__

#include "../GladeGui.h"
#include <XournalType.h>

class ToolbarModel;
class ToolbarData;

class ToolbarManageDialog : public GladeGui
{
public:
	ToolbarManageDialog(GladeSearchpath* gladeSearchPath, ToolbarModel* model);
	virtual ~ToolbarManageDialog();

public:
	virtual void show(GtkWindow* parent);

private:
	static void treeSelectionChangedCallback(GtkTreeSelection* selection,
											ToolbarManageDialog* dlg);
	static void treeCellEditedCallback(GtkCellRendererText* renderer,
									gchar* pathString, gchar* newText, ToolbarManageDialog* dlg);

	static void buttonNewCallback(GtkButton* button, ToolbarManageDialog* dlg);
	static void buttonDeleteCallback(GtkButton* button, ToolbarManageDialog* dlg);
	static void buttonCopyCallback(GtkButton* button, ToolbarManageDialog* dlg);

	void addToolbarData(ToolbarData* data);
	void entrySelected(ToolbarData* data);

private:
	XOJ_TYPE_ATTRIB;

	ToolbarModel* tbModel;
	GtkListStore* model;

	ToolbarData* selected;
};

#endif /* __TOOLBARMANAGEDIALOG_H__ */
