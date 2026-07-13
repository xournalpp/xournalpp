#include "MissingPdfUndoAction.h"

#include "control/Control.h"  // for Control
#include "model/Document.h"   // for Document
#include "undo/UndoAction.h"  // for UndoAction
#include "util/i18n.h"        // for _

MissingPdfUndoAction::MissingPdfUndoAction(const fs::path& oldFilepath, bool oldAttachPdf):
        UndoAction("MissingPdfUndoAction"), filepath(oldFilepath), attachPdf(oldAttachPdf) {}

MissingPdfUndoAction::~MissingPdfUndoAction() = default;

auto MissingPdfUndoAction::undo(Control* control) -> bool {
    Document* doc = control->getDocument();

    fs::path redoFilepath = doc->getPdfFilepath();
    bool redoAttachPdf = doc->isAttachPdf();

    doc->lock();
    doc->resetPdf();
    doc->setPdfAttributes(this->filepath, this->attachPdf);
    doc->unlock();

    control->firePdfContentChanged();
    control->refreshAfterPdfChange();

    this->filepath = std::move(redoFilepath);
    this->attachPdf = redoAttachPdf;

    return true;
}

auto MissingPdfUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();

    doc->lock();
    fs::path undoFilepath = doc->getPdfFilepath();
    bool undoAttachPdf = doc->isAttachPdf();
    doc->unlock();

    doc->readPdf(this->filepath, false, this->attachPdf);
    control->firePdfContentChanged();
    control->refreshAfterPdfChange();

    this->filepath = std::move(undoFilepath);
    this->attachPdf = undoAttachPdf;

    return true;
}

auto MissingPdfUndoAction::getText() -> std::string { return _("Replace missing PDF"); }
