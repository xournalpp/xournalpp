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
