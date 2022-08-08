#include "ExportTemplate.h"

#include <cmath>  // for round
#include <utility>

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "model/Document.h"                 // for Document

ExportTemplate::ExportTemplate(Document* doc, ExportBackgroundType exportBackground, ProgressListener* progressListener,
                               fs::path filePath, const PageRangeVector& exportRange):
        exportRange{exportRange},
        doc{doc},
        exportBackground{exportBackground},
        progressListener{progressListener},
        filePath{std::move(filePath)} {}

ExportTemplate::~ExportTemplate() {}

auto ExportTemplate::setLayerRange(const char* rangeStr) -> void {
    if (rangeStr) {
        // Use no upper bound for layer indices, as the maximal value can vary between pages
        layerRange =
                std::make_unique<LayerRangeVector>(ElementRange::parse(rangeStr, std::numeric_limits<size_t>::max()));
    }
}

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
            exportPage(i);

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
