#include "StrokeHandler.h"

#include "gui/XournalView.h"
#include "gui/PageView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"
#include "control/shaperecognizer/ShapeRecognizerResult.h"
#include "undo/RecognizerUndoAction.h"

#include <gdk/gdk.h>
#include <math.h>

StrokeHandler::StrokeHandler(XournalView* xournal,
                             PageView* redrawable,
                             PageRef page)
	: InputHandler(xournal, redrawable, page),
	  surfMask(NULL), crMask(NULL), reco(NULL)
{
	XOJ_INIT_TYPE(StrokeHandler);
}

StrokeHandler::~StrokeHandler()
{
	XOJ_CHECK_TYPE(StrokeHandler);

	destroySurface();
	delete reco;

	XOJ_RELEASE_TYPE(StrokeHandler);
}

void StrokeHandler::draw(cairo_t* cr)
{
	XOJ_CHECK_TYPE(StrokeHandler);

	if (!stroke)
	{
		return;
	}

	view.applyColor(cr, stroke);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_mask_surface(cr, surfMask, visRect.x, visRect.y);
}

bool StrokeHandler::onMotionNotifyEvent(GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(StrokeHandler);

	if (!stroke)
	{
		return false;
	}

	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;
	bool presureSensitivity = xournal->getControl()->getSettings()->isPresureSensitivity();
	int pointCount = stroke->getPointCount();

	Point currentPoint(x, y);

	if (pointCount > 0)
	{
		if(!validMotion(currentPoint, stroke->getPoint(pointCount - 1)))
		{
			return true;
		}
	}

	ToolHandler* h = xournal->getControl()->getToolHandler();

	if (presureSensitivity)
	{
		double pressure = Point::NO_PRESURE;
		if (h->getToolType() == TOOL_PEN)
		{
			if (getPressureMultiplier((GdkEvent*) event, pressure))
			{
				pressure = pressure * stroke->getWidth();
			}
			else
			{
				pressure = Point::NO_PRESURE;
			}
		}

		stroke->setLastPressure(pressure);
	}

	if (pointCount > 0)
	{
		Point prevPoint(stroke->getPoint(pointCount - 1));

		Stroke lastSegment;

		lastSegment.addPoint(prevPoint);
		lastSegment.addPoint(currentPoint);
		lastSegment.setWidth(stroke->getWidth());

		cairo_set_operator(crMask, CAIRO_OPERATOR_OVER);
		cairo_set_source_rgba(crMask, 1, 1, 1, 1);

		view.drawStroke(crMask, &lastSegment, 0, 1, false);
	}

	stroke->addPoint(currentPoint);

	const double w = stroke->getWidth();

	this->redrawable->repaintRect(stroke->getX() - w,
	                              stroke->getY() - w,
	                              stroke->getElementWidth() + 2*w,
	                              stroke->getElementHeight() + 2*w);

	return true;
}

void StrokeHandler::onButtonReleaseEvent(GdkEventButton* event)
{
	XOJ_CHECK_TYPE(StrokeHandler);

	if (!stroke)
	{
		return;
	}

	// Backward compatibility and also easier to handle for me;-)
	// I cannot draw a line with one point, to draw a visible line I need two points,
	// twice the same Point is also OK
	if (stroke->getPointCount() == 1)
	{
		ArrayIterator<Point> it = stroke->pointIterator();
		if (it.hasNext())
		{
			stroke->addPoint(it.next());
		}
		// No pressure sensitivity
		stroke->clearPressure();
	}

	stroke->freeUnusedPointItems();

	if (page->getSelectedLayerId() < 1)
	{
		// This creates a layer if none exists
		page->getSelectedLayer();
		page->setSelectedLayerId(1);
		xournal->getControl()->getWindow()->updateLayerCombobox();
	}

	Layer* layer = page->getSelectedLayer();

	UndoRedoHandler* undo = xournal->getControl()->getUndoRedoHandler();

	undo->addUndoAction(new InsertUndoAction(page,
	                                         layer,
	                                         stroke));


	ToolHandler* h = xournal->getControl()->getToolHandler();

	if(h->isShapeRecognizer())
	{
		if (!reco)
		{
			reco = new ShapeRecognizer();
		}

		ShapeRecognizerResult* result = reco->recognizePatterns(stroke);
		
		if(result)
		{
			Stroke* recognized = result->getRecognized();

			RecognizerUndoAction* recognizerUndo = new RecognizerUndoAction(page, layer, stroke, recognized);

			undo->addUndoAction(recognizerUndo);
			layer->addElement(result->getRecognized());

			Range range(recognized->getX(), recognized->getY());
			range.addPoint(recognized->getX() + recognized->getElementWidth(),
			recognized->getY() + recognized->getElementHeight());

			range.addPoint(stroke->getX(), stroke->getY());
			range.addPoint(stroke->getX() + stroke->getElementWidth(),
			               stroke->getY() + stroke->getElementHeight());

			for (Stroke* s : *result->getSources())
			{
				layer->removeElement(s, false);

				recognizerUndo->addSourceElement(s);

				range.addPoint(s->getX(), s->getY());
				range.addPoint(s->getX() + s->getElementWidth(),
						           s->getY() + s->getElementHeight());
			}

			page->fireRangeChanged(range);

			// delete the result object, this is not needed anymore, the stroke are not deleted with this
			delete result;

		}
		else
		{
			layer->addElement(stroke);
			page->fireElementChanged(stroke);
		}
	}
	else
	{
		layer->addElement(stroke);
		page->fireElementChanged(stroke);
	}

	stroke = NULL;

	return;
}

