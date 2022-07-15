/*
 * Xournal++
 *
 * Template class for export classes (e.g. PDF/Image export)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for std::unique_ptr

#include "util/ElementRange.h"  // for LayerRangeVector

/**
 * @brief Template class for export classes
 */
class ExportTemplate {
public:
    /**
     * @brief Select layers to export by parsing str
     * @param rangeStr A string parsed to get a list of layers
     */
    auto setLayerRange(const char* rangeStr) -> void;

    /**
     * @brief Get the last error message
     * @return The last error message to show to the user
     */
    auto getLastErrorMsg() const -> std::string;

protected:
    /**
     * @brief A pointer to a range of layers to export (the same for every exported pages)
     */
    std::unique_ptr<LayerRangeVector> layerRange;

    /**
     * The last error message to show to the user
     */
    std::string lastError;
};
