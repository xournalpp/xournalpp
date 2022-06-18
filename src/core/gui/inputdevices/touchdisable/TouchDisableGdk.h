/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gtk/gtk.h"  // for GtkWidget

#include "TouchDisableInterface.h"  // for TouchDisableInterface


class TouchDisableGdk: public TouchDisableInterface {
public:
    explicit TouchDisableGdk(GtkWidget* widget);
    ~TouchDisableGdk() override;

public:
    void enableTouch() override;
    void disableTouch() override;
    void init() override;

private:
    GtkWidget* widget;
};
