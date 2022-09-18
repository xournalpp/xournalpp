#include "ExportTemplate.h"

#include <cmath>  // for round
#include <utility>

#include <cairo-pdf.h>

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "model/Document.h"                 // for Document
#include "model/XojPage.h"                  // for XojPage
#include "util/i18n.h"                      // for _
#include "view/DocumentView.h"              // for DocumentView

ExportTemplate::ExportTemplate(Document* doc, fs::path filePath): doc{doc}, filePath{std::move(filePath)} {
    ElementRangeVector fullRange;
    fullRange.emplace_back(0, doc->getPageCount() - 1);
    setExportRange(fullRange);
}

ExportTemplate::~ExportTemplate() {}

void ExportTemplate::setExportBackground(const ExportBackgroundType exportBackground) {
    this->exportBackground = exportBackground;
}

void ExportTemplate::setProgressListener(ProgressListener* progressListener) {
    this->progressListener = progressListener;
}

void ExportTemplate::setExportRange(const PageRangeVector& exportRange) {
    if (exportRange.empty()) {
        ElementRangeVector fullRange;
        fullRange.emplace_back(0, doc->getPageCount() - 1);
        this->exportRange = fullRange;
    } else {
        this->exportRange = exportRange;
    }
}

void ExportTemplate::setExportRange(const char* rangeStr) {
    ElementRangeVector fullRange;
    fullRange.emplace_back(0, doc->getPageCount() - 1);
    exportRange = parseRange(rangeStr).value_or(fullRange);
}

void ExportTemplate::setLayerRange(const LayerRangeVector& layerRange) {
    if (layerRange.empty()) {
        this->layerRange = std::nullopt;
    } else {
        this->layerRange = layerRange;
    }
}

void ExportTemplate::setLayerRange(const char* rangeStr) { layerRange = parseRange(rangeStr); }

auto parseRange(const char* rangeStr) -> std::optional<ElementRangeVector> {
    if (!rangeStr) {
        return std::nullopt;
    }

    // Use no upper bound for layer indices, as the maximal value can vary between pages
    ElementRangeVector v = ElementRange::parse(rangeStr, std::numeric_limits<size_t>::max());

    if (v.empty()) {
        return std::nullopt;
    }

    return v;
}

void ExportTemplate::setProgressiveMode(const bool progressiveMode) { this->progressiveMode = progressiveMode; }

void ExportTemplate::setCropToContent(const bool cropToContent) { this->cropToContent = cropToContent; }

auto ExportTemplate::getLastErrorMsg() const -> std::optional<std::string> { return lastError; }

void ExportTemplate::exportDocument() {
    if (!doc) {
        lastError = _("Error: no document to export.");
        return;
    }

    numberOfPagesToExport = countPagesToExport(exportRange);

    if (progressListener) {
        progressListener->setMaximumState(static_cast<int>(numberOfPagesToExport));
    }

    size_t exportedPages = 0;
    for (const auto& rangeEntry: exportRange) {
        exportedPages += exportPagesInRangeEntry(rangeEntry, exportedPages);
    }
}

auto countPagesToExport(const PageRangeVector& exportRange) -> size_t {
    size_t count = 0;
    for (const auto& e: exportRange) {
        count += e.last - e.first + 1;
    }
    return count;
}

auto ExportTemplate::exportPagesInRangeEntry(const ElementRangeEntry& rangeEntry, const size_t exportedPages)
        -> size_t {
    size_t exportedPagesInEntry = 0;
    auto lastPage = std::min(rangeEntry.last, doc->getPageCount());

    for (auto i = rangeEntry.first; i <= lastPage; ++i, ++exportedPagesInEntry) {
        if (cropToContent) {
            const PageRef page = doc->getPage(i);
            cropRange = calcRangeFromElements(page->getSelectedLayer()->getElements());
        }

        if (progressiveMode) {
            exportPageLayers(i);
        } else {
            exportPage(i);
        }

        if (progressListener) {
            int progressCount = static_cast<int>(exportedPages + exportedPagesInEntry);
            progressListener->setCurrentState(progressCount);
        }
    }

    return exportedPagesInEntry;
}

