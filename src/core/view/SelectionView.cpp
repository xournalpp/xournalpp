#include "SelectionView.h"

#include "model/Element.h"
#include "model/ElementContainer.h"

#include "View.h"

using namespace xoj::view;

SelectionView::SelectionView(const ElementContainer* container): container(container) {}

void SelectionView::draw(const Context& ctx) const {
    for (Element* e: container->getElements()) {
        auto elementView = ElementView::createFromElement(e);
        elementView->draw(ctx);
    }
}
