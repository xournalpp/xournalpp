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
class ToolbarAdapter;
class ToolbarCustomizeDialog;
class MainWindow;

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
	XOJ_TYPE_ATTRIB;

	Control* control;

	ToolbarAdapter** toolbars;
	ToolbarCustomizeDialog* customizeDialog;
};
