/*
 * Xournal++
 *
 * Definition for StrokeStyles
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "LineStyle.h"  // for LineStyle

class StrokeStyle {
private:
    StrokeStyle();
    virtual ~StrokeStyle();

public:
    static LineStyle parseStyle(const char* style);
    static std::string formatStyle(const double* dashes, int count);
    static std::string formatStyle(const LineStyle& style);

public:
};
