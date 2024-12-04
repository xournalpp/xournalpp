#include "PageBackgroundChangeController.h"

#include <memory>   // for __shared_ptr...
#include <string>   // for allocator
#include <utility>  // for move

#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_g...
#include <gio/gio.h>                // for GFile
#include <glib.h>                   // for g_error_free

#include "control/settings/PageTemplateSettings.h"       // for PageTemplate...
#include "control/settings/Settings.h"                   // for Settings
#include "gui/MainWindow.h"                              // for MainWindow
#include "gui/XournalView.h"                             // for XournalView
#include "gui/dialog/backgroundSelect/ImagesDialog.h"    // for ImagesDialog
#include "gui/dialog/backgroundSelect/PdfPagesDialog.h"  // for PdfPagesDialog
#include "gui/menus/menubar/Menubar.h"                   // for Menubar
#include "gui/menus/menubar/PageTypeSubmenu.h"           // for PageTypeSubmenu
#include "model/BackgroundImage.h"                       // for BackgroundImage
#include "model/Document.h"                              // for Document
#include "model/PageType.h"                              // for PageType
#include "model/XojPage.h"                               // for XojPage
#include "pdf/base/XojPdfPage.h"                         // for XojPdfPageSPtr
#include "undo/GroupUndoAction.h"                        // for GroupUndoAction
#include "undo/MissingPdfUndoAction.h"                   // for MissingPdfUn...
#include "undo/PageBackgroundChangedUndoAction.h"        // for PageBackgrou...
#include "undo/UndoAction.h"                             // for UndoAction
#include "undo/UndoRedoHandler.h"                        // for UndoRedoHandler
#include "util/Assert.h"                                 // for xoj_assert
#include "util/PathUtil.h"                               // for fromGFile
#include "util/PopupWindowWrapper.h"
#include "util/Util.h"       // for npos
#include "util/XojMsgBox.h"  // for XojMsgBox
#include "util/i18n.h"       // for FS, _, _F

#include "Control.h"     // for Control
#include "filesystem.h"  // for path


PageBackgroundChangeController::PageBackgroundChangeController(Control* control): control(control) {
    registerListener(control);
}

void PageBackgroundChangeController::applyBackgroundToAllPages(const PageType& pt) {
    control->clearSelectionEndText();

    auto apply = [pt, this](CommitParameter param = std::nullopt) {
        auto groupUndoAction = std::make_unique<GroupUndoAction>();

        Document* doc = control->getDocument();
        doc->lock();
        auto nbPages = doc->getPageCount();
        for (size_t p = 0; p < nbPages; p++) {
            auto undoAction = commitPageTypeChange(p, pt, param);
            if (undoAction) {
                groupUndoAction->addAction(std::move(undoAction));
            }
        }
        doc->unlock();

        control->getUndoRedoHandler()->addUndoAction(std::move(groupUndoAction));

        // Special background types may alter the page sizes as well
        auto fire = pt.isSpecial() ? &Control::firePageSizeChanged : &Control::firePageChanged;
        for (size_t n = 0; n < nbPages; n++) {
            (control->*fire)(n);
        }
        control->updateBackgroundSizeButton();
        control->getWindow()->getMenubar()->getPageTypeSubmenu().setSelectedPT(std::move(pt));
    };

    if (pt.isImagePage()) {
        askForImageBackground([apply = std::move(apply)](BackgroundImage img) { apply(std::move(img)); });
    } else if (pt.isPdfPage()) {
        askForPdfBackground([apply = std::move(apply)](size_t pdfPage) { apply(pdfPage); });
    } else {
        apply();
    }
}

void PageBackgroundChangeController::applyPageSizeToAllPages(const PaperSize& paperSize) {
    control->clearSelectionEndText();

    Document* doc = control->getDocument();

    auto groupUndoAction = std::make_unique<GroupUndoAction>();

    for (size_t p = 0; p < doc->getPageCount(); p++) {
        auto undoAction = commitPageSizeChange(p, paperSize);
        if (undoAction) {
            groupUndoAction->addAction(std::move(undoAction));
        }
    }

    control->getUndoRedoHandler()->addUndoAction(std::move(groupUndoAction));
}

void PageBackgroundChangeController::applyCurrentPageBackgroundToAll() {
    PageType pt = control->getCurrentPage()->getBackgroundType();
    applyBackgroundToAllPages(pt);
}

void PageBackgroundChangeController::changePdfPagesBackground(const fs::path& filepath, bool attachPdf) {
    Document* doc = this->control->getDocument();

    const fs::path oldFilepath = doc->getPdfFilepath();
    const bool oldAttachPdf = doc->isAttachPdf();

    if (!doc->readPdf(filepath, false, attachPdf)) {
        std::string msg = FS(_F("Error reading PDF: {1}") % doc->getLastErrorMsg());
        XojMsgBox::showErrorToUser(this->control->getGtkWindow(), msg);
        return;
    }
    this->control->getWindow()->getXournal()->recreatePdfCache();

    this->control->fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);

    auto undoAction = std::make_unique<MissingPdfUndoAction>(oldFilepath, oldAttachPdf);
    this->control->getUndoRedoHandler()->addUndoAction(std::move(undoAction));
}

