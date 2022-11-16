#include "PageHandler.h"

#include "PageListener.h"  // for PageListener

namespace xoj::util {
template <class T>
class Rectangle;
}  // namespace xoj::util

using xoj::util::Rectangle;

PageHandler::PageHandler() = default;

PageHandler::~PageHandler() = default;

void PageHandler::addListener(PageListener* l) { this->listeners.push_back(l); }

void PageHandler::removeListener(PageListener* l) { this->listeners.remove(l); }

void PageHandler::fireRectChanged(Rectangle<double>& rect) {
    for (PageListener* pl: this->listeners) { pl->rectChanged(rect); }
}

void PageHandler::fireRangeChanged(Range& range) {
    for (PageListener* pl: this->listeners) { pl->rangeChanged(range); }
}

void PageHandler::fireElementChanged(Element* elem) {
    for (PageListener* pl: this->listeners) { pl->elementChanged(elem); }
}

void PageHandler::fireElementsChanged(const std::vector<Element*>& elements, Range range) {
    for (PageListener* pl: this->listeners) {
        pl->elementsChanged(elements, range);
    }
}

void PageHandler::firePageChanged() {
    for (PageListener* pl: this->listeners) { pl->pageChanged(); }
}
