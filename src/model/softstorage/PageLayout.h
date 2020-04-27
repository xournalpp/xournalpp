//
// Created by julius on 26.04.20.
//

#pragma once

#include "Layout.h"
#include "Viewport.h"

class PageLayout: public Layout {
public:
    enum Mode { FIT_WIDTH, FIT_HEIGHT, FREE };

public:
    PageLayout(std::shared_ptr<Viewport> viewport);


    auto onAction(const Action& action) -> void override;
    auto getDocumentSize() -> const Rectangle<double>& override;
    auto isInfiniteHorizontally() -> bool override;
    auto isInfiniteVertically() -> bool override;
    /**
     * Return the pageview containing co-ordinates.
     */
    auto getViewAt(int x, int y) -> XojPageView*;

    /**
     * Return current page
     */
    auto getCurrentPage() -> PageRef;

private:
    LayoutMapper mapper;

    std::vector<unsigned> widthCols;
    std::vector<unsigned> heightRows;

    /**
     * The width and height of all our pages
     */
    size_t minWidth = 0;
    size_t minHeight = 0;

    Mode mode = FREE;
};
