#include "EmergencySaveRestore.h"

#include "i18n.h"

EmergencySaveRestore::EmergencySaveRestore(): UndoAction("EmergencySaveRestore") {}

EmergencySaveRestore::~EmergencySaveRestore() = default;

auto EmergencySaveRestore::redo(Control* control) -> bool {
    // Does nothing, only used to mark the document as changed
    return true;
}

auto EmergencySaveRestore::undo(Control* control) -> bool {
    // Does nothing, only used to mark the document as changed
    return true;
}

auto EmergencySaveRestore::getText() -> string { return _("Emergency saved document"); }
