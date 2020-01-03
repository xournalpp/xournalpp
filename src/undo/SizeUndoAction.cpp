#include "SizeUndoAction.h"

#include <utility>

#include "gui/Redrawable.h"
#include "model/Stroke.h"

#include "Range.h"
#include "i18n.h"

class SizeUndoActionEntry {
public:
    SizeUndoActionEntry(Stroke* s, double orignalWidth, double newWidth, vector<double> originalPressure,
                        vector<double> newPressure, int pressureCount) {
        this->s = s;
        this->orignalWidth = orignalWidth;
        this->newWidth = newWidth;
        this->originalPressure = std::move(originalPressure);
        this->newPressure = std::move(newPressure);
        this->pressureCount = pressureCount;
    }

    ~SizeUndoActionEntry() = default;
    Stroke* s;
    double orignalWidth;
    double newWidth;

    vector<double> originalPressure;
    vector<double> newPressure;
    int pressureCount;
};

SizeUndoAction::SizeUndoAction(const PageRef& page, Layer* layer): UndoAction("SizeUndoAction") {
    this->page = page;
    this->layer = layer;
}

SizeUndoAction::~SizeUndoAction() {
    for (SizeUndoActionEntry* e: this->data) {
        delete e;
    }
    this->data.clear();
}

auto SizeUndoAction::getPressure(Stroke* s) -> vector<double> {
    int count = s->getPointCount();
    vector<double> data;
    data.reserve(count);
    for (int i = 0; i < count; i++) {
        data.push_back(s->getPoint(i).z);
    }

    return data;
}

void SizeUndoAction::addStroke(Stroke* s, double originalWidth, double newWidth, vector<double> originalPressure,
                               vector<double> newPressure, int pressureCount) {
    this->data.push_back(new SizeUndoActionEntry(s, originalWidth, newWidth, std::move(originalPressure),
                                                 std::move(newPressure), pressureCount));
}

auto SizeUndoAction::undo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    SizeUndoActionEntry* e = this->data.front();
    Range range(e->s->getX(), e->s->getY());

    for (SizeUndoActionEntry* e: this->data) {
        e->s->setWidth(e->orignalWidth);
        e->s->setPressure(e->originalPressure);

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
        e->s->setPressure(e->newPressure);

        range.addPoint(e->s->getX(), e->s->getY());
        range.addPoint(e->s->getX() + e->s->getElementWidth(), e->s->getY() + e->s->getElementHeight());
    }

    this->page->fireRangeChanged(range);

    return true;
}

auto SizeUndoAction::getText() -> string { return _("Change stroke width"); }
