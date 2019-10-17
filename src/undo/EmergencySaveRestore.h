/*
 * Xournal++
 *
 * Element used to mark the document as changed, so it is changed after restore.
 * Without this, it's unchanged and does not ask for save
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class EmergencySaveRestore : public UndoAction
{
public:
	EmergencySaveRestore();
	virtual ~EmergencySaveRestore();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	};
