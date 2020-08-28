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

#include "util/Color.h"

#include "XournalType.h"

class XojColor {
public:
    XojColor(Color color, string name);
    virtual ~XojColor();

public:
    Color getColor() const;
    string getName();

private:
    Color color;
    // the localized name of the color
    string name;
};
