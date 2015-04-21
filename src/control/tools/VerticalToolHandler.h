/*
 * Xournal++
 *
 * Vertical Space tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>
#include "../../gui/Redrawable.h"
#include "../../model/PageRef.h"
#include "../../view/ElementContainer.h"
#include "../../undo/MoveUndoAction.h"
#include <XournalType.h>

class VerticalToolHandler : public ElementContainer
{
public:
	VerticalToolHandler(Redrawable* view, PageRef page, double y, double zoom);
	virtual ~VerticalToolHandler();

	void paint(cairo_t* cr, GdkRectangle* rect, double zoom);
	void currentPos(double x, double y);

	MoveUndoAction* finalize();

	ElementVector* getElements();

private:
	XOJ_TYPE_ATTRIB;

	Redrawable* view;
	PageRef page;
	Layer* layer;
	ElementVector elements;

	cairo_surface_t* crBuffer;

	double startY;
	double endY;

	/**
	 * When we create a new page
	 */
	double jumpY;
};
