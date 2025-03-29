#include "ElementContainerView.h"

#include <memory>  // for unique_ptr

#include "model/ElementContainer.h"  // for ElementContainer

#include "View.h"  // for ElementView

class Element;

using namespace xoj::view;

ElementContainerView::ElementContainerView(const ElementContainer* container): container(container) {}

void ElementContainerView::draw(const Context& ctx) const {
    container->forEachElement([&ctx](const Element* e) {
        auto elementView = ElementView::createFromElement(e);
        elementView->draw(ctx);
    });
}
