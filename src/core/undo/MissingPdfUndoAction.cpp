#include "MissingPdfUndoAction.h"

#include "control/Control.h"                         // for Control
#include "control/PageBackgroundChangeController.h"  // for PageBackGroundChangeController
#include "gui/MainWindow.h"                          // for MainWindow
#include "gui/XournalView.h"                         // for XournalView
#include "model/Document.h"                          // for Document
#include "model/PageType.h"                          // for PageTypeFormat
#include "model/XojPage.h"                           // for XojPage
#include "undo/UndoAction.h"                         // for UndoAction
#include "util/i18n.h"                               // for _

MissingPdfUndoAction::MissingPdfUndoAction(const fs::path& oldFilepath, bool oldAttachPdf):
        UndoAction("MissingPdfUndoAction"), filepath(oldFilepath), attachPdf(oldAttachPdf) {}

MissingPdfUndoAction::~MissingPdfUndoAction() = default;

auto MissingPdfUndoAction::undo(Control* control) -> bool {
    Document* doc = control->getDocument();

    fs::path redoFilepath = doc->getPdfFilepath();
    bool redoAttachPdf = doc->isAttachPdf();

    doc->resetPdf();
    doc->setPdfAttributes(this->filepath, this->attachPdf);

    doc->unlock();
    control->getWindow()->getXournal()->recreatePdfCache();
    doc->lock();

    for (size_t p = 0; p < doc->getPageCount(); p++) {
        if (doc->getPage(p)->getBackgroundType().format == PageTypeFormat::Pdf) {
            control->firePageChanged(p);
        }
    }

    this->filepath = std::move(redoFilepath);
    this->attachPdf = redoAttachPdf;

    return true;
}

auto MissingPdfUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();

    fs::path undoFilepath = doc->getPdfFilepath();
    bool undoAttachPdf = doc->isAttachPdf();

    doc->unlock();
    doc->readPdf(this->filepath, false, this->attachPdf);
    control->getWindow()->getXournal()->recreatePdfCache();
    doc->lock();

    for (size_t p = 0; p < doc->getPageCount(); p++) {
        if (doc->getPage(p)->getBackgroundType().format == PageTypeFormat::Pdf) {
            control->firePageChanged(p);
        }
    }

    this->filepath = std::move(undoFilepath);
    this->attachPdf = undoAttachPdf;

    return true;
}

auto MissingPdfUndoAction::getText() -> std::string { return _("Replace missing PDF"); }