void PageBackgroundChangeController::changeCurrentPageBackground(const PageType& pt) {
    control->clearSelectionEndText();

    const size_t pageNr = control->getCurrentPageNo();
    xoj_assert(pageNr != npos);

    auto apply = [this, pt, pageNr](CommitParameter param = std::nullopt) {
        Document* doc = control->getDocument();
        doc->lock();
        auto undoAction = commitPageTypeChange(pageNr, pt, std::move(param));
        doc->unlock();
        if (undoAction) {
            control->getUndoRedoHandler()->addUndoAction(std::move(undoAction));
        }

        // Special background types may alter the page sizes as well
        if (pt.isSpecial()) {
            control->firePageSizeChanged(pageNr);
        } else {
            control->firePageChanged(pageNr);
        }
        control->updateBackgroundSizeButton();
        control->getWindow()->getMenubar()->getPageTypeSubmenu().setSelectedPT(std::move(pt));
    };

    if (pt.isImagePage()) {
        askForImageBackground([apply = std::move(apply)](BackgroundImage img) { apply(std::move(img)); });
    } else if (pt.isPdfPage()) {
        askForPdfBackground([apply = std::move(apply)](size_t pdfPage) { apply(pdfPage); });
    } else {
        apply();
    }
}

void PageBackgroundChangeController::changeCurrentPageSize(const PaperSize& pageSize) {
    control->clearSelectionEndText();

    auto undoAction = commitPageSizeChange(control->getCurrentPageNo(), pageSize);
    if (undoAction) {
        control->getUndoRedoHandler()->addUndoAction(std::move(undoAction));
    }
}

void PageBackgroundChangeController::setPageTypeForNewPages(const std::optional<PageType>& pt) {
    this->pageTypeForNewPages = pt;
}
void PageBackgroundChangeController::setPaperSizeForNewPages(const std::optional<PaperSize>& ps) {
    this->paperSizeForNewPages = ps;
}

