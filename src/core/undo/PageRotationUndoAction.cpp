#include "PageRotationUndoAction.h"

#include <memory>  // for __shared_ptr_access, allocator

#include "control/Control.h"  // for Control
#include "model/Document.h"   // for Document
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Util.h"        // for npos
#include "util/i18n.h"        // for _

PageRotationUndoAction::PageRotationUndoAction(const PageRef& page, int oldPgOrientation, int newPgOrientation):
        UndoAction("PageRotationUndoAction"), oldPgOrientation(oldPgOrientation), newPgOrientation(newPgOrientation) {
    this->page = page;
}

PageRotationUndoAction::~PageRotationUndoAction() = default;

auto PageRotationUndoAction::applyRotation(Control* control, int orientation) -> bool {
    Document* doc = control->getDocument();

    doc->lock();
    auto pageNr = doc->indexOf(this->page);
    if (pageNr == npos) {
        return false;
    }

    double w = this->page->getWidth();
    double h = this->page->getHeight();

    this->page->setSize(h, w);
    this->page->setPdfPageOrientation(orientation);
    doc->unlock();

    control->firePageSizeChanged(pageNr);

    return true;
}

auto PageRotationUndoAction::undo(Control* control) -> bool { return applyRotation(control, oldPgOrientation); }
auto PageRotationUndoAction::redo(Control* control) -> bool { return applyRotation(control, newPgOrientation); }

auto PageRotationUndoAction::getText() -> std::string { return _("Page rotation"); }
