/*
 * Xournal++
 *
 * Toolbar drag & drop controller
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARDRAGDROPHANDLER_H__
#define __TOOLBARDRAGDROPHANDLER_H__

#include <XournalType.h>
#include "ToolbarAdapter.h"
#include "ToolbarListener.h"

class Control;
class ToolbarAdapter;
class ToolbarCustomizeDialog;
class MainWindow;

class ToolbarDragDropHandler : public ToolbarListener {
public:
	ToolbarDragDropHandler(Control * control);
	virtual ~ToolbarDragDropHandler();

public:
	void configure();

	virtual void toolbarDataChanged();

	void toolbarConfigDialogClosed();

	bool isInDragAndDrop();
public:
	void prepareToolbarsForDragAndDrop();
	void clearToolbarsFromDragAndDrop();

private:
	XOJ_TYPE_ATTRIB;

	Control * control;

	ToolbarAdapter ** toolbars;
	ToolbarCustomizeDialog * customizeDialog;
};

#endif /* __TOOLBARDRAGDROPHANDLER_H__ */
