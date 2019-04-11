#include "BaseStrokeHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "undo/InsertUndoAction.h"
#include "gui/Cursor.h"
#include <cmath>

#define FIX_DRAW_TYPE_LOCK_DISTANCE 50 

BaseStrokeHandler::BaseStrokeHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
 : InputHandler(xournal, redrawable, page)
{
	XOJ_INIT_TYPE(BaseStrokeHandler);
}

void BaseStrokeHandler::snapToGrid(double& x, double& y)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	if (!xournal->getControl()->getSettings()->isSnapGrid())
	{
		return;
	}

	/**
	 * Snap points to a grid:
	 * If x/y coordinates are under a certain tolerance,
	 * fix the point to the grid intersection value
	 */
	double gridSize = 14.17;

	double t = xournal->getControl()->getSettings()->getSnapGridTolerance();
	double tolerance = (gridSize / 2) - (1 / t);

	double xRem = fmod(x, gridSize);
	double yRem = fmod(y, gridSize);

	bool snapX = false;
	bool snapY = false;

	double tmpX = 0;
	double tmpY = 0;

	if (xRem < tolerance)
	{
		tmpX = x - xRem;
		snapX = true;
	}
	if (xRem > gridSize - tolerance )
	{
		tmpX = x + (gridSize - xRem);
		snapX = true;
	}
	if (yRem < tolerance)
	{
		tmpY = y - yRem;
		snapY = true;
	}
	if (yRem > gridSize - tolerance )
	{
		tmpY = y + (gridSize - yRem);
		snapY = true;
	}

	if (snapX && snapY)
	{
		x = tmpX;
		y = tmpY;
	}
}

BaseStrokeHandler::~BaseStrokeHandler()
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	XOJ_RELEASE_TYPE(BaseStrokeHandler);
}

void BaseStrokeHandler::draw(cairo_t* cr)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	double zoom = xournal->getZoom();
	int dpiScaleFactor = xournal->getDpiScaleFactor();

	cairo_scale(cr, zoom * dpiScaleFactor, zoom * dpiScaleFactor);
	view.drawStroke(cr, stroke, 0);
}

bool BaseStrokeHandler::onMotionNotifyEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	if (!stroke)
	{
		return false;
	}

	double zoom = xournal->getZoom();
	double x = pos.x / zoom;
	double y = pos.y / zoom;
	int pointCount = stroke->getPointCount();

	Point currentPoint(x, y);
	Rectangle rect = stroke->boundingRect();

	if (pointCount > 0)
	{
		if (!validMotion(currentPoint, stroke->getPoint(pointCount - 1)))
		{
			return true;
		}
	}

	this->redrawable->repaintRect(stroke->getX(), stroke->getY(),
								  stroke->getElementWidth(), stroke->getElementHeight());

	drawShape(currentPoint, pos);
	
	rect.add(stroke->boundingRect());
	double w = stroke->getWidth();

	redrawable->repaintRect(rect.x - w, rect.y - w,
							rect.width + 2 * w,
							rect.height + 2 * w);

	return true;
}

void BaseStrokeHandler::onButtonReleaseEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	if (stroke == NULL)
	{
		return;
	}

	// This is not a valid stroke
	if (stroke->getPointCount() < 2)
	{
		g_warning("Stroke incomplete!");

		delete stroke;
		stroke = NULL;
		return;
	}

	stroke->freeUnusedPointItems();


	Control* control = xournal->getControl();
	control->getLayerController()->ensureLayerExists(page);

	Layer* layer = page->getSelectedLayer();

	UndoRedoHandler* undo = control->getUndoRedoHandler();

	undo->addUndoAction(new InsertUndoAction(page, layer, stroke));

	layer->addElement(stroke);
	page->fireElementChanged(stroke);

	stroke = NULL;

	return;
}

void BaseStrokeHandler::onButtonPressEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);
	
	Settings* settings = xournal->getControl()->getSettings();
	this->settingsDrawDirModsEnabled = settings->getDrawDirModsEnabled();
	this->settingsDrawDirModsRadius =  settings->getDrawDirModsRadius();

	double zoom = xournal->getZoom();
	double x = pos.x / zoom;
	double y = pos.y / zoom;

	if (!stroke)
	{
		createStroke(Point(x, y));
	}
}


void BaseStrokeHandler::setModifiers(double width, double height, const PositionInputData& pos, bool doDirMods, bool changeCursor)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);
		
	bool gestureShift = false;
	bool gestureControl = false;
	
	if (doDirMods && this->settingsDrawDirModsEnabled)
	{

		if( this->drawModifier == NONE){
			gestureShift = (width  < 0);
			gestureControl =  (height < 0 );
			
			double zoom = xournal->getZoom();
			double fixate_Dir_Mods_Dist = std::pow( this->settingsDrawDirModsRadius / zoom, 2.0); 
			if (std::pow(width,2.0) > fixate_Dir_Mods_Dist ||  std::pow(height,2.0) > fixate_Dir_Mods_Dist )
			{
				this->drawModifier = (DIRSET_MODIFIERS)(SET |
					(gestureShift? SHIFT:NONE) |
					(gestureControl? CONTROL:NONE) );
				if(changeCursor)
				{
					xournal->getCursor()->updateCursor();
				}
			}
			else
			{
				if (changeCursor)
				{
					int corner = ( gestureShift?0:1 ) + ( gestureControl?2:0);
					
					if( corner != this-> lastCursor)
					{
						switch (corner)
						{
							case 1:
								xournal->getCursor()->setTempCursor(GDK_BOTTOM_RIGHT_CORNER);
								break;
							case 3:
								xournal->getCursor()->setTempCursor(GDK_TOP_RIGHT_CORNER);
								break;
							case 0:
								xournal->getCursor()->setTempCursor(GDK_BOTTOM_LEFT_CORNER);
								break;
							case 2:
								xournal->getCursor()->setTempCursor(GDK_TOP_LEFT_CORNER);
								break;
						}
						this->lastCursor = corner;
					}
				}
			}
		}
		else
		{
			gestureShift = this->drawModifier & SHIFT;
			gestureControl = this->drawModifier & CONTROL;
		}
	}
	this->modShift = pos.isShiftDown() ==  !gestureShift;
	this->modControl = pos.isControlDown() == !gestureControl;
		
}
