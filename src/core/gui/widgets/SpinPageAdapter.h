/*
 * Xournal++
 *
 * Handle the Page Spin Widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <list>     // for list

#include <glib.h>     // for guint, gulong
#include <gtk/gtk.h>  // for GtkWidget, GtkSpinButton

class SpinPageListener;

class SpinPageAdapter {
public:
    SpinPageAdapter();
    virtual ~SpinPageAdapter();

public:
    bool hasWidget();

    /**
     * Assumes ownership of widget
     */
    void setWidget(GtkWidget* widget);
    void removeWidget();

    int getPage() const;
    void setPage(size_t page);
    void setMinMaxPage(size_t min, size_t max);

    void addListener(SpinPageListener* listener);
    void removeListener(SpinPageListener* listener);

private:
    static bool pageNrSpinChangedTimerCallback(SpinPageAdapter* adapter);
    static void pageNrSpinChangedCallback(GtkSpinButton* spinbutton, SpinPageAdapter* adapter);

    void firePageChanged();

private:
    GtkWidget* widget = nullptr;
    gulong pageNrSpinChangedHandlerId = 0;
    size_t page = 0;

    guint lastTimeoutId = 0;
    std::list<SpinPageListener*> listener;

    size_t min = 0;
    size_t max = 0;
};

class SpinPageListener {
public:
    virtual void pageChanged(size_t page) = 0;
    virtual ~SpinPageListener();
};
