#include "SelectionView.h"

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include "model/ElementContainer.h"  // for ElementContainer

#include "View.h"  // for ElementView

class Element;

using namespace xoj::view;

SelectionView::SelectionView(const ElementContainer* container): container(container) {}

void SelectionView::draw(const Context& ctx) const {
    for (Element* e: container->getElements()) {
        auto elementView = ElementView::createFromElement(e);
        elementView->draw(ctx);
    }
}
