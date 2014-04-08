#include "InputHandler.h"
#include "../../gui/XournalView.h"
#include "../../gui/PageView.h"
#include "../../gui/widgets/XournalWidget.h"
#include "../Control.h"
#include "../shaperecognizer/ShapeRecognizerResult.h"
#include "../../undo/InsertUndoAction.h"
#include "../../undo/RecognizerUndoAction.h"
#include "../../view/DocumentView.h"
#include "../../model/Layer.h"
#include "../../util/XInputUtils.h"
#include "../../util/Rectangle.h"

#include <math.h>

#define PIXEL_MOTION_THRESHOLD 0.3

InputHandler::InputHandler(XournalView* xournal, PageView* redrawable)
{
	XOJ_INIT_TYPE(InputHandler);

	this->tmpStroke = NULL;
	this->currentInputDevice = NULL;
	this->tmpStrokeDrawElem = 0;
	this->view = new DocumentView();
	this->redrawable = redrawable;
	this->xournal = xournal;
	this->reco = NULL;
}

InputHandler::~InputHandler()
{
	XOJ_CHECK_TYPE(InputHandler);

	this->tmpStroke = NULL;
	this->currentInputDevice = NULL;
	this->redrawable = NULL;
	delete this->view;
	this->view = NULL;

	if (this->reco)
	{
		delete this->reco;
		this->reco = NULL;
	}

	XOJ_RELEASE_TYPE(InputHandler);
}

void InputHandler::addPointToTmpStroke(GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(InputHandler);

	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;
	bool presureSensitivity =
	    xournal->getControl()->getSettings()->isPresureSensitivity();

	if (tmpStroke->getPointCount() > 0)
	{
		Point p = tmpStroke->getPoint(tmpStroke->getPointCount() - 1);

		if (hypot(p.x - x, p.y - y) < PIXEL_MOTION_THRESHOLD)
		{
			return; // not a meaningful motion
		}
	}

	ToolHandler* h = xournal->getControl()->getToolHandler();

	if (h->isRuler())
	{
		int count = tmpStroke->getPointCount();

		this->redrawable->repaintRect(tmpStroke->getX(), tmpStroke->getY(),
		                              tmpStroke->getElementWidth(),
		                              tmpStroke->getElementHeight());

		if (count < 2)
		{
			tmpStroke->addPoint(Point(x, y));
		}
		else
		{
			tmpStroke->setLastPoint(x, y);
		}
		Point p = tmpStroke->getPoint(0);

		drawTmpStroke(true);
		return;
	}
	else if (h->isRectangle())
	{
		//printf("Drawing rectangle\n");
		int count = tmpStroke->getPointCount();
		this->redrawable->repaintRect(tmpStroke->getX(), tmpStroke->getY(),
					      tmpStroke->getElementWidth(),
					      tmpStroke->getElementHeight());

		if (count < 1)
		{
			tmpStroke->addPoint(Point(x,y));
		}
		else
		{
			Point p = tmpStroke->getPoint(0);
			if (count > 3)
			{
				tmpStroke->deletePoint(4);
				tmpStroke->deletePoint(3);
				tmpStroke->deletePoint(2);
				tmpStroke->deletePoint(1);
			}
			tmpStroke->addPoint(Point(x,p.y));
			tmpStroke->addPoint(Point(x,y));
			tmpStroke->addPoint(Point(p.x,y));
			tmpStroke->addPoint(p);
		}
		drawTmpStroke(true);
		return;

	}
	else if (h->isCircle())
	{
		int count = tmpStroke->getPointCount();
		this->redrawable->repaintRect(tmpStroke->getX(), tmpStroke->getY(),
					      tmpStroke->getElementWidth(),
					      tmpStroke->getElementHeight());

		//g_mutex_lock(this->redrawable->drawingMutex);
		if (count < 2)
		{
			tmpStroke->addPoint(Point(x,y));
		}
		else
		{
			Point p = tmpStroke->getPoint(0);
			//set resolution proportional to radius
			double diameter = sqrt(pow(x-p.x,2.0) + pow(y-p.y,2.0));
			int npts = (int) ( diameter/2.0 );
			double center_x = (x + p.x)/2.0;
			double center_y = (y + p.y)/2.0;
			double angle = atan2( (y-p.y) , (x-p.x) );


			if (npts < 12)
			{
				npts = 12; // min. number of points
			}

			//remove previous points
			count = tmpStroke->getPointCount();
			tmpStroke->deletePointsFrom(1);
			for (int i = 1; i < npts; i++)
			{
				double xp = center_x + diameter/2.0 * cos((2 * M_PI * i ) / npts + angle + M_PI);
				double yp = center_y + diameter/2.0 * sin((2 * M_PI * i ) / npts + angle + M_PI);
				tmpStroke->addPoint(Point(xp, yp));
			}
			tmpStroke->addPoint(Point(p.x,p.y));
		}
		//g_mutex_unlock(this->redrawable->drawingMutex);

		drawTmpStroke(true);
		return;
	}
	else if (h->isArrow())
	{
		int count = tmpStroke->getPointCount();
		this->redrawable->repaintRect(tmpStroke->getX(), tmpStroke->getY(),
					      tmpStroke->getElementWidth(),
					      tmpStroke->getElementHeight());

		if (count < 1)
		{
			tmpStroke->addPoint(Point(x,y));
		}
		else
		{
			Point p = tmpStroke->getPoint(0);

			if (count > 3)
			{
				//remove previous points
				tmpStroke->deletePoint(4);
				tmpStroke->deletePoint(3);
				tmpStroke->deletePoint(2);
				tmpStroke->deletePoint(1);
			}

			//We've now computed the line points for the arrow
			//so we just have to build the head
			
			//set up the size of the arrowhead to be 1/8 the length of arrow
			double dist = sqrt( pow(x-p.x,2.0) + pow(y-p.y,2.0) )/8.0;
			
			double angle = atan2( (y-p.y) , (x-p.x) );
			//an appropriate delta is Pi/3 radians for an arrow shape
			double delta = M_PI / 6.0;


			tmpStroke->addPoint(Point(x, y));

			tmpStroke->addPoint(Point(x - dist * cos(angle + delta),
					  y - dist * sin(angle + delta)));

			tmpStroke->addPoint(Point(x, y));

			tmpStroke->addPoint(Point(x - dist * cos(angle - delta),
					  y - dist * sin(angle - delta)));
		}
		drawTmpStroke(true);
		return;
	}

	if (presureSensitivity)
	{
		double pressure = Point::NO_PRESURE;
		if (h->getToolType() == TOOL_PEN)
		{
			if (getPressureMultiplier((GdkEvent*) event, pressure))
			{
				pressure = pressure * tmpStroke->getWidth();
			}
			else
			{
				pressure = Point::NO_PRESURE;
			}
		}

		tmpStroke->setLastPressure(pressure);
	}
	tmpStroke->addPoint(Point(x, y));

	drawTmpStroke();
}

