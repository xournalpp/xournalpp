/*
 * Xournal++
 *
 * Scroll handling for different scroll implementations
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkAdjustment, GtkWidget, GtkScrollable

class Layout;

class ScrollHandling {
public:
    ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical);
    ScrollHandling(GtkScrollable* scrollable);
    ~ScrollHandling();

public:
    GtkAdjustment* getHorizontal();
    GtkAdjustment* getVertical();

    void init(GtkWidget* xournal, Layout* layout);

    void setLayoutSize(int width, int height);

private:
protected:
    GtkAdjustment* adjHorizontal = nullptr;
    GtkAdjustment* adjVertical = nullptr;

    GtkWidget* xournal = nullptr;
    Layout* layout = nullptr;
};
