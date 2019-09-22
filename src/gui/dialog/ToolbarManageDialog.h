/*
 * Xournal++
 *
 * Toolbar edit dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"

class ToolbarData;
class ToolbarModel;

class ToolbarManageDialog : public GladeGui
{
public:
	ToolbarManageDialog(GladeSearchpath* gladeSearchPath, ToolbarModel* model);
	virtual ~ToolbarManageDialog();

public:
	virtual void show(GtkWindow* parent);

private:
	static void treeSelectionChangedCallback(GtkTreeSelection* selection, ToolbarManageDialog* dlg);
	static void treeCellEditedCallback(GtkCellRendererText* renderer, gchar* pathString,
									   gchar* newText, ToolbarManageDialog* dlg);

	static void buttonNewCallback(GtkButton* button, ToolbarManageDialog* dlg);
	static void buttonDeleteCallback(GtkButton* button, ToolbarManageDialog* dlg);
	static void buttonCopyCallback(GtkButton* button, ToolbarManageDialog* dlg);

	void addToolbarData(ToolbarData* data);
	void entrySelected(ToolbarData* data);

	void updateSelectionData();

	ToolbarData* getSelectedEntry();

private:
	ToolbarModel* tbModel;
	GtkListStore* model;
};
