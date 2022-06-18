#include "PageBackgroundChangedUndoAction.h"

#include <memory>   // for __shared_ptr_access, allocator
#include <utility>  // for move

#include "control/Control.h"  // for Control
#include "model/Document.h"   // for Document
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Util.h"        // for npos
#include "util/i18n.h"        // for _

PageBackgroundChangedUndoAction::PageBackgroundChangedUndoAction(const PageRef& page, const PageType& origType,
                                                                 int origPdfPage, BackgroundImage origBackgroundImage,
                                                                 double origW, double origH):
        UndoAction("PageBackgroundChangedUndoAction") {
    this->page = page;
    this->origType = origType;
    this->origPdfPage = origPdfPage;
    this->origBackgroundImage = std::move(origBackgroundImage);
    this->origW = origW;
    this->origH = origH;
    this->newType.format = PageTypeFormat::Plain;
}

PageBackgroundChangedUndoAction::~PageBackgroundChangedUndoAction() = default;

auto PageBackgroundChangedUndoAction::undo(Control* control) -> bool {
    this->newType = this->page->getBackgroundType();
    this->newPdfPage = this->page->getPdfPageNr();
    this->newBackgroundImage = this->page->getBackgroundImage();
    this->newW = this->page->getWidth();
    this->newH = this->page->getHeight();

    Document* doc = control->getDocument();
    auto pageNr = doc->indexOf(this->page);
    if (pageNr == npos) {
        return false;
    }

    if (this->newW != this->origW || this->newH != this->origH) {
        this->page->setSize(this->origW, this->origH);
        control->firePageSizeChanged(pageNr);
    }

    this->page->setBackgroundType(this->origType);
    if (this->origType.isPdfPage()) {
        this->page->setBackgroundPdfPageNr(this->origPdfPage);
    } else if (this->origType.isImagePage()) {
        this->page->setBackgroundImage(this->origBackgroundImage);
    }

    control->firePageChanged(pageNr);

    return true;
}

auto PageBackgroundChangedUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();

    auto pageNr = doc->indexOf(this->page);

    if (pageNr == npos) {
        return false;
    }

    if (this->newW != this->origW || this->newH != this->origH) {
        this->page->setSize(this->newW, this->newH);
        control->firePageSizeChanged(pageNr);
    }

    this->page->setBackgroundType(this->newType);
    if (this->newType.isPdfPage()) {
        this->page->setBackgroundPdfPageNr(this->newPdfPage);
    } else if (this->newType.isImagePage()) {
        this->page->setBackgroundImage(this->newBackgroundImage);
    }

    control->firePageChanged(pageNr);

    return true;
}

auto PageBackgroundChangedUndoAction::getText() -> std::string { return _("Page background changed"); }
