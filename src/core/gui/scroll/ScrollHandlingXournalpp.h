/*
 * Xournal++
 *
 * Scroll handling for Xournal workaround without standard GTK Scroll handling
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ScrollHandling.h"

class ScrollHandlingXournalpp: public ScrollHandling {
public:
    ScrollHandlingXournalpp();
    ~ScrollHandlingXournalpp() override;

public:
    void setLayoutSize(int width, int height) override;

    int getPreferredWidth() override;
    int getPreferredHeight() override;

    void translate(cairo_t* cr, double& x1, double& x2, double& y1, double& y2) override;
    void translate(double& x, double& y) override;

    bool fullRepaint() override;

    void scrollChanged() override;

private:
};
