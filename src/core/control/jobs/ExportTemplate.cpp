#include "ExportTemplate.h"

#include <cmath>  // for round
#include <utility>

#include "control/jobs/ProgressListener.h"  // for ProgressListener

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
