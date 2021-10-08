/*
 * Xournal++
 *
 * List of unused colors for toolbar customisation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "gui/XojColor.h"


class CustomizeableColorList {
public:
    CustomizeableColorList();

public:
    const std::vector<XojColor>& getPredefinedColors() const;

private:
    void addPredefinedColor(Color color, std::string name);

private:
    std::vector<XojColor> colors;
};