void StrokeHandler::onButtonPressEvent(GdkEventButton* event)
{
	XOJ_CHECK_TYPE(StrokeHandler);

	destroySurface();

	const double width = redrawable->getDisplayWidth();
	const double height = redrawable->getDisplayHeight();
	const double zoom = xournal->getZoom();

	/* TODO: The surface can be made much smaller, we
	 *       only need the part of the page
	 *       that is actually visible on the screen
	 * 
	 *       If you want to debug the mask, use
	 *       CAIRO_FORMAT_RGB24 instead
	 */

	Rectangle *rectPtr = redrawable->getVisibleRect();

	if(!rectPtr)
	{
		g_warning("Attempting to draw on an invisible surface");
		return;
	}

	visRect = *(rectPtr);

	delete rectPtr;

	surfMask = cairo_image_surface_create(CAIRO_FORMAT_A8,
	                                      visRect.width, visRect.height);

	crMask = cairo_create(surfMask);

	// for debugging purposes
	// cairo_set_source_rgba(crMask, 0, 0, 0, 1);
	cairo_set_source_rgba(crMask, 0, 0, 0, 0);
	cairo_rectangle(crMask, 0, 0, width, height);

	cairo_fill(crMask);

	cairo_translate(crMask, -visRect.x, -visRect.y);
	cairo_scale(crMask, zoom, zoom);

	if (!stroke)
	{
		double x = event->x / zoom;
		double y = event->y / zoom;

		createStroke(Point(x, y));
	}
}

bool StrokeHandler::getPressureMultiplier(GdkEvent* event, double& pressure)
{
	XOJ_CHECK_TYPE(StrokeHandler);

	GdkDevice* device = gdk_event_get_device(event);
	gdouble* axes = event->button.axes;

	pressure = 1.0;

	gdk_device_get_state(device,
	                     gtk_widget_get_parent_window(xournal->getWidget()),
	                     axes, NULL);

	if (!gdk_device_get_axis(device, axes, GDK_AXIS_PRESSURE, &pressure))
	{
		pressure = 1.0;
		return false;
	}

	// from the documentation:
	// "The pressure field is a a double value ranging from 0.0 to 1.0"

	Settings* settings = xournal->getControl()->getSettings();

	if (!finite(pressure))
	{
		pressure = 1.0;
	}

	pressure = ((1 - pressure) * settings->getWidthMinimumMultiplier() +
	           pressure * settings->getWidthMaximumMultiplier());

	return true;
}

void StrokeHandler::destroySurface()
{
	XOJ_CHECK_TYPE(StrokeHandler);

	if (surfMask || crMask)
	{
		cairo_destroy(crMask);
		cairo_surface_destroy(surfMask);
		surfMask = NULL;
		crMask = NULL;
	}
}

void StrokeHandler::resetShapeRecognizer()
{
	XOJ_CHECK_TYPE(StrokeHandler);

	if (reco)
	{
		delete reco;
		reco = NULL;
	}
}

