#include "SearchControl.h"

#include <memory>   // for __shared_ptr_access
#include <utility>  // for move

#include "model/Element.h"                   // for Element, ELEMENT_TEXT
#include "model/Layer.h"                     // for Layer
#include "model/Text.h"                      // for Text
#include "model/XojPage.h"                   // for XojPage
#include "view/overlays/SearchResultView.h"  // for SEARCH_CHANGED_NOTIFICATION

SearchControl::SearchControl(const PageRef& page, XojPdfPageSPtr pdf):
        page(page),
        pdf(std::move(pdf)),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::SearchResultView>>()) {}

SearchControl::~SearchControl() = default;

auto SearchControl::search(const std::string& text, size_t index, size_t* occurrences, XojPdfRectangle* matchRect)
        -> bool {
    this->highlightRect = nullptr;
    if (text.empty()) {
        if (!this->results.empty()) {
            this->results.clear();
            this->currentText.clear();
            this->viewPool->dispatch(xoj::view::SearchResultView::SEARCH_CHANGED_NOTIFICATION);
        }
        return true;
    }

    if (text != this->currentText) {
        this->results.clear();
        this->currentText = text;

        if (this->pdf) {
            this->results = this->pdf->findText(text);
        }

        for (Layer* l: this->page->getLayers()) {
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
    }

    this->viewPool->dispatch(xoj::view::SearchResultView::SEARCH_CHANGED_NOTIFICATION);
    if (occurrences) {
        *occurrences = this->results.size();
    }
    bool found;
    if (index - 1 >= this->results.size()) {
        if (matchRect) {
            *matchRect = XojPdfRectangle(0, 0, 0, 0);
        }
        found = false;
    } else {
        if (matchRect) {
            this->highlightRect = &this->results[index - 1];
            *matchRect = *this->highlightRect;
        }
        found = true;
    }
    return found;
}
