#include "SearchControl.h"

#include <algorithm>  // for min_element
#include <memory>     // for __shared_ptr_access
#include <utility>    // for move

#include "model/Element.h"  // for Element, ELEMENT_TEXT
#include "model/Layer.h"    // for Layer
#include "model/Text.h"     // for Text
#include "model/XojPage.h"  // for XojPage
#include "view/overlays/SearchResultView.h"

SearchControl::SearchControl(const PageRef& page, XojPdfPageSPtr pdf):
        page(page),
        pdf(std::move(pdf)),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::SearchResultView>>()) {}

SearchControl::~SearchControl() = default;

auto SearchControl::search(const std::string& text, size_t* occurrences, XojPdfRectangle* upperMostMatch) -> bool {
    if (text.empty()) {
        if (!this->results.empty()) {
            this->results.clear();
            this->viewPool->dispatch(xoj::view::SearchResultView::SEARCH_CHANGED_NOTIFICATION);
        }
        return true;
    }

    this->results.clear();

    if (this->pdf) {
        this->results = this->pdf->findText(text);
    }

    for (Layer* l: *this->page->getLayers()) {
        if (!l->isVisible()) {
            continue;
        }

        for (Element* e: l->getElements()) {
            if (e->getType() == ELEMENT_TEXT) {
                Text* t = dynamic_cast<Text*>(e);

                std::vector<XojPdfRectangle> textResult = t->findText(text);

                this->results.insert(this->results.end(), textResult.begin(), textResult.end());
            }
        }
    }

    if (occurrences) {
        *occurrences = this->results.size();
    }

    if (upperMostMatch) {
        if (this->results.empty()) {
            *upperMostMatch = XojPdfRectangle(0, 0, 0, 0);
        } else {
            *upperMostMatch =
                    *std::min_element(this->results.begin(), this->results.end(),
                                      [](const XojPdfRectangle& a, const XojPdfRectangle& b) { return a.y1 < b.y1; });
        }
    }

    this->viewPool->dispatch(xoj::view::SearchResultView::SEARCH_CHANGED_NOTIFICATION);
    return !this->results.empty();
}
