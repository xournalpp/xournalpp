/*
 * Xournal++
 *
 * Draws graph backgrounds of all sorts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "BaseBackgroundPainter.h"
#include "XournalType.h"

class GraphBackgroundPainter: public BaseBackgroundPainter {
public:
    GraphBackgroundPainter();
    virtual ~GraphBackgroundPainter();

public:
    virtual void paint();
    void paintBackgroundGraph();

    /**
     * Reset all used configuration values
     */
    virtual void resetConfig();

    double getUnitSize();

private:
    void updateGraphColor();
};
