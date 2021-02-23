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

#include "XournalType.h"

class CustomizeableColorList {
public:
    CustomizeableColorList();

public:
    const vector<XojColor>& getPredefinedColors() const;

private:
    void addPredefinedColor(Color color, string name);

private:
    vector<XojColor> colors;
};