static void setPageImageBackground(const PageRef& page, BackgroundImage img) {
    page->setBackgroundImage(std::move(img));
    page->setBackgroundType(PageType(PageTypeFormat::Image));

    // Apply correct page size
    GdkPixbuf* pixbuf = page->getBackgroundImage().getPixbuf();
    if (pixbuf) {
        page->setSize(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
    } else {
        g_warning("PageBackgroundChangeController::setPageImageBackground(): Page with image background but nullptr "
                  "pixbuf");
    }
}

static void setPagePdfBackground(const PageRef& page, size_t pdfPageNr, Document* doc) {
    if (pdfPageNr < doc->getPdfPageCount()) {
        // no need to set a type, if we set the page number the type is also set
        page->setBackgroundPdfPageNr(pdfPageNr);

        XojPdfPageSPtr p = doc->getPdfPage(pdfPageNr);
        page->setSize(p->getWidth(), p->getHeight());
    } else {
        g_warning("PageBackgroundChangeController::setPagePdfBackground() with past-the-end PDF page number");
    }
}

auto PageBackgroundChangeController::commitPageTypeChange(size_t pageNum, const PageType& pageType,
                                                          CommitParameter param) -> std::unique_ptr<UndoAction> {
    xoj_assert(pageType.isImagePage() == std::holds_alternative<BackgroundImage>(param));
    xoj_assert(pageType.isPdfPage() == std::holds_alternative<size_t>(param));

    PageRef page = control->getDocument()->getPage(pageNum);
    if (!page) {
        g_warning("PageBackgroundChangeController::commitPageTypeChange() could not fetch page %zu", pageNum);
        return {};
    }

    Document* doc = control->getDocument();

    // Get values for Undo / Redo
    const double origW = page->getWidth();
    const double origH = page->getHeight();
    BackgroundImage origBackgroundImage = page->getBackgroundImage();
    const size_t origPdfPage = page->getPdfPageNr();
    PageType origType = page->getBackgroundType();

    // Apply the new background
    if (std::holds_alternative<BackgroundImage>(param)) {  // Image background
        xoj_assert(!std::get<BackgroundImage>(param).isEmpty());
        setPageImageBackground(page, std::move(std::get<BackgroundImage>(param)));
    } else if (std::holds_alternative<size_t>(param)) {  // PDF background
        setPagePdfBackground(page, std::get<size_t>(param), doc);
    } else {
        page->setBackgroundType(pageType);
    }

    return std::make_unique<PageBackgroundChangedUndoAction>(page, origType, origPdfPage, origBackgroundImage, origW,
                                                             origH);
}
auto PageBackgroundChangeController::commitPageSizeChange(const size_t pageNum, const PaperSize& pageSize)
        -> std::unique_ptr<UndoAction> {
    Document* doc = control->getDocument();
    doc->lock();
    PageRef page = doc->getPage(pageNum);
    if (!page) {
        doc->unlock();
        return {};
    }

    const size_t pageNr = doc->indexOf(page);
    xoj_assert(pageNr != npos);

    // Get values for Undo / Redo
    const double origW = page->getWidth();
    const double origH = page->getHeight();
    BackgroundImage origBackgroundImage = page->getBackgroundImage();
    const size_t origPdfPage = page->getPdfPageNr();
    PageType origType = page->getBackgroundType();

    page->setSize(pageSize.width, pageSize.height);
    doc->unlock();

    control->firePageSizeChanged(pageNr);
    return std::make_unique<PageBackgroundChangedUndoAction>(page, origType, origPdfPage, origBackgroundImage, origW,
                                                             origH);
}

void PageBackgroundChangeController::askForImageBackground(std::function<void(BackgroundImage)> callback) {
    Document* doc = control->getDocument();

    auto dlg = xoj::popup::PopupWindowWrapper<ImagesDialog>(control->getGladeSearchPath(), doc, control->getSettings(),
                                                            std::move(callback));
    dlg.show(control->getGtkWindow());
}

void PageBackgroundChangeController::askForPdfBackground(std::function<void(size_t)> callback) {
    Document* doc = control->getDocument();

    if (doc->getPdfPageCount() == 0) {
        std::string msg = _("You don't have any PDF pages to select from. Cancel operation.\n"
                            "Please select another background type: Menu \"Journal\" → \"Configure Page Template\".");
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
        return;
    }

    auto dlg = xoj::popup::PopupWindowWrapper<PdfPagesDialog>(control->getGladeSearchPath(), doc,
                                                              control->getSettings(), std::move(callback));
    dlg.show(control->getGtkWindow());
}

/**
 * Copy the background from source to target
 */
void PageBackgroundChangeController::copyBackgroundFromOtherPage(PageRef target, PageRef source) {
    // Copy page background type
    PageType bg = source->getBackgroundType();
    target->setBackgroundType(bg);

    if (bg.isPdfPage()) {
        // If PDF: Copy PDF Page and its size
        target->setSize(source->getWidth(), source->getHeight());
        target->setBackgroundPdfPageNr(source->getPdfPageNr());
    } else if (bg.isImagePage()) {
        // If Image: Copy the Image and its size
        target->setSize(source->getWidth(), source->getHeight());
        target->setBackgroundImage(source->getBackgroundImage());
    } else {
        // Copy the background color
        target->setBackgroundColor(source->getBackgroundColor());
    }
}

void PageBackgroundChangeController::insertNewPage(size_t position, bool shouldScrollToPage) {
    control->clearSelectionEndText();

    Document* doc = control->getDocument();
    if (position > doc->getPageCount()) {
        position = doc->getPageCount();
    }

    PageTemplateSettings model;
    model.parse(control->getSettings()->getPageTemplate());

    PageRef current = control->getCurrentPage();
    xoj_assert(current);
    double width, height;
    if (paperSizeForNewPages) {
        width = paperSizeForNewPages->width;
        height = paperSizeForNewPages->height;
    } else {
        width = current->getWidth();
        height = current->getHeight();
    }
    auto page = std::make_shared<XojPage>(width, height);

    auto afterConfigured = [position, shouldScrollToPage, ctrl = this->control](PageRef page) {
        ctrl->insertPage(page, position, shouldScrollToPage);
    };

    if (!pageTypeForNewPages) {
        copyBackgroundFromOtherPage(page, current);
        afterConfigured(std::move(page));
    } else if (pageTypeForNewPages->isImagePage()) {
        askForImageBackground([after = std::move(afterConfigured), page = std::move(page)](BackgroundImage img) {
            setPageImageBackground(page, std::move(img));
            after(std::move(page));
        });
    } else if (pageTypeForNewPages->isPdfPage()) {
        askForPdfBackground([after = std::move(afterConfigured), page = std::move(page), doc](size_t pdfPageNr) {
            setPagePdfBackground(page, pdfPageNr, doc);
            after(std::move(page));
        });
    } else {
        // Create a new page from template
        page->setBackgroundType(pageTypeForNewPages.value());

        // Set background Color
        page->setBackgroundColor(model.getBackgroundColor());

        afterConfigured(std::move(page));
    }
}

void PageBackgroundChangeController::documentChanged(DocumentChangeType type) {}

void PageBackgroundChangeController::pageSizeChanged(size_t page) {}

void PageBackgroundChangeController::pageChanged(size_t page) {}

void PageBackgroundChangeController::pageInserted(size_t page) {}

void PageBackgroundChangeController::pageDeleted(size_t page) {}

void PageBackgroundChangeController::pageSelected(size_t page) {
    auto const& current = control->getCurrentPage();
    if (!current) {
        return;
    }

    control->getWindow()->getMenubar()->getPageTypeSubmenu().setSelectedPT(current->getBackgroundType());
}
