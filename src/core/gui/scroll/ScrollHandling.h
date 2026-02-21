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

#include <gtk/gtk.h>

namespace xoj::util {
template <typename T>
struct Point;
}

class ScrollHandling final {
public:
    ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical);
    ScrollHandling(GtkScrolledWindow* scrollable);
    ~ScrollHandling();

public:
    GtkAdjustment* getHorizontal();
    GtkAdjustment* getVertical();
    xoj::util::Point<double> getPosition() const;

private:
    GtkAdjustment* adjHorizontal = nullptr;
    GtkAdjustment* adjVertical = nullptr;
};
