#include "SnapToGridInputHandler.h"

#include <cmath>  // for floor, sqrt

#include "control/settings/Settings.h"
#include "model/BackgroundConfig.h"  // for BackgroundConfig
#include "model/Snapping.h"
#include "model/XojPage.h"  // for XojPage

using namespace background_config_strings;

SnapToGridInputHandler::SnapToGridInputHandler(const Settings* settings): settings(settings), currentPage(nullptr) {}

void SnapToGridInputHandler::setPageRef(PageRef page) {
    this->currentPage = page;
    this->offsetsCached = false;  // Invalidate cache when page changes
}

void SnapToGridInputHandler::ensureOffsetsCached() const {
    if (!currentPage) {
        // Defensive: if page is null, ensure cache is reset to defaults
        cachedXOffset = 0.0;
        cachedYOffset = 0.0;
        offsetsCached = true;  // Mark as cached to avoid repeated checks
        return;
    }

    if (!offsetsCached) {
        calculateGridOffsets(cachedXOffset, cachedYOffset, currentPage->getWidth(), currentPage->getHeight());
        offsetsCached = true;
    }
}

void SnapToGridInputHandler::calculateGridOffsets(double& xOffset, double& yOffset, double pageWidth,
                                                  double pageHeight) const {
    // Initialize offsets to zero (origin-based grid by default)
    xOffset = 0.0;
    yOffset = 0.0;

    if (!currentPage) {
        return;
    }

    const PageType bgType = currentPage->getBackgroundType();
    const BackgroundConfig config(bgType.config);

    // Constants for background-specific calculations
    constexpr double DEFAULT_RASTER_SIZE = 14.17;  // 5mm in points (1/72 inch)
    constexpr double RULED_HEADER_SIZE = 80.0;     // Standard header spacing for ruled paper

    switch (bgType.format) {
        case PageTypeFormat::Graph: {
            // Graph paper: square grid with optional margins and bold line intervals
            double margin = 0.0;
            double squareSize = DEFAULT_RASTER_SIZE;
            int roundToGrid = 0;
            int boldLineInterval = 0;

            config.loadValue(CFG_MARGIN, margin);
            config.loadValue(CFG_RASTER, squareSize);
            config.loadValue(CFG_ROUND_MARGIN, roundToGrid);
            config.loadValue(CFG_BOLD_LINE_INTERVAL, boldLineInterval);

            if (margin > 0.0) {
                if (roundToGrid) {
                    // Centered grid: adjust margin to align grid with bold line intervals
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
                    // Simple fixed margin
                    xOffset = margin;
                    yOffset = margin;
                }
            }
            break;
        }

        case PageTypeFormat::Dotted: {
            // Dotted paper: simple square grid at origin (no margins)
            // Offsets remain 0.0 - grid starts at page origin
            break;
        }

        case PageTypeFormat::IsoDotted:
        case PageTypeFormat::IsoGraph: {
            // Isometric paper: triangular grid pattern, automatically centered on page
            double triangleSize = DEFAULT_RASTER_SIZE;
            config.loadValue(CFG_RASTER, triangleSize);

            // Isometric grid geometry
            const double xStep = std::sqrt(3.0) / 2.0 * triangleSize;  // Horizontal column spacing
            const double yStep = triangleSize / 2.0;                   // Vertical row spacing

            // Calculate grid size (matching BaseIsometricBackgroundView logic)
            const double isoMargin = triangleSize;
            const int cols = static_cast<int>(std::floor((pageWidth - 2.0 * isoMargin) / xStep));
            const int rows = static_cast<int>(std::floor((pageHeight - 2.0 * isoMargin) / yStep));

            const double contentWidth = cols * xStep;
            const double contentHeight = rows * yStep;

            // Center the grid on the page
            xOffset = (pageWidth - contentWidth) / 2.0;
            yOffset = (pageHeight - contentHeight) / 2.0;
            break;
        }

        case PageTypeFormat::Ruled:
        case PageTypeFormat::Lined: {
            // Ruled/Lined paper: horizontal lines with header spacing
            // Apply vertical offset only (snap to horizontal line positions)
            yOffset = RULED_HEADER_SIZE;
            // xOffset remains 0.0 - lines span full page width
            break;
        }

        case PageTypeFormat::Staves:
        case PageTypeFormat::Plain:
        case PageTypeFormat::Pdf:
        case PageTypeFormat::Image:
        default:
            // No special grid snapping for these background types
            // Use default origin-based rectangular grid (offsets remain 0.0)
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

    // Load triangle size from config
    const BackgroundConfig config(bgType.config);
    triangleSize = 14.17;  // Default 5mm
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
        // Snapping is disabled (either explicitly off, or alt-toggled off)
        return pos;
    }

    ensureOffsetsCached();

    // Use isometric snapping for isometric backgrounds, rectangular snapping for others
    double triangleSize;
    if (isIsometricBackground(triangleSize)) {
        return Snapping::snapToIsometricGrid(pos, triangleSize, settings->getSnapGridTolerance(), cachedXOffset,
                                             cachedYOffset);
    }

    return Snapping::snapToGrid(pos, settings->getSnapGridSize(), settings->getSnapGridTolerance(), cachedXOffset,
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
