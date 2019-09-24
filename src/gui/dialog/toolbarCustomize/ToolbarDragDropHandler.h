/*
 * Xournal++
 *
 * Toolbar drag & drop controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class Control;
class MainWindow;
class ToolbarAdapter;
class ToolbarCustomizeDialog;

class ToolbarDragDropHandler
{
public:
	ToolbarDragDropHandler(Control* control);
	virtual ~ToolbarDragDropHandler();

public:
	void configure();

	void toolbarConfigDialogClosed();

	bool isInDragAndDrop();

public:
	void prepareToolbarsForDragAndDrop();
	void clearToolbarsFromDragAndDrop();

private:
	Control* control;

	ToolbarAdapter** toolbars = nullptr;
	ToolbarCustomizeDialog* customizeDialog = nullptr;
};
