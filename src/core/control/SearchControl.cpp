#include "SearchControl.h"

#include <algorithm>  // for min
#include <memory>     // for __shared_ptr_access
#include <utility>    // for move

#include "model/Element.h"  // for Element, ELEMENT_TEXT
#include "model/Layer.h"    // for Layer
#include "model/Text.h"     // for Text
#include "model/XojPage.h"  // for XojPage
#include "view/TextView.h"  // for TextView
#include "view/overlays/SearchResultView.h"

SearchControl::SearchControl(const PageRef& page, XojPdfPageSPtr pdf):
        page(page),
        pdf(std::move(pdf)),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::SearchResultView>>()) {}

SearchControl::~SearchControl() = default;

auto SearchControl::search(const std::string& text, size_t* occurrences, double* yOfUpperMostMatch) -> bool {
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

                std::vector<XojPdfRectangle> textResult = xoj::view::TextView::findText(t, text);

                this->results.insert(this->results.end(), textResult.begin(), textResult.end());
            }
        }
    }

    if (occurrences) {
        *occurrences = this->results.size();
    }

    if (yOfUpperMostMatch) {
        if (this->results.empty()) {
            *yOfUpperMostMatch = 0;
        } else {

            const XojPdfRectangle& first = this->results.front();

            double min = first.y1;
            for (const XojPdfRectangle& rect: this->results) {
                min = std::min(min, rect.y1);
            }

            *yOfUpperMostMatch = min;
        }
    }

    this->viewPool->dispatch(xoj::view::SearchResultView::SEARCH_CHANGED_NOTIFICATION);
    return !this->results.empty();
}