auto calcRangeFromElements(std::vector<Element*> elements) -> std::optional<Range> {
    if (elements.empty()) {
        return std::nullopt;
    }

    Element* first = elements.front();
    Range range(first->getX(), first->getY());

    for (Element* e: elements) {
        range.addPoint(e->getX(), e->getY());
        range.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
    }

    // Add padding:
    int padding = 1;
    range.addPoint(range.getX() - padding, range.getY() - padding);
    range.addPoint(range.getX2() + padding, range.getY2() + padding);

    return range;
}

void ExportTemplate::exportPageLayers(const size_t pageNo) {
    PageRef page = doc->getPage(pageNo);

    std::map<Layer*, bool> initialVisibility = clearLayerVisibilityStateOfPage(page);

    // We draw as many pages as there are layers. The first page has
    // only Layer 1 visible, the last has all layers visible.
    for (const auto& layer: *page->getLayers()) {
        layer->setVisible(true);
        exportPage(pageNo);
    }

    setLayerVisibilityStateOfPage(page, initialVisibility);
}

auto clearLayerVisibilityStateOfPage(const PageRef& page) -> std::map<Layer*, bool> {
    std::map<Layer*, bool> layerVisibilityState;
    for (const auto& layer: *page->getLayers()) {
        layerVisibilityState[layer] = layer->isVisible();
        layer->setVisible(false);
    }
    return layerVisibilityState;
}

void setLayerVisibilityStateOfPage(const PageRef& page, std::map<Layer*, bool> visibilityState) {
    for (const auto& layer: *page->getLayers()) {
        layer->setVisible(visibilityState[layer]);
    }
}

auto ExportTemplate::exportPage(const size_t pageNo) -> bool {
    PageRef page = doc->getPage(pageNo);

    if (!configureCairoResourcesForPage(pageNo)) {
        return false;
    }

    xoj::util::CairoSurfaceSPtr tmpSurface;
    xoj::util::CairoSPtr tmpCr;
    if (cropRange) {
        // swap out cairo
        Range range = cropRange.value();
        // create new cropped cairo surface
        tmpSurface = cairo_pdf_surface_create(nullptr, page->getWidth(), page->getHeight());
        tmpCr = cairo_create(tmpSurface.get());
        std::swap(surface, tmpSurface);
        std::swap(cr, tmpCr);
    }

    renderBackground(page);
    drawPage(page);

    if (cropRange) {
        Range range = cropRange.value();
        // copy
        cairo_set_source_surface(tmpCr.get(), surface.get(), -range.getX(), -range.getY());
        cairo_rectangle(tmpCr.get(), 0, 0, range.getWidth(), range.getHeight());
        cairo_fill(tmpCr.get());

        // TODO: Avoid blurry output (not working)
        cairo_set_source_rgb(tmpCr.get(), 0, 0, 0);
        cairo_stroke(tmpCr.get());

        // swap in
        std::swap(surface, tmpSurface);
        std::swap(cr, tmpCr);
    }

    clearCairoConfig(pageNo);

    return true;
}

void ExportTemplate::renderBackground(const PageRef& page) {
    // For a better pdf quality, we use a dedicated pdf rendering
    if (page->getBackgroundType().isPdfPage() && (exportBackground != EXPORT_BACKGROUND_NONE)) {
        // Handle the pdf page separately, to call renderForPrinting for better quality.
        auto pgNo = page->getPdfPageNr();
        XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);
        if (!popplerPage) {
            lastError = _("Error while exporting the pdf background: cannot find the pdf page number.");
            lastError.value() += std::to_string(pgNo);
        } else {
            popplerPage->renderForPrinting(cr.get());
        }
    }
}

void ExportTemplate::drawPage(const PageRef& page) {
    DocumentView view;
    bool dontRenderEditingStroke = true;
    bool hidePdfBackground = true;
    bool hideImageBackground = exportBackground == EXPORT_BACKGROUND_NONE;
    bool hideRulingBackground = exportBackground <= EXPORT_BACKGROUND_UNRULED;
    if (layerRange) {
        view.drawLayersOfPage(layerRange.value(), page, cr.get(), dontRenderEditingStroke, hidePdfBackground,
                              hideImageBackground, hideRulingBackground);
    } else {
        view.drawPage(page, cr.get(), dontRenderEditingStroke, hidePdfBackground, hideImageBackground,
                      hideRulingBackground);
    }
}
