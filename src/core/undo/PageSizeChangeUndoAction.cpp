#include "PageSizeChangeUndoAction.h"

#include <memory>  // for __shared_ptr_access, allocator

#include "control/Control.h"  // for Control
#include "model/Document.h"   // for Document
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Util.h"        // for npos
#include "util/i18n.h"        // for _

PageSizeChangeUndoAction::PageSizeChangeUndoAction(const PageRef& page, double origW, double origH):
        UndoAction("PageSizeChangeUndoAction"), otherWidth(origW), otherHeight(origH) {
    this->page = page;
}

PageSizeChangeUndoAction::~PageSizeChangeUndoAction() = default;

auto PageSizeChangeUndoAction::swapSizes(Control* control) -> bool {
    Document* doc = control->getDocument();

    doc->lock();
    auto pageNr = doc->indexOf(this->page);
    if (pageNr == npos) {
        return false;
    }

    double w = this->page->getWidth();
    double h = this->page->getHeight();

    this->page->setSize(std::exchange(this->otherWidth, w), std::exchange(this->otherHeight, h));
    doc->unlock();

    control->firePageSizeChanged(pageNr);

    return true;
}

auto PageSizeChangeUndoAction::undo(Control* control) -> bool { return swapSizes(control); }
auto PageSizeChangeUndoAction::redo(Control* control) -> bool { return swapSizes(control); }

auto PageSizeChangeUndoAction::getText() -> std::string { return _("Page size changed"); }
