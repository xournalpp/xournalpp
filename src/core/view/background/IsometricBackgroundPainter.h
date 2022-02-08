/*
 * Xournal++
 *
 * Draws dotted background
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


class IsometricBackgroundPainter: public BaseBackgroundPainter {
public:
    IsometricBackgroundPainter(bool drawLines);
    ~IsometricBackgroundPainter() override;

public:
    void paint() override;

    /**
     * Reset all used configuration values
     */
    void resetConfig() override;

private:
    bool drawLines;
};
