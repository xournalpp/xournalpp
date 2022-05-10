/*
 * Xournal++
 *
 * Draws lined backgrounds of all sorts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "model/PageType.h"

#include "BaseBackgroundPainter.h"


class LineBackgroundPainter: public BaseBackgroundPainter {
public:
    LineBackgroundPainter(const PageTypeFormat format);
    ~LineBackgroundPainter() override;

public:
    void paint() override;

    /**
     * Reset all used configuration values
     */
    void resetConfig() override;


    void paintBackgroundRuled();
    void paintBackgroundVerticalLine();

private:
    PageTypeFormat format;
};
