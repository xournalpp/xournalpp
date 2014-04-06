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
#include "../../model/PageRef.h"
#include "../../view/ElementContainer.h"
#include "../../undo/MoveUndoAction.h"
#include <XournalType.h>

class VerticalToolHandler: public ElementContainer
{
public:
	VerticalToolHandler(Redrawable* view, PageRef page, double y, double zoom);
	virtual ~VerticalToolHandler();

	void paint(cairo_t* cr, GdkRectangle* rect, double zoom);
	void currentPos(double x, double y);

	MoveUndoAction* finalize();

	ListIterator<Element*> getElements();

private:
	XOJ_TYPE_ATTRIB;

	Redrawable* view;
	PageRef page;
	Layer* layer;
	GList* elements;

	cairo_surface_t* crBuffer;

	double startY;
	double endY;

	/**
	 * When we create a new page
	 */
	double jumpY;
};

#endif /* __VERTICALTOOLHANDLER_H__ */
