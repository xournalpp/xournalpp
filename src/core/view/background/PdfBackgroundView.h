/*
 * Xournal++
 *
 * Displays a pdf background
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t

#include <cairo.h>  // for cairo_t

#include "BackgroundView.h"  // for BackgroundView

class PdfCache;

namespace xoj {
namespace view {

class PdfBackgroundView: public BackgroundView {
public:
    PdfBackgroundView(double pageWidth, double pageHeight, size_t pageNo, PdfCache* pdfCache = nullptr);
    virtual ~PdfBackgroundView() = default;

    /**
     * @brief Draws the background on the entire mask represented by the cairo context cr
     */
    void draw(cairo_t* cr) const override;

private:
    size_t pageNo;
    PdfCache* pdfCache = nullptr;
};

};  // namespace view
};  // namespace xoj
