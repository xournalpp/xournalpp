/*
 * Xournal++
 *
 * Class for transparent backgrounds (checkerboard pattern)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_pattern_t, cairo_t

#include "util/Color.h"  // for Color

#include "BackgroundView.h"  // for BackgroundView

namespace xoj::view {
class TransparentCheckerboardBackgroundView: public BackgroundView {
public:
    TransparentCheckerboardBackgroundView(double pageWidth, double pageHeight);
    virtual ~TransparentCheckerboardBackgroundView() noexcept;

    void draw(cairo_t* cr) const override;

private:
    cairo_pattern_t* pattern = nullptr;

protected:
    static cairo_pattern_t* createPattern();
    static constexpr int CHECKER_SIZE = 8;
    static constexpr Color DARK_GREY{Colors::gray};
    static constexpr Color LIGHT_GREY{Colors::silver};
};
};  // namespace xoj::view