bool InputHandler::getPressureMultiplier(GdkEvent* event, double& presure)
{
	XOJ_CHECK_TYPE(InputHandler);

	double* axes = NULL;
	GdkDevice* device = NULL;

	if (event->type == GDK_MOTION_NOTIFY)
	{
		axes = event->motion.axes;
		device = event->motion.device;
	}
	else
	{
		axes = event->button.axes;
		device = event->button.device;
	}

	if (device == gdk_device_get_core_pointer() || device->num_axes <= 2)
	{
		presure = 1.0;
		return false;
	}

	double rawpressure = axes[2] / (device->axes[2].max - device->axes[2].min);
	if (!finite(rawpressure))
	{
		presure = 1.0;
		return false;
	}

	Settings* settings = xournal->getControl()->getSettings();

	presure = ((1 - rawpressure) * settings->getWidthMinimumMultiplier() +
	           rawpressure * settings->getWidthMaximumMultiplier());
	return true;
}

void InputHandler::drawTmpStroke(bool do_redraw)
{
	XOJ_CHECK_TYPE(InputHandler);

	if (this->tmpStroke)
	{
		cairo_t* cr = gtk_xournal_create_cairo_for(this->xournal->getWidget(),
		                                           this->redrawable);

		double zoom = xournal->getControl()->getZoomControl()->getZoom();

		/*
		 * After strokes are drawn they are re-drawn, it appears with slightly different width or anti-aliasing.
		 * I don't know the problem exactly, one time we draw direct to the screen (here) or to a buffer (normal drawing)
		 * To avoid the difference I did some workaround with factors, so its nearly invisible... Hopefully with all
		 * cairo releases
		 *
		 * Andreas Butti
		 */

		g_mutex_lock(&this->redrawable->drawingMutex);

		this->view->drawStroke(cr,
		                       this->tmpStroke,
		                       do_redraw ? 0 : this->tmpStrokeDrawElem,
		                       getZoomFactor(zoom));

		this->tmpStrokeDrawElem = this->tmpStroke->getPointCount() - 1;
		cairo_destroy(cr);

		g_mutex_unlock(&this->redrawable->drawingMutex);
	}
}

