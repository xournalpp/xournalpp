#include "ExportTemplate.h"

#include <cmath>  // for round
#include <map>    // for map
#include <utility>

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "model/Document.h"                 // for Document
#include "model/Layer.h"                    // for Layer
#include "model/XojPage.h"                  // for XojPage
#include "util/i18n.h"                      // for _
#include "view/DocumentView.h"              // for DocumentView

ExportTemplate::ExportTemplate(Document* doc, fs::path filePath): doc{doc}, filePath{std::move(filePath)} {}

ExportTemplate::~ExportTemplate() {}

auto ExportTemplate::setLayerRange(const char* rangeStr) -> void {
    if (rangeStr) {
        // Use no upper bound for layer indices, as the maximal value can vary between pages
        layerRange =
                std::make_unique<LayerRangeVector>(ElementRange::parse(rangeStr, std::numeric_limits<size_t>::max()));
    }
}

void ExportTemplate::setExportBackground(const ExportBackgroundType exportBackground) {
    this->exportBackground = exportBackground;
}

void ExportTemplate::setProgressListener(ProgressListener* progressListener) {
    this->progressListener = progressListener;
}

void ExportTemplate::setExportRange(const PageRangeVector& exportRange) { this->exportRange = exportRange; }

void ExportTemplate::setProgressiveMode(const bool progressiveMode) { this->progressiveMode = progressiveMode; }

auto ExportTemplate::getLastErrorMsg() const -> std::string { return lastError; }

auto ExportTemplate::freeCairoResources() -> bool {
    cairo_destroy(cr);
    cr = nullptr;

    cairo_surface_destroy(surface);
    surface = nullptr;

    return true;
}

auto ExportTemplate::exportDocument() -> bool {
    if (progressListener) {
        size_t n = countPagesToExport(exportRange);
        progressListener->setMaximumState(static_cast<int>(n));
    }

    size_t exportedPages = 0;
    for (const auto& rangeEntry: exportRange) {
        auto lastPage = std::min(rangeEntry.last, doc->getPageCount());

        for (auto i = rangeEntry.first; i <= lastPage; ++i, ++exportedPages) {
            if (progressiveMode && !exportPageLayers(i)) {
                return false;
            }
            if (!progressiveMode && !exportPage(i)) {
                return false;
            }

            if (progressListener) {
                progressListener->setCurrentState(static_cast<int>(exportedPages));
            }
        }
    }

    return true;
}

auto countPagesToExport(const PageRangeVector& exportRange) -> size_t {
    size_t count = 0;
    for (const auto& e: exportRange) {
        count += e.last - e.first + 1;
    }
    return count;
}

auto ExportTemplate::exportPageLayers(const size_t pageNo) -> bool {
    PageRef p = doc->getPage(pageNo);

    // We keep a copy of the layers initial Visible state
    std::map<Layer*, bool> initialVisibility;
    for (const auto& layer: *p->getLayers()) {
        initialVisibility[layer] = layer->isVisible();
        layer->setVisible(false);
    }

    // We draw as many pages as there are layers. The first page has
    // only Layer 1 visible, the last has all layers visible.
    for (const auto& layer: *p->getLayers()) {
        layer->setVisible(true);
        if (!exportPage(pageNo)) {
            return false;
        }
    }

    // We restore the initial visibilities
    for (const auto& layer: *p->getLayers()) {
        layer->setVisible(initialVisibility[layer]);
    }

    return true;
}

auto ExportTemplate::exportPage(const size_t pageNo) -> bool {
    PageRef page = doc->getPage(pageNo);  // TODO: lock or don't lock?

    if (!configureCairoResourcesForPage(page)) {
        return false;
    }

    DocumentView view;

    // For a better pdf quality, we use a dedicated pdf rendering
    if (page->getBackgroundType().isPdfPage() && (exportBackground != EXPORT_BACKGROUND_NONE)) {
        // Handle the pdf page separately, to call renderForPrinting for better
        // quality.
        auto pgNo = page->getPdfPageNr();
        XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);
        if (!popplerPage) {
            lastError = _("Error while exporting the pdf background: cannot find the pdf page number.");
            lastError += std::to_string(pgNo);
            // TODO return? Problem free CairoResources
        } else if (format == EXPORT_GRAPHICS_PNG) {
            popplerPage->render(cr);
        } else {
            popplerPage->renderForPrinting(cr);
        }
    }

    bool dontRenderEraseable = true;  // TODO rename
    bool dontRenderPdfBackground = true;
    if (layerRange) {
        view.drawLayersOfPage(*layerRange, page, cr, dontRenderEraseable, dontRenderPdfBackground,
                              exportBackground == EXPORT_BACKGROUND_NONE,
                              exportBackground <= EXPORT_BACKGROUND_UNRULED);
    } else {
        view.drawPage(page, cr, dontRenderEraseable, dontRenderPdfBackground,
                      exportBackground == EXPORT_BACKGROUND_NONE, exportBackground <= EXPORT_BACKGROUND_UNRULED);
    }

    if (!clearCairoConfig()) {
        return false;
    }

    return true;
}
