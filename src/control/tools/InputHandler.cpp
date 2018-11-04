#include "InputHandler.h"
#include "gui/XournalView.h"
#include "gui/PageView.h"
#include "gui/widgets/XournalWidget.h"
#include "control/Control.h"
#include "control/shaperecognizer/ShapeRecognizerResult.h"
#include "undo/InsertUndoAction.h"
#include "undo/RecognizerUndoAction.h"
#include "view/DocumentView.h"
#include "model/Layer.h"
#include "util/Rectangle.h"

#include <math.h>

#define PIXEL_MOTION_THRESHOLD 0.3

InputHandler::InputHandler(XournalView* _xournal,
                           PageView* _redrawable,
                           PageRef _page)
	: xournal(_xournal), redrawable(_redrawable), page(_page), stroke(NULL)
{
	XOJ_INIT_TYPE(InputHandler);
}

InputHandler::~InputHandler()
{
	XOJ_CHECK_TYPE(InputHandler);

	XOJ_RELEASE_TYPE(InputHandler);
}

Stroke* InputHandler::getStroke()
{
	XOJ_CHECK_TYPE(InputHandler);

	return stroke;
}

/**
 * Reset the shape recognizer, only implemented by drawing instances,
 * but needs to be in the base interface.
 */
void InputHandler::resetShapeRecognizer()
{
	// Does nothing here. Implemented in the extending classes
}

void InputHandler::createStroke(Point p)
{
	XOJ_CHECK_TYPE(InputHandler);

	ToolHandler* h = xournal->getControl()->getToolHandler();

	stroke = new Stroke();
	stroke->setWidth(h->getThickness());
	stroke->setColor(h->getColor());

	if (h->getToolType() == TOOL_PEN)
	{
		stroke->setToolType(STROKE_TOOL_PEN);
	}
	else if (h->getToolType() == TOOL_HILIGHTER)
	{
		stroke->setToolType(STROKE_TOOL_HIGHLIGHTER);
	}
	else if (h->getToolType() == TOOL_ERASER)
	{
		stroke->setToolType(STROKE_TOOL_ERASER);
		stroke->setColor(0xffffff);
	}

	stroke->addPoint(p);
}

bool InputHandler::validMotion(Point p, Point q)
{
	XOJ_CHECK_TYPE(InputHandler);

	return (hypot(p.x - q.x, p.y - q.y) >= PIXEL_MOTION_THRESHOLD);
}
