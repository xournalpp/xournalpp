#include "GraphBackgroundView.h"

#include <algorithm>  // for max, min
#include <cmath>      // for floor
#include <memory>     // for allocator

#include "model/BackgroundConfig.h"                  // for BackgroundConfig
#include "view/background/BackgroundView.h"          // for view
#include "view/background/OneColorBackgroundView.h"  // for OneColorBackgrou...
#include "view/background/PlainBackgroundView.h"     // for PlainBackgroundView

using namespace background_config_strings;
using namespace xoj::view;

GraphBackgroundView::GraphBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                         const BackgroundConfig& config):
        OneColorBackgroundView(pageWidth, pageHeight, backgroundColor, config, DEFAULT_LINE_WIDTH, DEFAULT_LINE_COLOR,
                               ALT_DEFAULT_LINE_COLOR) {

    config.loadValue(CFG_RASTER, squareSize);
    config.loadValue(CFG_MARGIN, margin);
    config.loadValue(CFG_BOLD_LINE_INTERVAL, boldLineInterval);
    config.loadValue(CFG_BOLD_LINE_WIDTH, boldLineWidth);

    if (int roundToGrid = 0; margin > 0.0 && config.loadValue(CFG_ROUND_MARGIN, roundToGrid) && roundToGrid) {
        roundUpMargin = true;
    }
}

void GraphBackgroundView::draw(cairo_t* cr) const {
    PlainBackgroundView::draw(cr);

    double minX;
    double maxX;
    double minY;
    double maxY;
    cairo_clip_extents(cr, &minX, &minY, &maxX, &maxY);

    double effectiveMarginX = margin;
    double effectiveMarginY = margin;

    if (roundUpMargin && margin > 0.0) {
        double roundingUnit = (boldLineInterval > 0) ? (squareSize * boldLineInterval) : squareSize;

        double maxGridWidth = pageWidth - 2 * margin;
        double maxGridHeight = pageHeight - 2 * margin;

        int completeUnitsX = static_cast<int>(std::floor(maxGridWidth / roundingUnit));
        int completeUnitsY = static_cast<int>(std::floor(maxGridHeight / roundingUnit));

        double actualGridWidth = completeUnitsX * roundingUnit;
        double actualGridHeight = completeUnitsY * roundingUnit;

        effectiveMarginX = (pageWidth - actualGridWidth) / 2.0;
        effectiveMarginY = (pageHeight - actualGridHeight) / 2.0;
    }

    if (margin > 0.0 || roundUpMargin) {
        minX = std::max(minX, effectiveMarginX);
        maxX = std::min(maxX, pageWidth - effectiveMarginX);
        minY = std::max(minY, effectiveMarginY);
        maxY = std::min(maxY, pageHeight - effectiveMarginY);
    }

    const double halfLineWidth = 0.5 * (boldLineInterval > 0 ? boldLineWidth : lineWidth);

    int indexMinX = static_cast<int>(std::ceil((minX - halfLineWidth - effectiveMarginX) / squareSize));
    int indexMaxX = static_cast<int>(std::floor((maxX + halfLineWidth - effectiveMarginX) / squareSize));
    int indexMinY = static_cast<int>(std::ceil((minY - halfLineWidth - effectiveMarginY) / squareSize));
    int indexMaxY = static_cast<int>(std::floor((maxY + halfLineWidth - effectiveMarginY) / squareSize));

    indexMinX = std::max(0, indexMinX);
    indexMinY = std::max(0, indexMinY);

    cairo_save(cr);
    Util::cairo_set_source_rgbi(cr, foregroundColor);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);

    if (boldLineInterval > 0) {
        cairo_set_line_width(cr, lineWidth);
        if (minY < maxY) {
            for (int i = indexMinX; i <= indexMaxX; ++i) {
                if (i % boldLineInterval != 0) {
                    double x = effectiveMarginX + i * squareSize;
                    cairo_move_to(cr, x, minY);
                    cairo_line_to(cr, x, maxY);
                }
            }
        }
        if (minX < maxX) {
            for (int i = indexMinY; i <= indexMaxY; ++i) {
                if (i % boldLineInterval != 0) {
                    double y = effectiveMarginY + i * squareSize;
                    cairo_move_to(cr, minX, y);
                    cairo_line_to(cr, maxX, y);
                }
            }
        }
        cairo_stroke(cr);

        cairo_set_line_width(cr, boldLineWidth);
        if (minY < maxY) {
            for (int i = indexMinX; i <= indexMaxX; ++i) {
                if (i % boldLineInterval == 0) {
                    double x = effectiveMarginX + i * squareSize;
                    cairo_move_to(cr, x, minY);
                    cairo_line_to(cr, x, maxY);
                }
            }
        }
        if (minX < maxX) {
            for (int i = indexMinY; i <= indexMaxY; ++i) {
                if (i % boldLineInterval == 0) {
                    double y = effectiveMarginY + i * squareSize;
                    cairo_move_to(cr, minX, y);
                    cairo_line_to(cr, maxX, y);
                }
            }
        }
        cairo_stroke(cr);
    } else {
        cairo_set_line_width(cr, lineWidth);
        if (minY < maxY) {
            for (int i = indexMinX; i <= indexMaxX; ++i) {
                double x = effectiveMarginX + i * squareSize;
                cairo_move_to(cr, x, minY);
                cairo_line_to(cr, x, maxY);
            }
        }
        if (minX < maxX) {
            for (int i = indexMinY; i <= indexMaxY; ++i) {
                double y = effectiveMarginY + i * squareSize;
                cairo_move_to(cr, minX, y);
                cairo_line_to(cr, maxX, y);
            }
        }
        cairo_stroke(cr);
    }

    cairo_restore(cr);
}
