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

#include <string>  // for string

#include "util/Color.h"  // for Color


struct XojColor {
public:
    XojColor(Color color, std::string name);

public:
    Color getColor() const;
    std::string getName() const;

private:
    Color color;
    // the localized name of the color
    std::string name;
};
