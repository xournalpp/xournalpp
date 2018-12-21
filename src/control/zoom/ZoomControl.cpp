#include "ZoomControl.h"

#include "gui/Layout.h"
#include "gui/PageView.h"
#include "gui/widgets/XournalWidget.h"
#include "gui/XournalView.h"

const double zoomStep = 0.04;

ZoomControl::ZoomControl()
 : view(NULL)
{
	XOJ_INIT_TYPE(ZoomControl);

	this->zoom = 1.0;
	this->lastZoomValue = 1.0;
	this->zoom100Value = 1.0;
	this->zoomFitValue = 1.0;
	this->zoomFitMode = true;
	this->zoom_center_x = -1;
	this->zoom_center_y = -1;

	this->zoomSequenceStart = 1;
}

ZoomControl::~ZoomControl()
{
	XOJ_CHECK_TYPE(ZoomControl);

	XOJ_RELEASE_TYPE(ZoomControl);
}

/**
 * Call this before any zoom is done, it saves the current page and position
 *
 * @param centerX Zoom Center X (use -1 for center of the visible rect)
 * @param centerY Zoom Center Y (use -1 for center of the visible rect)
 */
void ZoomControl::startZoomSequence(double centerX, double centerY)
{
	XOJ_CHECK_TYPE(ZoomControl);

	GtkWidget* widget = view->getWidget();
	Layout* layout = gtk_xournal_get_layout(widget);
	// Save visible rectangle at beginning of zoom
	zoomSequenceRectangle = layout->getVisibleRect();
	zoomSequenceStart = view->getZoom();

	if (centerX == -1 || centerY == -1)
	{
		this->zoom_center_x = gtk_widget_get_allocated_width(widget) / 2;
		this->zoom_center_y = gtk_widget_get_allocated_height(widget) / 2;
	}
	else
	{
		this->zoom_center_x = centerX;
		this->zoom_center_y = centerY;
	}
}

/**
 * Change the zoom within a Zoom sequnce (startZoomSequence() / endZoomSequence())
 *
 * @param zoom Current zoom value
 * @param realative If the zoom is realative to the start value (for Gesture)
 */
void ZoomControl::zoomSequnceChange(double zoom, bool realative)
{
	XOJ_CHECK_TYPE(ZoomControl);

	if (realative) {
		zoom *= zoomSequenceStart;
	}


}

/**
 * Clear all stored data from startZoomSequence()
 */
void ZoomControl::endZoomSequence()
{
	XOJ_CHECK_TYPE(ZoomControl);
	zoom_center_x = -1;
	zoom_center_y = -1;
}

/**
 * Zoom to correct position on zooming
 */
void ZoomControl::scrollToZoomPosition(XojPageView* view, double lastZoom)
{
	// Keep zoom center at static position in current view
	// by scrolling relative to counter motion induced by zoom
	// in orignal version top left corner of first page static
	// Pack into extra function later
	double zoom_now = getZoom();
	// relative scrolling
	double zoom_eff = zoom_now / lastZoom;
	int scroll_x;
	int scroll_y;
	// x,y position of visible rectangle for gesture scrolling
	int vis_x;
	int vis_y;

	Layout* layout = gtk_xournal_get_layout(this->view->getWidget());

	// get margins for relative scroll calculation
	double marginLeft = (double) view->layout.getMarginLeft();
	double marginTop = (double) view->layout.getMarginTop();

		// Absolute centred scrolling used for gesture
		if (true)//(this->zoom_gesture_active)
		{
			vis_x = (int) ((zoom_center_x - marginLeft) * (zoom_now / zoomSequenceStart - 1));
			vis_y = (int) ((zoom_center_y - marginTop) * (zoom_now / zoomSequenceStart - 1));
			layout->scrollAbs(zoomSequenceRectangle.x + vis_x, zoomSequenceRectangle.y + vis_y);
		}

//		// Relative centered scrolling used for SHIFT-mousewheel
//		if (zoom_eff != 1 && zoom->zoom_center_x != -1 && this->zoom_gesture_active == false)
//		{
//			scroll_x = (int) ((zoom->zoom_center_x - marginLeft) * (zoom_eff - 1));
//			scroll_y = (int) ((zoom->zoom_center_y - marginTop) * (zoom_eff - 1));
//
//			// adjust view by scrolling
//			layout->scrollRelativ(scroll_x, scroll_y);
//		}
}


void ZoomControl::addZoomListener(ZoomListener* listener)
{
	XOJ_CHECK_TYPE(ZoomControl);

	this->listener.push_back(listener);
}

