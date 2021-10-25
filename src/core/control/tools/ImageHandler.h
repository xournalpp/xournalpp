/*
 * Xournal++
 *
 * Image Tool handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

class Control;
class XojPageView;

class ImageHandler final {
public:
    ImageHandler(Control* control, XojPageView* view): control(control), view(view){};

public:
    bool insertImage(double x, double y);
    bool insertImage(GFile* file, double x, double y);

private:
    Control* control;
    XojPageView* view;
};
