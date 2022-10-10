#include "ElementContainerView.h"

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include "model/ElementContainer.h"  // for ElementContainer

#include "View.h"  // for ElementView

class Element;

using namespace xoj::view;

ElementContainerView::ElementContainerView(const ElementContainer* container): container(container) {}

void ElementContainerView::draw(const Context& ctx) const {
    for (Element* e: container->getElements()) {
        auto elementView = ElementView::createFromElement(e);
        elementView->draw(ctx);
    }
}
