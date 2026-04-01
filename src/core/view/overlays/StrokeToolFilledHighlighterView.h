/*
 * Xournal++
 *
 * View active stroke tool -- for filled highlighter only
 *      In this case, the mask needs to be wiped at every iteration, and repainted to avoid artefacts like in
 *          https://github.com/xournalpp/xournalpp/issues/3709
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "StrokeToolFilledView.h"

namespace xoj::view {
class StrokeToolFilledHighlighterView: public StrokeToolFilledView {
public:
    StrokeToolFilledHighlighterView(const StrokeHandler* strokeHandler, const Stroke& stroke, Repaintable* parent);
    virtual ~StrokeToolFilledHighlighterView() noexcept;

    void draw(cairo_t* cr) const override;
};
};  // namespace xoj::view
