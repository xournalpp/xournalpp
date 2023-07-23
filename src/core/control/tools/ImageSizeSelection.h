/*
 * Xournal++
 *
 * Select a size before inserting an image
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/OverlayBase.h"                     // for OverlayBase
#include "util/DispatchPool.h"                     // for Listener
#include "util/Rectangle.h"                        // for Rectangle
#include "view/overlays/ImageSizeSelectionView.h"  // for ImageSizeSelectionView

using namespace xoj::view;

class ImageSizeSelection: public OverlayBase {
public:
    ImageSizeSelection(double x, double y);

    void updatePosition(double x, double y);

    void finalize();

    auto getSelectedSpace() const -> xoj::util::Rectangle<double>;

    inline auto getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<ImageSizeSelectionView>>& {
        return viewPool;
    }

private:
    double startX;
    double startY;
    double endX;
    double endY;

    Range box;

    std::shared_ptr<xoj::util::DispatchPool<ImageSizeSelectionView>> viewPool;
};
