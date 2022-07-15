#include "ExportTemplate.h"

#include <cmath>  // for round

auto ExportTemplate::setLayerRange(const char* rangeStr) -> void {
    if (rangeStr) {
        // Use no upper bound for layer indices, as the maximal value can vary between pages
        layerRange =
                std::make_unique<LayerRangeVector>(ElementRange::parse(rangeStr, std::numeric_limits<size_t>::max()));
    }
}
