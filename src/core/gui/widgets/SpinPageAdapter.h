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
#include <string>   // for string
#include <vector>   // for vector

#include <glib.h>     // for guint, gulong
#include <gtk/gtk.h>  // for GtkWidget, GtkSpinButton

#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr
#include "util/raii/GSourceURef.h"

class SpinPageListener;

class SpinPageAdapter final {
public:
    SpinPageAdapter() = default;
    ~SpinPageAdapter() = default;

public:
    /**
     * Assumes ownership of widget
     */
    void setWidget(GtkWidget* widget);

    size_t getPage() const;
    void setPage(size_t page);
    void setMinMaxPage(size_t min, size_t max);

    void addListener(SpinPageListener* listener);
    void removeListener(SpinPageListener* listener);

    void setLabels(std::vector<std::string> labels);
    void insertLabel(size_t pos, std::string label);
    void deleteLabel(size_t pos);
    void swapLabels(size_t a, size_t b);
    bool hasAnyLabels() const;

private:
    static bool pageNrSpinChangedTimerCallback(SpinPageAdapter* adapter);
    static void pageNrSpinChangedCallback(GtkSpinButton* spinbutton, SpinPageAdapter* adapter);
    static gboolean spinOutputCallback(GtkSpinButton* spin, SpinPageAdapter* adapter);
    static gint spinInputCallback(GtkSpinButton* spin, gdouble* newValue, SpinPageAdapter* adapter);

    void firePageChanged();

private:
    xoj::util::WidgetSPtr widget;
    gulong pageNrSpinChangedHandlerId = 0;
    gulong outputHandlerId = 0;
    gulong inputHandlerId = 0;
    size_t page = 0;

    xoj::util::GSourceURef timeout;
    SpinPageListener* listener;

    size_t min = 0;
    size_t max = 0;

    std::vector<std::string> labels;
};

class SpinPageListener {
public:
    virtual void pageChanged(size_t page) = 0;
    virtual ~SpinPageListener();
};
