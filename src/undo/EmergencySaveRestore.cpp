#include "EmergencySaveRestore.h"

#include <i18n.h>

EmergencySaveRestore::EmergencySaveRestore()
 : UndoAction("EmergencySaveRestore")
{
	XOJ_INIT_TYPE(EmergencySaveRestore);
}

EmergencySaveRestore::~EmergencySaveRestore()
{
	XOJ_CHECK_TYPE(EmergencySaveRestore);
	XOJ_RELEASE_TYPE(EmergencySaveRestore);
}

bool EmergencySaveRestore::redo(Control* control)
{
	XOJ_CHECK_TYPE(EmergencySaveRestore);

	// Does nothing, only used to mark the document as changed
	return true;
}

bool EmergencySaveRestore::undo(Control* control)
{
	XOJ_CHECK_TYPE(EmergencySaveRestore);

	// Does nothing, only used to mark the document as changed
	return true;
}

string EmergencySaveRestore::getText()
{
	XOJ_CHECK_TYPE(EmergencySaveRestore);
	return _("Emergency saved document");
}
