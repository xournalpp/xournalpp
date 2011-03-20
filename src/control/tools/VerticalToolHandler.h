/*
 * Xournal++
 *
 * Vertical Space tool
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __VERTICALTOOLHANDLER_H__
#define __VERTICALTOOLHANDLER_H__

#include <cairo.h>
#include "../../gui/Redrawable.h"
#include "../../model/Page.h"
#include "../../view/ElementContainer.h"
#include "../../undo/MoveUndoAction.h"

class VerticalToolHandler: public ElementContainer {
public:
	VerticalToolHandler(Redrawable * view, XojPage * page, double y, double zoom);
	virtual ~VerticalToolHandler();

	void paint(cairo_t * cr, GdkRectangle * rect, double zoom);
	void currentPos(double x, double y);

	MoveUndoAction * finalize();

	ListIterator<Element *> getElements();
private:
	Redrawable * view;
	XojPage * page;
	Layer * layer;
	GList * elements;

	cairo_surface_t * crBuffer;

	double startY;
	double endY;

	/**
	 * When we create a new page
	 */
	double jumpY;

	friend class MoveUndoAction;
};

#endif /* __VERTICALTOOLHANDLER_H__ */
