#include "SnapToGridInputHandler.h"

#include <cmath>  // for floor, sqrt

#include "control/settings/Settings.h"
#include "model/BackgroundConfig.h"  // for BackgroundConfig
#include "model/Snapping.h"
#include "model/XojPage.h"  // for XojPage

using namespace background_config_strings;

namespace {
constexpr double DEFAULT_RASTER_SIZE = 14.17;  // 5mm
constexpr double RULED_HEADER_SIZE = 80.0;
}  // namespace

SnapToGridInputHandler::SnapToGridInputHandler(const Settings* settings): settings(settings), currentPage(nullptr) {}

void SnapToGridInputHandler::setPageRef(PageRef page) {
    this->currentPage = page;
    this->offsetsCached = false;
}

void SnapToGridInputHandler::ensureOffsetsCached() const {
    if (!currentPage) {
        cachedXOffset = 0.0;
        cachedYOffset = 0.0;
        offsetsCached = true;
        return;
    }
    if (!offsetsCached) {
        calculateGridOffsets(cachedXOffset, cachedYOffset, currentPage->getWidth(), currentPage->getHeight());
        offsetsCached = true;
    }
}

void SnapToGridInputHandler::calculateGraphOffsets(const BackgroundConfig& config, double pageWidth, double pageHeight,
                                                   double& xOffset, double& yOffset) const {
    double margin = 0.0;
    double squareSize = DEFAULT_RASTER_SIZE;
    int roundToGrid = 0;
    int boldLineInterval = 0;

    config.loadValue(CFG_MARGIN, margin);
    config.loadValue(CFG_RASTER, squareSize);
    config.loadValue(CFG_ROUND_MARGIN, roundToGrid);
    config.loadValue(CFG_BOLD_LINE_INTERVAL, boldLineInterval);

    if (margin <= 0.0) {
        return;
    }
    if (roundToGrid) {
        // Centered grid: align with bold line intervals
        const double roundingUnit = (boldLineInterval > 0) ? (squareSize * boldLineInterval) : squareSize;
        const double maxGridWidth = pageWidth - 2.0 * margin;
        const double maxGridHeight = pageHeight - 2.0 * margin;

        const int completeUnitsX = static_cast<int>(std::floor(maxGridWidth / roundingUnit));
        const int completeUnitsY = static_cast<int>(std::floor(maxGridHeight / roundingUnit));

        const double actualGridWidth = completeUnitsX * roundingUnit;
        const double actualGridHeight = completeUnitsY * roundingUnit;

        xOffset = (pageWidth - actualGridWidth) / 2.0;
        yOffset = (pageHeight - actualGridHeight) / 2.0;
    } else {
        xOffset = margin;
        yOffset = margin;
    }
}

void SnapToGridInputHandler::calculateIsometricOffsets(const BackgroundConfig& config, double pageWidth,
                                                       double pageHeight, double& xOffset, double& yOffset) const {
    double triangleSize = DEFAULT_RASTER_SIZE;
    config.loadValue(CFG_RASTER, triangleSize);

    const double xStep = std::sqrt(3.0) / 2.0 * triangleSize;
    const double yStep = triangleSize / 2.0;
    const double isoMargin = triangleSize;

    const int cols = static_cast<int>(std::floor((pageWidth - 2.0 * isoMargin) / xStep));
    const int rows = static_cast<int>(std::floor((pageHeight - 2.0 * isoMargin) / yStep));

    const double contentWidth = cols * xStep;
    const double contentHeight = rows * yStep;

    xOffset = (pageWidth - contentWidth) / 2.0;
    yOffset = (pageHeight - contentHeight) / 2.0;
}

void SnapToGridInputHandler::calculateGridOffsets(double& xOffset, double& yOffset, double pageWidth,
                                                  double pageHeight) const {
    xOffset = 0.0;
    yOffset = 0.0;
    if (!currentPage) {
        return;
    }
    const PageType bgType = currentPage->getBackgroundType();
    const BackgroundConfig config(bgType.config);

    switch (bgType.format) {
        case PageTypeFormat::Graph:
            calculateGraphOffsets(config, pageWidth, pageHeight, xOffset, yOffset);
            break;
        case PageTypeFormat::IsoDotted:
        case PageTypeFormat::IsoGraph:
            calculateIsometricOffsets(config, pageWidth, pageHeight, xOffset, yOffset);
            break;
        case PageTypeFormat::Ruled:
        case PageTypeFormat::Lined:
            yOffset = RULED_HEADER_SIZE;
            break;
        case PageTypeFormat::Dotted:
        case PageTypeFormat::Staves:
        case PageTypeFormat::Plain:
        case PageTypeFormat::Pdf:
        case PageTypeFormat::Image:
        default:
            // Origin-based grid (offsets remain 0.0)
            break;
    }
}

bool SnapToGridInputHandler::isIsometricBackground(double& triangleSize) const {
    if (!currentPage) {
        return false;
    }
    const PageType bgType = currentPage->getBackgroundType();
    if (bgType.format != PageTypeFormat::IsoDotted && bgType.format != PageTypeFormat::IsoGraph) {
        return false;
    }
    const BackgroundConfig config(bgType.config);
    triangleSize = DEFAULT_RASTER_SIZE;
    config.loadValue(CFG_RASTER, triangleSize);
    return true;
}

double SnapToGridInputHandler::snapVertically(double y, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        ensureOffsetsCached();
        return Snapping::snapVertically(y, settings->getSnapGridSize(), settings->getSnapGridTolerance(),
                                        cachedYOffset);
    }
    return y;
}

double SnapToGridInputHandler::snapHorizontally(double x, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        ensureOffsetsCached();
        return Snapping::snapHorizontally(x, settings->getSnapGridSize(), settings->getSnapGridTolerance(),
                                          cachedXOffset);
    }
    return x;
}

Point SnapToGridInputHandler::snapToGrid(Point const& pos, bool alt) const {
    if (alt == settings->isSnapGrid()) {
        return pos;
    }
    ensureOffsetsCached();

    double triangleSize;
    if (isIsometricBackground(triangleSize)) {
        const double xStep = std::sqrt(3.0) / 2.0 * triangleSize;
        const double yStep = triangleSize / 2.0;
        return Snapping::snapToGrid(pos, xStep, yStep, settings->getSnapGridTolerance(), cachedXOffset, cachedYOffset);
    }

    const double gridSize = settings->getSnapGridSize();
    return Snapping::snapToGrid(pos, gridSize, gridSize, settings->getSnapGridTolerance(), cachedXOffset,
                                cachedYOffset);
}

double SnapToGridInputHandler::snapAngle(double radian, bool alt) const {
    if (alt != settings->isSnapRotation()) {
        return Snapping::snapAngle(radian, settings->getSnapRotationTolerance());
    }
    return radian;
}

Point SnapToGridInputHandler::snapRotation(Point const& pos, Point const& center, bool alt) const {
    if (alt != settings->isSnapRotation()) {
        return Snapping::snapRotation(pos, center, settings->getSnapRotationTolerance());
    }
    return pos;
}

Point SnapToGridInputHandler::snap(Point const& pos, Point const& center, bool alt) const {
    Point rotationSnappedPoint{snapRotation(pos, center, alt)};
    return snapToGrid(rotationSnappedPoint, alt);
}
