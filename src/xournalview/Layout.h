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

#include "Rectangle.h"

struct Layout {
    double documentWidth;
    double documentHeight;
    bool infiniteHorizontally;
    bool infiniteVertically;
};
