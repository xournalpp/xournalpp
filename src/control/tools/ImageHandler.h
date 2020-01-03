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

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "XournalType.h"

class Control;
class XojPageView;

class ImageHandler {
public:
    ImageHandler(Control* control, XojPageView* view);
    virtual ~ImageHandler();

public:
    bool insertImage(double x, double y);
    bool insertImage(GFile* file, double x, double y);

private:
    Control* control;
    XojPageView* view;
};
