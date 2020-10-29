#include "EraseHandler.h"

#include <cmath>
#include <memory>

#include "control/ToolHandler.h"
#include "control/jobs/RenderJob.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/eraser/EraseableStroke.h"
#include "undo/DeleteUndoAction.h"
#include "undo/EraseUndoAction.h"
#include "undo/UndoRedoHandler.h"

#include "Range.h"
#include "Rectangle.h"

EraseHandler::EraseHandler(UndoRedoHandler* undo, Document* doc, const PageRef& page, ToolHandler* handler,
                           Redrawable* view) {
    this->page = page;
    this->handler = handler;
    this->view = view;
    this->doc = doc;

    this->eraseDeleteUndoAction = nullptr;
    this->eraseUndoAction = nullptr;
    this->undo = undo;

    this->halfEraserSize = 0;
}

EraseHandler::~EraseHandler() {
    if (this->eraseDeleteUndoAction) {
        this->finalize();
    }
}

/**
 * Handle eraser event: "Delete Stroke" and "Standard", Whiteout is not handled here
 */
void EraseHandler::erase(double x, double y) {
    this->halfEraserSize = this->handler->getThickness();
    GdkRectangle eraserRect = {gint(x - halfEraserSize), gint(y - halfEraserSize), gint(halfEraserSize * 2),
                               gint(halfEraserSize * 2)};

    auto* range = new Range(x, y);

    Layer* l = page->getSelectedLayer();

    vector<Element*> tmp(*l->getElements());
    for (Element* e: tmp) {
        if (e->getType() == ELEMENT_STROKE && e->intersectsArea(&eraserRect)) {
            eraseStroke(l, dynamic_cast<Stroke*>(e), x, y, range);
        }
    }

    this->view->rerenderRange(*range);
    delete range;
}

void EraseHandler::eraseStroke(Layer* l, Stroke* s, double x, double y, Range* range) {
    if (!s->intersects(x, y, halfEraserSize)) {
        return;
    }

    // delete complete element
    if (this->handler->getEraserType() == ERASER_TYPE_DELETE_STROKE) {
        this->doc->lock();
        int pos = l->removeElement(s, false);
        this->doc->unlock();

        if (pos == -1) {
            return;
        }
        range->addPoint(s->getX(), s->getY());
        range->addPoint(s->getX() + s->getElementWidth(), s->getY() + s->getElementHeight());

        // removed the if statement - this prevents us from putting multiple elements into a
        // stroke erase operation, but it also prevents the crashing and layer issues!
        if (!this->eraseDeleteUndoAction) {
            auto eraseDel = std::make_unique<DeleteUndoAction>(this->page, true);
            // Todo check dangerous: this->eraseDeleteUndoAction could be a dangling reference
            this->eraseDeleteUndoAction = eraseDel.get();
            this->undo->addUndoAction(std::move(eraseDel));
        }

        this->eraseDeleteUndoAction->addElement(l, s, pos);
    } else  // Default eraser
    {
        int pos = l->indexOf(s);
        if (pos == -1) {
            return;
        }

        if (this->eraseUndoAction == nullptr) {
            auto eraseUndo = std::make_unique<EraseUndoAction>(this->page);
            // Todo check dangerous: this->eraseDeleteUndoAction could be a dangling reference
            this->eraseUndoAction = eraseUndo.get();
            this->undo->addUndoAction(std::move(eraseUndo));
        }

        EraseableStroke* eraseable = nullptr;
        if (s->getEraseable() == nullptr) {
            doc->lock();
            eraseable = new EraseableStroke(s);
            s->setEraseable(eraseable);
            doc->unlock();
            this->eraseUndoAction->addOriginal(l, s, pos);
        } else {
            eraseable = s->getEraseable();
        }

        eraseable->erase(x, y, halfEraserSize, range);
    }
}

void EraseHandler::finalize() {
    if (this->eraseUndoAction) {
        this->eraseUndoAction->finalize();
        this->eraseUndoAction = nullptr;
    } else if (this->eraseDeleteUndoAction) {
        this->eraseDeleteUndoAction = nullptr;
    }
}
