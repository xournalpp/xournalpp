/*
 * Xournal++
 *
 * A color to select in the toolbar
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "XournalType.h"

class XojColor {
public:
    XojColor(int color, string name);
    virtual ~XojColor();

public:
    int getColor() const;
    string getName();

private:
    int color;
    // the localized name of the color
    string name;
};