void ZoomControl::initZoomHandler(GtkWidget* widget, XournalView* view)
{
	XOJ_CHECK_TYPE(ZoomControl);

	g_signal_connect(widget, "scroll_event", G_CALLBACK(
		+[](GtkWidget* widget, GdkEventScroll* event, ZoomControl* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, ZoomControl);
			return self->onScrolledwindowMainScrollEvent(event);
		}), this);

	this->view = view;
}

void ZoomControl::fireZoomChanged(double lastZoom)
{
	XOJ_CHECK_TYPE(ZoomControl);

	if (this->zoom < MIN_ZOOM)
	{
		this->zoom = MIN_ZOOM;
	}

	if (this->zoom > MAX_ZOOM)
	{
		this->zoom = MAX_ZOOM;
	}

	for (ZoomListener* z : this->listener)
	{
		z->zoomChanged(lastZoom);
	}
}

void ZoomControl::fireZoomRangeValueChanged()
{
	XOJ_CHECK_TYPE(ZoomControl);

	for (ZoomListener* z : this->listener)
	{
		z->zoomRangeValuesChanged();
	}
}

double ZoomControl::getZoom()
{
	XOJ_CHECK_TYPE(ZoomControl);

	return this->zoom;
}

void ZoomControl::setZoom(double zoom)
{
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = this->zoom;
	this->zoom = zoom;
	this->zoomFitMode = false;
	fireZoomChanged(lastZoom);
}

void ZoomControl::setZoom100(double zoom)
{
	XOJ_CHECK_TYPE(ZoomControl);

	this->zoom100Value = zoom;
	fireZoomRangeValueChanged();
}

void ZoomControl::setZoomFit(double zoom)
{
	XOJ_CHECK_TYPE(ZoomControl);

	this->zoomFitValue = zoom;
	fireZoomRangeValueChanged();

	if (this->zoomFitMode)
	{
		double lastZoom = this->zoom;
		this->zoom = this->zoomFitValue;
		fireZoomChanged(lastZoom);
	}
}

double ZoomControl::getZoomFit()
{
	XOJ_CHECK_TYPE(ZoomControl);

	return this->zoomFitValue;
}

double ZoomControl::getZoom100()
{
	XOJ_CHECK_TYPE(ZoomControl);

	return this->zoom100Value;
}

void ZoomControl::zoom100()
{
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = this->zoom;
	this->zoom = this->zoom100Value;
	this->zoomFitMode = false;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomFit()
{
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = this->zoom;
	this->zoom = this->zoomFitValue;
	this->zoomFitMode = true;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomIn()
{
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = this->zoom;
	this->zoom += zoomStep;
	this->zoomFitMode = false;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomOut()
{
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = this->zoom;
	this->zoom -= zoomStep;
	this->zoomFitMode = false;
	fireZoomChanged(lastZoom);
}

bool ZoomControl::onScrolledwindowMainScrollEvent(GdkEventScroll* event)
{
	XOJ_CHECK_TYPE(ZoomControl);

	guint state = event->state & gtk_accelerator_get_default_mod_mask();

	// do not handle e.g. ALT + Scroll (e.g. Compiz use this shortcut for setting transparency...)
	if (state != 0 && (state & ~(GDK_CONTROL_MASK | GDK_SHIFT_MASK)))
	{
		return true;
	}

	if (state & GDK_CONTROL_MASK)
	{
		//set zoom center (for ctrl centered scroll)
		zoom_center_x = event->x;
		zoom_center_y = event->y;

		if (event->direction == GDK_SCROLL_UP ||
			(event->direction == GDK_SCROLL_SMOOTH && event->delta_y > 0))
		{
			zoomIn();
		}
		else if (event->direction == GDK_SCROLL_DOWN ||
			(event->direction == GDK_SCROLL_SMOOTH && event->delta_y < 0))
		{
			zoomOut();
		}
		else
		{
			// don't zoom if scroll left or right
			zoom_center_x = -1;
			zoom_center_y = -1;
		}
		return true;
	}

	// Shift + Wheel scrolls the in the perpendicular direction
	if (state & GDK_SHIFT_MASK)
	{
		if (event->direction == GDK_SCROLL_UP)
		{
			event->direction = GDK_SCROLL_LEFT;
		}
		else if (event->direction == GDK_SCROLL_LEFT)
		{
			event->direction = GDK_SCROLL_UP;
		}
		else if (event->direction == GDK_SCROLL_DOWN)
		{
			event->direction = GDK_SCROLL_RIGHT;
		}
		else if (event->direction == GDK_SCROLL_RIGHT)
		{
			event->direction = GDK_SCROLL_DOWN;
		}

		event->state &= ~GDK_SHIFT_MASK;
		state &= ~GDK_SHIFT_MASK;
	}

	return false;
}
