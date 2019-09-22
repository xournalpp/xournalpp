#include "EmergencySaveRestore.h"

#include <i18n.h>

EmergencySaveRestore::EmergencySaveRestore()
 : UndoAction("EmergencySaveRestore")
{
}

EmergencySaveRestore::~EmergencySaveRestore()
{
}

bool EmergencySaveRestore::redo(Control* control)
{
	// Does nothing, only used to mark the document as changed
	return true;
}

bool EmergencySaveRestore::undo(Control* control)
{
	// Does nothing, only used to mark the document as changed
	return true;
}

string EmergencySaveRestore::getText()
{
	return _("Emergency saved document");
}
