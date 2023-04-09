#include "ExportTemplate.h"

#include <cmath>  // for round
#include <utility>

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
        if (progressiveMode) {
            exportPageLayersProgressivelyForPage(i);
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

void ExportTemplate::exportPageLayersProgressivelyForPage(const size_t pageNo) {
    PageRef page = doc->getPage(pageNo);

    std::optional<LayerRangeVector> layerRange = this->layerRange;

    struct isIncluded {
        size_t val;
        isIncluded(size_t const& i): val{i} {}
        auto operator()(const LayerRangeEntry& e) -> bool { return e.first <= val && e.last >= val; }
    };
    LayerRangeVector rangeVector;
    for (size_t i = 0; i < (*page->getLayers()).size(); ++i) {
        if (layerRange && !std::any_of(layerRange.value().begin(), layerRange.value().end(), isIncluded{i})) {
            // there is a layerRange, but layer i is not part of it => don't draw layer i
            continue;
        }
        rangeVector.push_back(ElementRangeEntry(i, i));
        setLayerRange(rangeVector);
        exportPage(pageNo);
    }

    this->layerRange = layerRange;
}

auto ExportTemplate::exportPage(const size_t pageNo) -> bool {
    PageRef page = doc->getPage(pageNo);

    if (!configureCairoResourcesForPage(pageNo)) {
        return false;
    }

    renderBackground(page);
    drawPage(page);

    clearCairoConfig();

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
        } else if (format == EXPORT_GRAPHICS_PNG) {
            popplerPage->render(cr);
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
