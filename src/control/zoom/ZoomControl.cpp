#include "ZoomControl.h"

#include "gui/Layout.h"
#include "gui/PageView.h"
#include "gui/widgets/XournalWidget.h"
#include "gui/XournalView.h"

ZoomControl::ZoomControl()
 : view(NULL)
{
	XOJ_INIT_TYPE(ZoomControl);

	this->zoom = 1.0;
	this->lastZoomValue = 1.0;
	this->zoom100Value = 1.0;
	this->zoomFitValue = 1.0;
	this->zoomFitMode = true;

	this->zoomStep = DEFAULT_ZOOM_STEP;
	this->zoomMax = DEFAULT_ZOOM_MAX;
	this->zoomMin = DEFAULT_ZOOM_MIN;

	this->zoomSequenceStart = -1;

	this->zoomWidgetPosX = 0;
	this->zoomWidgetPosY = 0;
	this->scrollPositionX = 0;
	this->scrollPositionY = 0;
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

	if (centerX == -1 || centerY == -1)
	{
		GtkWidget* widget = view->getWidget();
		this->zoomWidgetPosX = gtk_widget_get_allocated_width(widget) / 2;
		this->zoomWidgetPosY = gtk_widget_get_allocated_height(widget) / 2;
	}
	else
	{
		this->zoomWidgetPosX = centerX;
		this->zoomWidgetPosY = centerY;
	}

	Rectangle rect = getVisibleRect();

	this->scrollPositionX = (rect.x + this->zoomWidgetPosX) / this->zoom;
	this->scrollPositionY = (rect.y + this->zoomWidgetPosY) / this->zoom;

	this->zoomSequenceStart = this->zoom;
}

/**
 * Change the zoom within a Zoom sequence (startZoomSequence() / endZoomSequence())
 *
 * @param zoom Current zoom value
 * @param relative If the zoom is relative to the start value (for Gesture)
 */
void ZoomControl::zoomSequnceChange(double zoom, bool relative)
{
	XOJ_CHECK_TYPE(ZoomControl);

	if (relative && this->zoomSequenceStart != -1)
	{
		zoom *= zoomSequenceStart;
	}

	setZoom(zoom);
}

/**
 * Clear all stored data from startZoomSequence()
 */
void ZoomControl::endZoomSequence()
{
	XOJ_CHECK_TYPE(ZoomControl);
	scrollPositionX = -1;
	scrollPositionY = -1;

	zoomSequenceStart = -1;
}

/**
 * Get visible rect on xournal view, for Zoom Gesture
 */
Rectangle ZoomControl::getVisibleRect()
{
	GtkWidget* widget = view->getWidget();
	Layout* layout = gtk_xournal_get_layout(widget);
	return layout->getVisibleRect();
}

/**
 * Zoom to correct position on zooming
 */
void ZoomControl::scrollToZoomPosition(XojPageView* view)
{
	if (this->zoomSequenceStart == -1)
	{
		return;
	}

	Layout* layout = gtk_xournal_get_layout(this->view->getWidget());

	double x = this->scrollPositionX * this->zoom;
	double y = this->scrollPositionY * this->zoom;

	x -= this->zoomWidgetPosX;
	y -= this->zoomWidgetPosY;

	layout->scrollAbs(x, y);
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

void ZoomControl::fireZoomChanged()
{
	XOJ_CHECK_TYPE(ZoomControl);

	if (this->zoom < this->zoomMin)
	{
		this->zoom = this->zoomMin;
	}

	if (this->zoom > this->zoomMax)
	{
		this->zoom = this->zoomMax;
	}

	for (ZoomListener* z : this->listener)
	{
		z->zoomChanged();
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

	this->zoom = zoom;
	this->zoomFitMode = false;
	fireZoomChanged();
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
		this->zoom = this->zoomFitValue;
		fireZoomChanged();
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

	startZoomSequence(-1, -1);

	this->zoom = this->zoom100Value;
	this->zoomFitMode = false;
	fireZoomChanged();

	endZoomSequence();
}

void ZoomControl::zoomFit()
{
	XOJ_CHECK_TYPE(ZoomControl);

	startZoomSequence(-1, -1);

	this->zoom = this->zoomFitValue;
	this->zoomFitMode = true;
	fireZoomChanged();

	endZoomSequence();
}

void ZoomControl::zoomIn(double x, double y)
{
	XOJ_CHECK_TYPE(ZoomControl);

	startZoomSequence(x, y);

	this->zoom += this->zoomStep;
	this->zoomFitMode = false;
	fireZoomChanged();

	endZoomSequence();
}

void ZoomControl::zoomOut(double x, double y)
{
	XOJ_CHECK_TYPE(ZoomControl);

	startZoomSequence(x, y);

	this->zoom -= this->zoomStep;
	this->zoomFitMode = false;
	fireZoomChanged();

	endZoomSequence();
}

double ZoomControl::getZoomStep()
{
	return this->zoomStep;
}

void ZoomControl::setZoomStep(double zoomStep)
{
	XOJ_CHECK_TYPE(ZoomControl);

	this->zoomStep = zoomStep;
}

double ZoomControl::getZoomMax()
{
	XOJ_CHECK_TYPE(ZoomControl);

	return this->zoomMax;
}

void ZoomControl::setZoomMax(double zoomMax)
{
	XOJ_CHECK_TYPE(ZoomControl);

	this->zoomMax = zoomMax;
}

double ZoomControl::getZoomMin()
{
	XOJ_CHECK_TYPE(ZoomControl);

	return this->zoomMin;
}

void ZoomControl::setZoomMin(double zoomMin)
{
	XOJ_CHECK_TYPE(ZoomControl);

	this->zoomMin = zoomMin;
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
		GtkWidget* topLevel = gtk_widget_get_toplevel(view->getWidget());
		int wx = 0;
		int wy = 0;
		gtk_widget_translate_coordinates(view->getWidget(), topLevel, 0, 0, &wx, &wy);

		if (event->direction == GDK_SCROLL_UP ||
			(event->direction == GDK_SCROLL_SMOOTH && event->delta_y > 0))
		{
			zoomOut(event->x + wx, event->y + wy);
		}
		else if (event->direction == GDK_SCROLL_DOWN ||
			(event->direction == GDK_SCROLL_SMOOTH && event->delta_y < 0))
		{
			zoomIn(event->x + wx, event->y + wy);
		}
		return true;
	}

	return false;
}
