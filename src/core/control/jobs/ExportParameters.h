/*
 * Xournal++
 *
 * Export parameters
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "pdf/base/PdfExportBackend.h"
#include "util/ElementRange.h"  // for PageRangeVector, LayerRangeVector

enum ExportFormat {
    EXPORT_GRAPHICS_UNDEFINED,
    EXPORT_GRAPHICS_PDF,
    EXPORT_GRAPHICS_PNG,
    EXPORT_GRAPHICS_SVG,
    EXPORT_XOJ
};

/**
 * @brief List of available criterion for determining a PNG export quality.
 * The order must agree with the corresponding listAvailableCriterion in ui/exportSettings.glade
 */
enum ExportQualityCriterion { EXPORT_QUALITY_DPI, EXPORT_QUALITY_WIDTH, EXPORT_QUALITY_HEIGHT };

/**
 *  @brief List of types for the export of background components.
 *  The order must agree with the corresponding listBackgroundType in ui/exportSettings.glade.
 *  It is constructed so that one can check for intermediate types using comparison.
 */
enum ExportBackgroundType { EXPORT_BACKGROUND_NONE, EXPORT_BACKGROUND_UNRULED, EXPORT_BACKGROUND_ALL };

/**
 * @brief A class storing the available quality parameters for PNG export
 */
class RasterImageQualityParameter {
public:
    constexpr RasterImageQualityParameter(ExportQualityCriterion criterion, int value):
            qualityCriterion(criterion), value(value) {}
    RasterImageQualityParameter() = default;
    ~RasterImageQualityParameter() = default;

    /**
     * @brief Get the quality criterion of this parameter
     * @return The quality criterion
     */
    constexpr ExportQualityCriterion getQualityCriterion() const { return qualityCriterion; }

    /**
     * @brief Get the target value of this parameter
     * @return The target value
     */
    constexpr int getValue() const { return value; }

private:
    /**
     * @brief Default quality is: DPI=300
     */
    ExportQualityCriterion qualityCriterion = EXPORT_QUALITY_DPI;
    int value = 300;
};

struct ExportParameters {
    bool progressiveMode;
    ExportBackgroundType backgroundType;
    PdfExportBackend pdfExportBackend;
    RasterImageQualityParameter qualityParameter;
    PageRangeVector pageRanges;
    ExportFormat format;
};