double InputHandler::getZoomFactor(double zoom)
{
	double factor = 1;

	if (zoom < 0.8)
	{
		factor = sqrt(zoom);
	}
	else if (zoom < 1.0)
	{
		factor = sqrt(zoom) - 0.3;
	}
	else if (zoom < 1.5)
	{
		factor = sqrt(zoom) - 0.2;
	}

	return factor;
}

void InputHandler::draw(cairo_t* cr, double zoom)
{
	XOJ_CHECK_TYPE(InputHandler);

	if (this->tmpStroke)
	{
		this->view->drawStroke(cr, this->tmpStroke, true, getZoomFactor(zoom));
	}
}

void InputHandler::onButtonReleaseEvent(GdkEventButton* event, PageRef page)
{
	XOJ_CHECK_TYPE(InputHandler);

	if (!this->tmpStroke)
	{
		return;
	}

	// Backward compatibility and also easier to handle for me;-)
	// I cannot draw a line with one point, to draw a visible line I need two points,
	// twice the same Point is also OK
	if (this->tmpStroke->getPointCount() == 1)
	{
		ArrayIterator<Point> it = this->tmpStroke->pointIterator();
		if (it.hasNext())
		{
			this->tmpStroke->addPoint(it.next());
		}
		// No pressure sensitivity
		this->tmpStroke->clearPressure();
	}

	this->tmpStroke->freeUnusedPointItems();

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
	                                         this->tmpStroke));

	ToolHandler* h = xournal->getControl()->getToolHandler();
	if (h->isShapeRecognizer())
	{
		if (this->reco == NULL)
		{
			this->reco = new ShapeRecognizer();
		}
		ShapeRecognizerResult* result = this->reco->recognizePatterns(this->tmpStroke);

		if (result != NULL)
		{
			UndoRedoHandler* undo = xournal->getControl()->getUndoRedoHandler();

			Stroke* recognized = result->getRecognized();

			RecognizerUndoAction* recognizerUndo = new RecognizerUndoAction(page,
			                                                                layer,
			                                                                this->tmpStroke,
			                                                                recognized);

			undo->addUndoAction(recognizerUndo);
			layer->addElement(result->getRecognized());

			Range range(recognized->getX(), recognized->getY());
			range.addPoint(recognized->getX() + recognized->getElementWidth(),
			               recognized->getY() + recognized->getElementHeight());

			range.addPoint(this->tmpStroke->getX(), this->tmpStroke->getY());
			range.addPoint(this->tmpStroke->getX() + this->tmpStroke->getElementWidth(),
			               this->tmpStroke->getY() + this->tmpStroke->getElementHeight());

			ListIterator<Stroke*> l = result->getSources();
			while (l.hasNext())
			{
				Stroke* s = l.next();

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
			layer->addElement(this->tmpStroke);
			page->fireElementChanged(this->tmpStroke);
		}

	}
	else
	{
		layer->addElement(this->tmpStroke);
		page->fireElementChanged(this->tmpStroke);
	}

	this->tmpStroke = NULL;

	if (currentInputDevice == event->device)
	{
		currentInputDevice = NULL;
		INPUTDBG("currentInputDevice = NULL\n", 0);
	}

	this->tmpStrokeDrawElem = 0;
}

bool InputHandler::onMotionNotifyEvent(GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(InputHandler);

	if (this->tmpStroke != NULL)
	{
		this->addPointToTmpStroke(event);
		return true;
	}
	return false;
}

void InputHandler::startStroke(GdkEventButton* event, StrokeTool tool, double x,
                               double y)
{
	XOJ_CHECK_TYPE(InputHandler);

	ToolHandler* h = xournal->getControl()->getToolHandler();

	if(event->device == NULL)
	{
		g_warning("startStroke: event->device == null");
	}

	if (tmpStroke == NULL)
	{
		currentInputDevice = event->device;
		tmpStroke = new Stroke();
		tmpStroke->setWidth(h->getThickness());
		tmpStroke->setColor(h->getColor());
		tmpStroke->setToolType(tool);
		tmpStroke->addPoint(Point(x, y));
	}
}

Stroke* InputHandler::getTmpStroke()
{
	XOJ_CHECK_TYPE(InputHandler);

	return tmpStroke;
}

void InputHandler::resetShapeRecognizer()
{
	XOJ_CHECK_TYPE(InputHandler);

	if (this->reco)
	{
		delete this->reco;
		this->reco = NULL;
	}
}
