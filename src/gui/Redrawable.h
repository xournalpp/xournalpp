/*
 * Xournal++
 *
 * Interface for GUI handling
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "Range.h"

class Element;

class Redrawable {
protected:
    Redrawable() = default;

public:
    Redrawable(Redrawable&&) = default;
    Redrawable(Redrawable const&) = default;
    virtual ~Redrawable() = default;
    Redrawable& operator=(Redrawable&&) = default;
    Redrawable& operator=(Redrawable const&) = default;

    /**
     * Call this if you only need to repaint the view, this means the buffer will be painted again,
     * and all selections, text editors etc. are drawn again, but the view buffer is not refreshed.
     *
     * for refreshing the view buffer (if you have changed the document) call rerender.
     */
    virtual void repaintArea(double x1, double y1, double x2, double y2) = 0;
    void repaintRect(double x, double y, double width, double height);
    void repaintRange(Range& r);
    void repaintElement(Element* e);

    /**
     * Call this if you only need to readraw the view, this means the buffer will be painted again,
     * and all selections, text edtiors etc. are drawn again, but the view buffer is not refreshed.
     *
     * for refreshing the view buffer (if you have changed the document) call repaint.
     */
    virtual void repaintPage() = 0;

    /**
     * Repaint our buffer, then redraw the widget
     */
    virtual void rerenderPage() = 0;

    /**
     * Call this if you add an element, remove an element etc.
     */
    void rerenderElement(Element* e);
    void rerenderRange(Range& r);

    /**
     * This updated the view buffer and then rerender the the region, call this if you changed the document
     */
    virtual void rerenderRect(double x, double y, double width, double height) = 0;
    void rerenderArea(double x1, double y1, double x2, double y2);

    /**
     * Return the GTK selection color
     */
    virtual GdkRGBA getSelectionColor() = 0;


    virtual void deleteViewBuffer() = 0;

    virtual int getX() const = 0;
    virtual int getY() const = 0;
};
