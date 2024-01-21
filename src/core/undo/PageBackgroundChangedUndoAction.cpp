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
                                                                 size_t origPdfPage,
                                                                 BackgroundImage origBackgroundImage, double origW,
                                                                 double origH):
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
    Document* doc = control->getDocument();
    doc->lock();
    this->newType = this->page->getBackgroundType();
    this->newPdfPage = this->page->getPdfPageNr();
    this->newBackgroundImage = this->page->getBackgroundImage();
    this->newW = this->page->getWidth();
    this->newH = this->page->getHeight();

    auto pageNr = doc->indexOf(this->page);
    if (pageNr == npos) {
        doc->unlock();
        return false;
    }

    if (this->newW != this->origW || this->newH != this->origH) {
        this->page->setSize(this->origW, this->origH);
        doc->unlock();
        control->firePageSizeChanged(pageNr);
        doc->lock();
    }

    this->page->setBackgroundType(this->origType);
    if (this->origType.isPdfPage()) {
        this->page->setBackgroundPdfPageNr(this->origPdfPage);
    } else if (this->origType.isImagePage()) {
        this->page->setBackgroundImage(this->origBackgroundImage);
    }

    doc->unlock();
    control->firePageChanged(pageNr);

    return true;
}

auto PageBackgroundChangedUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();

    auto pageNr = doc->indexOf(this->page);

    if (pageNr == npos) {
        doc->unlock();
        return false;
    }

    if (this->newW != this->origW || this->newH != this->origH) {
        this->page->setSize(this->newW, this->newH);
        doc->unlock();
        control->firePageSizeChanged(pageNr);
        doc->lock();
    }

    this->page->setBackgroundType(this->newType);
    if (this->newType.isPdfPage()) {
        this->page->setBackgroundPdfPageNr(this->newPdfPage);
    } else if (this->newType.isImagePage()) {
        this->page->setBackgroundImage(this->newBackgroundImage);
    }

    doc->unlock();
    control->firePageChanged(pageNr);

    return true;
}

auto PageBackgroundChangedUndoAction::getText() -> std::string { return _("Page background changed"); }
