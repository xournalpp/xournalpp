/*
 * Xournal++
 *
 * Handles the layout of the pages within a Xournal document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>
#include <model/PageRef.h>

#include "gui/LayoutMapper.h"
#include "model/Storage.h"

#include "LayoutEvent.h"
#include "PageView.h"
#include "Rectangle.h"
#include "Viewport.h"
#include "XournalType.h"

/**
 * @brief The Layout manager for the XournalWidget
 *
 * This class manages the layout of the XojPageView's contained
 * in the XournalWidget
 */
class Layout: public Storage<LayoutEvent> {
public:
    enum Mode { FIT_WIDTH, FIT_HEIGHT, FREE };

public:
    Layout(std::shared_ptr<Viewport> viewport);

public:
    auto getDocumentSize() -> const Rectangle<double>&;
    auto isInfiniteHorizontally() -> bool;
    auto isInfiniteVertically() -> bool;

    auto onAction(const Action& action) -> void override;

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
};
