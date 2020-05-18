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
#include "XournalType.h"

class IsometricBackgroundPainter: public BaseBackgroundPainter {
public:
    IsometricBackgroundPainter(bool drawLines);
    virtual ~IsometricBackgroundPainter() override;

public:
    virtual void paint() override;

    /**
     * Reset all used configuration values
     */
    virtual void resetConfig() override;

private:
    bool drawLines;
};
