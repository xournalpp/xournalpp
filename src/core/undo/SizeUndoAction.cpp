#include "SizeUndoAction.h"

#include <memory>   // for allocator, __shared_ptr_access, __share...
#include <utility>  // for move

#include "model/Point.h"      // for Point
#include "model/Stroke.h"     // for Stroke
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Range.h"       // for Range
#include "util/i18n.h"        // for _

class Control;

using std::vector;

class SizeUndoActionEntry {
public:
    SizeUndoActionEntry(Stroke* s, double originalWidth, double newWidth, vector<double> originalPressure,
                        vector<double> newPressure) {
        this->s = s;
        this->originalWidth = originalWidth;
        this->newWidth = newWidth;
        this->originalPressure = std::move(originalPressure);
        this->newPressure = std::move(newPressure);
    }

    ~SizeUndoActionEntry() = default;
    Stroke* s;
    double originalWidth;
    double newWidth;

    vector<double> originalPressure;
    vector<double> newPressure;
};

SizeUndoAction::SizeUndoAction(const PageRef& page, Layer* layer): UndoAction("SizeUndoAction") {
    this->page = page;
    this->layer = layer;
}

SizeUndoAction::~SizeUndoAction() {
    for (SizeUndoActionEntry* e: this->data) { delete e; }
    this->data.clear();
}

void SizeUndoAction::addStroke(Stroke* s, double originalWidth, double newWidth, vector<double> originalPressure,
                               vector<double> newPressure) {
    this->data.push_back(
            new SizeUndoActionEntry(s, originalWidth, newWidth, std::move(originalPressure), std::move(newPressure)));
}

auto SizeUndoAction::undo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    SizeUndoActionEntry* e = this->data.front();
    Range range(e->s->getX(), e->s->getY());

    for (SizeUndoActionEntry* e: this->data) {
        e->s->setWidth(e->originalWidth);
        e->s->setPressureValues(e->originalPressure);

        range.addPoint(e->s->getX(), e->s->getY());
        range.addPoint(e->s->getX() + e->s->getElementWidth(), e->s->getY() + e->s->getElementHeight());
    }

    this->page->fireRangeChanged(range);

    return true;
}

auto SizeUndoAction::redo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    SizeUndoActionEntry* e = this->data.front();
    Range range(e->s->getX(), e->s->getY());

    for (SizeUndoActionEntry* e: this->data) {
        e->s->setWidth(e->newWidth);
        e->s->setPressureValues(e->newPressure);

        range.addPoint(e->s->getX(), e->s->getY());
        range.addPoint(e->s->getX() + e->s->getElementWidth(), e->s->getY() + e->s->getElementHeight());
    }

    this->page->fireRangeChanged(range);

    return true;
}

auto SizeUndoAction::getText() -> std::string { return _("Change stroke width"); }
