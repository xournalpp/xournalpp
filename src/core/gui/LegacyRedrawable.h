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

#include <gdk/gdk.h>  // for GdkRGBA

class Element;
class Range;

/*
 * The (base) class Redrawable (now LegacyRedrawable) is deprecated. Use xoj::view::Repaintable instead.
 *
 * Reasons for the deprecation:
 *
 *  * It is basically used synonymously to XojPageView.
 *  * More importantly, the coexistence of Redrawable::repaint... and Redrawable::rerender... is bothersome.
 *    it is not clear (without diving in the code) what the difference should be.
 *    (repaint = ask gtk to call us back to blitt the buffer, rerender = change the buffer's content)
 *    The rerender... methods should not exist. The derived class should be told what happened (something changed),
 *    and decide by itself if the buffer should be updated or not (if there is a buffer to begin with).
 */
class [[deprecated]] LegacyRedrawable {
protected:
    LegacyRedrawable() = default;

public:
    LegacyRedrawable(LegacyRedrawable&&) = default;
    LegacyRedrawable(LegacyRedrawable const&) = default;
    virtual ~LegacyRedrawable() = default;
    LegacyRedrawable& operator=(LegacyRedrawable&&) = default;
    LegacyRedrawable& operator=(LegacyRedrawable const&) = default;

    /**
     * Call this if you only need to repaint the view, this means the buffer will be painted again,
     * and all selections, text editors etc. are drawn again, but the view buffer is not refreshed.
     *
     * for refreshing the view buffer (if you have changed the document) call rerender.
     */
    virtual void repaintArea(double x1, double y1, double x2, double y2) const = 0;
    [[maybe_unused]] void repaintRect(double x, double y, double width, double height) const;
    [[maybe_unused]] void repaintElement(Element* e) const;

    /**
     * Call this if you only need to readraw the view, this means the buffer will be painted again,
     * and all selections, text edtiors etc. are drawn again, but the view buffer is not refreshed.
     *
     * for refreshing the view buffer (if you have changed the document) call repaint.
     */
    virtual void repaintPage() const = 0;

    /**
     * Repaint our buffer, then redraw the widget
     */
    virtual void rerenderPage() = 0;

    /**
     * Call this if you add an element, remove an element etc.
     */
    void rerenderElement(Element* e);
    void rerenderRange(const Range& r);

    /**
     * This updated the view buffer and then rerender the the region, call this if you changed the document
     */
    virtual void rerenderRect(double x, double y, double width, double height) = 0;

    /**
     * Return the GTK selection color
     */
    virtual GdkRGBA getSelectionColor() = 0;


    virtual void deleteViewBuffer() = 0;

    virtual int getX() const = 0;
    virtual int getY() const = 0;
};
