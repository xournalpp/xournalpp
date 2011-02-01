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
#include "Redrawable.h"
#include "../model/Page.h"
#include "../view/ElementContainer.h"


class MoveUndoAction;

class VerticalToolHandler: public ElementContainer {
public:
	VerticalToolHandler(Redrawable * view, XojPage * page, double y, double zoom);
	virtual ~VerticalToolHandler();

	void paint(cairo_t * cr, GdkEventExpose *event, double zoom);
	void currentPos(double x, double y);

	MoveUndoAction * finnalize();

	GList * getElements();
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
