/*
 * Xournal++
 *
 * Class for plain backgrounds
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/Color.h"

#include "BackgroundView.h"

class BackgroundConfig;
namespace xoj::view {
class PlainBackgroundView: public BackgroundView {
public:
    PlainBackgroundView(double pageWidth, double pageHeight, Color backgroundColor);
    virtual ~PlainBackgroundView() = default;

    virtual void draw(cairo_t* cr) const override;

protected:
    Color backgroundColor;

    static std::pair<int, int> getIndexBounds(double min, double max, double step, double margin, double length);
};
};  // namespace xoj::view
