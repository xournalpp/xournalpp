/*
 * Xournal++
 *
 * Handles text search on a PDF page and in Xournal Texts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include "model/OverlayBase.h"
#include "model/PageRef.h"        // for PageRef
#include "model/Point.h"          // for Point
#include "pdf/base/XojPdfPage.h"  // for XojPdfPageSPtr, XojPdfRectangle
#include "util/DispatchPool.h"

namespace xoj::view {
class OverlayView;
class Repaintable;
class SearchResultView;
};  // namespace xoj::view

class SearchControl: public OverlayBase {
public:
    SearchControl(const PageRef& page, XojPdfPageSPtr pdf);
    virtual ~SearchControl();

    bool search(const std::string& text, size_t index, size_t* occurrences, XojPdfRectangle* UpperMostMatch);

    const std::vector<XojPdfRectangle>& getResults() const { return results; }

    const XojPdfRectangle* getHighlightRect() const { return highlightRect; }

    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SearchResultView>>& getViewPool() const {
        return viewPool;
    }

private:
    PageRef page;
    XojPdfPageSPtr pdf;
    std::string currentText;
    XojPdfRectangle* highlightRect = nullptr;

    std::vector<XojPdfRectangle> results;
    std::shared_ptr<xoj::util::DispatchPool<xoj::view::SearchResultView>> viewPool;
};
