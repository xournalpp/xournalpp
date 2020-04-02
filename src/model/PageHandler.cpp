#include "PageHandler.h"

#include "PageListener.h"

PageHandler::PageHandler() = default;

PageHandler::~PageHandler() = default;

void PageHandler::addListener(PageListener* l) { this->listener.push_back(l); }

void PageHandler::removeListener(PageListener* l) { this->listener.remove(l); }

void PageHandler::fireRectChanged(Rectangle<double>& rect) {
    for (PageListener* pl: this->listener) {
        pl->rectChanged(rect);
    }
}

void PageHandler::fireRangeChanged(Range& range) {
    for (PageListener* pl: this->listener) {
        pl->rangeChanged(range);
    }
}

void PageHandler::fireElementChanged(Element* elem) {
    for (PageListener* pl: this->listener) {
        pl->elementChanged(elem);
    }
}

void PageHandler::firePageChanged() {
    for (PageListener* pl: this->listener) {
        pl->pageChanged();
    }
}
