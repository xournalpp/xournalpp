/*
 * Xournal++
 *
 * Interface for GUI handling
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __REDRAWABLE_H__
#define __REDRAWABLE_H__

#include "../util/MemoryCheck.h"
#include "../util/Range.h"

#include <gdk/gdktypes.h>
#include <gtk/gtk.h>

class Element;

class Redrawable: public virtual MemoryCheckObject {
public:
	/**
	 * Call this if you only need to repaint the view, this means the buffer will be painted again,
	 * and all selections, text edtiors etc. are drawed again, but the view buffer is not refreshed.
	 *
	 * for refreshing the view buffer (if you have changed the document) call rerender.
	 */
	virtual void repaint(double x1, double y1, double x2, double y2) = 0;
	virtual void repaint(Range & r);
	virtual void repaint(Element * e) = 0;

	/**
	 * Call this if you only need to readraw the view, this means the buffer will be painted again,
	 * and all selections, text edtiors etc. are drawed again, but the view buffer is not refreshed.
	 *
	 * for refreshing the view buffer (if you have changed the document) call repaint.
	 */
	virtual void repaint() = 0;

	/**
	 * Repaint our buffer, then redraw the widget
	 */
	virtual void rerender() = 0;

	/**
	 * Call this if you add an element, remove an element etc.
	 */
	virtual void rerender(Element * e) = 0;
	virtual void rerender(Range & r) = 0;

	/**
	 * This updated the view buffer and then rerender the the region, call this if you changed the document
	 */
	virtual void rerender(double x, double y, double width, double heigth) = 0;

	/**
	 * Return the GTK selection color
	 */
	virtual GdkColor getSelectionColor() = 0;


	virtual void deleteViewBuffer() = 0;
};

#endif /* __REDRAWABLE_H__ */
