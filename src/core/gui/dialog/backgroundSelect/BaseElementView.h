/*
 * Xournal++
 *
 * Base class for Background selection dialog entry
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>    // for cairo_t, cairo_surface_t
#include <gdk/gdk.h>  // for GdkEventButton
#include <glib.h>     // for gboolean
#include <gtk/gtk.h>  // for GtkWidget

#include "util/Util.h"  // for npos

class BackgroundSelectDialogBase;

class BaseElementView {
public:
    BaseElementView(size_t id, BackgroundSelectDialogBase* dlg);
    virtual ~BaseElementView();

public:
    GtkWidget* getWidget();
    int getWidth();
    int getHeight();

    /**
     * Select / unselect this entry
     */
    void setSelected(bool selected);

    /**
     * Repaint this widget
     */
    void repaint();

protected:
    /**
     * Apply the size to the Widget
     */
    void updateSize();

    /**
     * Paint the whole widget
     */
    void paint(cairo_t* cr);

    /**
     * Paint the contents (without border / selection)
     */
    virtual void paintContents(cairo_t* cr) = 0;

    /**
     * Get the width in pixel, without shadow / border
     */
    virtual int getContentWidth() = 0;

    /**
     * Get the height in pixel, without shadow / border
     */
    virtual int getContentHeight() = 0;

    /**
     * Will be called before getContentWidth() / getContentHeight(), can be overwritten
     */
    virtual void calcSize();

protected:
    BackgroundSelectDialogBase* dlg;

private:
    /**
     * Element ID
     */
    size_t id = npos;

    bool selected = false;

    GtkWidget* widget = nullptr;
    cairo_surface_t* crBuffer = nullptr;
};
