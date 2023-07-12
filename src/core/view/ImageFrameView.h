/*
 * Xournal++
 *
 * Displays an ImageFrame Element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "View.h"  // for ElementView

class ImageFrame;

namespace xoj::view {

class ImageFrameView: public ElementView {
public:
    explicit ImageFrameView(const ImageFrame* imgFrame);
    ~ImageFrameView() override;

    /**
     * Draws an ImageFrame model to a cairo surface
     */
    void draw(const Context& ctx) const override;

private:
    const ImageFrame* imageFrame;
};

};  // namespace xoj::view
