/*
 * Xournal++
 *
 * Draw setsquare
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "gui/PageView.h"
#include "model/Setsquare.h"

class Setsquare;

class SetsquareView {
public:
    SetsquareView(XojPageView* view, Setsquare* s);
    ~SetsquareView() = default;

public:
    void paint(cairo_t* cr);
    void move(double x, double y);
    void rotate(double da, double cx, double cy);
    void scale(double f);
    void setView(XojPageView* view);
    XojPageView* getView() const;
    PageRef getPage() const;

private:
private:
    XojPageView* view;
    cairo_t* cr;
    Setsquare* s;
};
