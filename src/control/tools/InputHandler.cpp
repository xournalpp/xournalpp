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

#include "gui/MainWindow.h"

#include <math.h>

#define PIXEL_MOTION_THRESHOLD 0.3

InputHandler::InputHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
 : xournal(xournal),
   redrawable(redrawable),
   page(page),
   stroke(NULL)
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
	stroke->setFill(h->getFill());

	if (h->getToolType() == TOOL_PEN)
	{
		stroke->setToolType(STROKE_TOOL_PEN);

		if (xournal->getControl()->getAudioController()->isRecording())
		{
			//TODO: move this logic inside AudioController
			string audioFilename = xournal->getControl()->getAudioController()->getAudioFilename();
			gint sttime = xournal->getControl()->getAudioController()->getStartTime();
			int seconds = ((g_get_monotonic_time() / 1000000) - sttime);
			stroke->setTimestamp(seconds);
			stroke->setAudioFilename(audioFilename);
		}
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
