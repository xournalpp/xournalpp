/*
 * Xournal++
 *
 * Displays a background
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"
#include "view/View.h"

class BackgroundConfig;

namespace xoj {
namespace view {

class BackgroundView {
public:
    BackgroundView(double pageWidth, double pageHeight): pageWidth(pageWidth), pageHeight(pageHeight) {}
    virtual ~BackgroundView() = default;

    /**
     * @brief Draws the background on the entire mask represented by the cairo context cr
     */
    virtual void draw(cairo_t* cr) const = 0;

    static std::unique_ptr<BackgroundView> create(double width, double height, Color backgroundColor,
                                                  const PageType& pt, double lineWidthFactor = 1.0);

protected:
    double pageWidth;
    double pageHeight;
};

class OneColorBackgroundView;
class ImageBackgroundView;
class PlainBackgroundView;
class RuledBackgroundView;
class LinedBackgroundView;
class GraphBackgroundView;
class StavesBackgroundView;
class DottedBackgroundView;
class BaseIsometricBackgroundView;
class IsoGraphBackgroundView;
class IsoDottedBackgroundView;
class PdfBackgroundView;
class TransparentCheckerboardBackgroundView;
};  // namespace view
};  // namespace xoj
