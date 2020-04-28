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
#include "gui/PageView.h"
#include "model/Storage.h"

#include "Rectangle.h"
#include "Viewport.h"
#include "XournalType.h"

class LayoutEvent {};

/**
 * @brief The Layout manager for the XournalWidget
 *
 * This class manages the layout of the XojPageView's contained
 * in the XournalWidget
 */
class Layout: public Storage<LayoutEvent> {
public:
    virtual ~Layout();

public:
    virtual auto getDocumentSize() -> const Rectangle<double>& = 0;
    virtual auto isInfiniteHorizontally() -> bool = 0;
    virtual auto isInfiniteVertically() -> bool = 0;
};
