#include "ZoomControl.h"

#include "control/Control.h"
#include "gui/Layout.h"
#include "gui/PageView.h"
#include "gui/widgets/XournalWidget.h"
#include "gui/XournalView.h"

ZoomControl::ZoomControl()
{
	this->zoomStep = DEFAULT_ZOOM_STEP * this->zoom100Value;
	this->zoomStepScroll = DEFAULT_ZOOM_STEP_SCROLL * this->zoom100Value;
	this->zoomMax = DEFAULT_ZOOM_MAX * this->zoom100Value;
	this->zoomMin = DEFAULT_ZOOM_MIN * this->zoom100Value;
}

ZoomControl::~ZoomControl()
{
}

void ZoomControl::zoomOneStep(bool zoomIn, double x, double y)
{
	if(this->zoomPresentationMode)
	{
		return;
	}

	startZoomSequence(x, y);

	if (this->zoomFitMode)
	{
		this->setZoomFitMode(false);
	}

	double newZoom;
	if (zoomIn)
	{
		newZoom = this->zoom + this->zoomStep;
	}
	else
	{
		newZoom = this->zoom - this->zoomStep;
	}
	this->zoomSequenceChange(newZoom, false);

	endZoomSequence();
}

void ZoomControl::zoomScroll(bool zoomIn, double x, double y)
{
	if(this->zoomPresentationMode)
	{
		return;
	}

	if (this->zoomFitMode)
	{
		this->setZoomFitMode(false);
	}

	if (this->zoomSequenceStart == -1 || scrollCursorPositionX != x || scrollCursorPositionY != y)
	{
		scrollCursorPositionX = x;
		scrollCursorPositionY = y;
		startZoomSequence(x, y);
	}

	double newZoom;
	if (zoomIn)
	{
		newZoom = this->zoom + this->zoomStepScroll;
	}
	else
	{
		newZoom = this->zoom - this->zoomStepScroll;
	}
	this->zoomSequenceChange(newZoom, false);
}

/**
 * Call this before any zoom is done, it saves the current page and position
 *
 * @param centerX Zoom Center X (use -1 for center of the visible rect)
 * @param centerY Zoom Center Y (use -1 for center of the visible rect)
 */
void ZoomControl::startZoomSequence(double centerX, double centerY)
{
	Rectangle rect = getVisibleRect();
	if (centerX == -1 || centerY == -1)
	{
		this->zoomWidgetPosX = rect.width / 2;
		this->zoomWidgetPosY = rect.height / 2;
	}
	else
	{
		this->zoomWidgetPosX = centerX;
		this->zoomWidgetPosY = centerY;
	}

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
void ZoomControl::zoomSequenceChange(double zoom, bool relative)
{
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

void ZoomControl::setScrollPositionAfterZoom(double x, double y)
{
	this->scrollPositionX = (x + this->zoomWidgetPosX) / this->zoom;
	this->scrollPositionY = (y + this->zoomWidgetPosY) / this->zoom;
}

/**
 * Zoom to correct position on zooming
 */
std::tuple<double, double> ZoomControl::getScrollPositionAfterZoom()
{
	if (this->zoomSequenceStart == -1 )
	{
		return std::make_tuple(-1,-1);
	}

	double x = (this->scrollPositionX * this->zoom) - this->zoomWidgetPosX;
	double y = (this->scrollPositionY * this->zoom) - this->zoomWidgetPosY;
	return std::make_tuple(x, y);
}


void ZoomControl::addZoomListener(ZoomListener* listener)
{
	this->listener.push_back(listener);
}

void ZoomControl::initZoomHandler(GtkWidget* widget, XournalView* view, Control* control)
{
	g_signal_connect(widget, "scroll_event", G_CALLBACK(onScrolledwindowMainScrollEvent), this);

	g_signal_connect(widget, "size-allocate", G_CALLBACK(onWidgetSizeChangedEvent), this);

	registerListener(control);
	this->view = view;
	this->control = control;
}

void ZoomControl::fireZoomChanged()
{
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
	for (ZoomListener* z : this->listener)
	{
		z->zoomRangeValuesChanged();
	}
}

double ZoomControl::getZoom()
{
	return this->zoom;
}

double ZoomControl::getZoomReal()
{
	return this->zoom / this->zoom100Value;
}

void ZoomControl::setZoom(double zoom)
{
	this->zoom = zoom;
	fireZoomChanged();
}

void ZoomControl::setZoom100Value(double zoom)
{
	this->zoom100Value = zoom;
	setZoomStep(this->zoomStepReal);
	setZoomStepScroll(this->zoomStepScrollReal);
	this->zoomMax = this->zoomMaxReal * zoom;
	this->zoomMin = this->zoomMinReal * zoom;
	fireZoomRangeValueChanged();
}

bool ZoomControl::updateZoomFitValue(size_t pageNo)
{
	return updateZoomFitValue(getVisibleRect(), pageNo);
}

bool ZoomControl::updateZoomFitValue(const Rectangle& widget_rect, size_t pageNo)
{
	if(pageNo == 0)
	{
		pageNo = view->getCurrentPage();
	}
	XojPageView* page = view->getViewFor(pageNo);
	if(!page)
	{
		//no page
		return false;
	}

	double zoom_fit_width = widget_rect.width / (page->getWidth() + 20.0);
	if(zoom_fit_width < this->zoomMin || zoom_fit_width > this->zoomMax)
	{
		return false;
	}

	this->zoomFitValue = zoom_fit_width;
	fireZoomRangeValueChanged();
	if(this->zoomFitMode && !this->zoomPresentationMode)
	{
		this->setZoomFitMode(true);
	}
	return true;
}

double ZoomControl::getZoomFitValue()
{
	return this->zoomFitValue;
}

bool ZoomControl::updateZoomPresentationValue(size_t pageNo)
{
	XojPageView* page = view->getViewFor(view->getCurrentPage());
	if(!page)
	{
		//no page
		return false;
	}

	Rectangle widget_rect = getVisibleRect();
	double zoom_fit_width = widget_rect.width / (page->getWidth() + 14.0);
	double zoom_fit_height = widget_rect.height / (page->getHeight() + 14.0);
	double zoom_presentation = zoom_fit_width < zoom_fit_height ? zoom_fit_width : zoom_fit_height;
	if(zoom_presentation < this->zoomMin)
	{
		return false;
	}

	this->zoomPresentationValue = zoom_presentation;
	if(this->zoomPresentationMode)
	{
		this->setZoomPresentationMode(true);
	}
	return true;
}

double ZoomControl::getZoomPresentationValue()
{
	return this->zoomPresentationValue;
}

double ZoomControl::getZoom100Value()
{
	return this->zoom100Value;
}

void ZoomControl::zoom100()
{
	if(this->zoomPresentationMode)
	{
		return;
	}

	if(this->zoomFitMode)
	{
		this->setZoomFitMode(false);
	}

	startZoomSequence(-1, -1);
	this->zoomSequenceChange(this->zoom100Value, false);
	endZoomSequence();
}

void ZoomControl::zoomFit()
{
	if(this->zoomFitMode && !this->zoomPresentationMode && this->zoom != this->zoomFitValue)
	{
		startZoomSequence(-1, -1);
		this->zoomSequenceChange(this->zoomFitValue, false);
		endZoomSequence();
	}
}

void ZoomControl::zoomPresentation()
{
	if(this->zoomPresentationMode && this->zoom != this->zoomPresentationValue)
	{
		startZoomSequence(-1, -1);
		this->zoomSequenceChange(this->zoomPresentationValue, false);
		endZoomSequence();
	}
}

void ZoomControl::setZoomFitMode(bool isZoomFitMode)
{
	if(this->zoomFitMode != isZoomFitMode)
	{
		this->zoomFitMode = isZoomFitMode;
		this->control->fireActionSelected(GROUP_ZOOM_FIT, isZoomFitMode ? ACTION_ZOOM_FIT : ACTION_NOT_SELECTED);
	}

	if(isZoomFitMode)
	{
		zoomFit();
	}
}

bool ZoomControl::isZoomFitMode()
{
	return this->zoomFitMode;
}

void ZoomControl::setZoomPresentationMode(bool isZoomPresentationMode)
{
	this->zoomPresentationMode = isZoomPresentationMode;

	if(isZoomPresentationMode)
	{
		zoomPresentation();
	}
}

bool ZoomControl::isZoomPresentationMode()
{
	return this->zoomPresentationMode;
}

double ZoomControl::getZoomStep()
{
	return this->zoomStep;
}

double ZoomControl::getZoomStepReal()
{
	return this->zoomStepReal;
}

void ZoomControl::setZoomStep(double zoomStep)
{
	this->zoomStepReal = zoomStep;
	this->zoomStep = zoomStep * this->zoom100Value;
}

double ZoomControl::getZoomStepScroll()
{
	return this->zoomStepScroll;
}

double ZoomControl::getZoomStepScrollReal()
{
	return this->zoomStepScrollReal;
}

void ZoomControl::setZoomStepScroll(double zoomStep)
{
	this->zoomStepScrollReal = zoomStep;
	this->zoomStepScroll = zoomStep * this->zoom100Value;
}

double ZoomControl::getZoomMax()
{
	return this->zoomMax;
}

double ZoomControl::getZoomMaxReal()
{
	return this->zoomMaxReal;
}

void ZoomControl::setZoomMax(double zoomMax)
{
	this->zoomMaxReal = zoomMax;
	this->zoomMax = zoomMax * this->zoom100Value;
}

double ZoomControl::getZoomMin()
{
	return this->zoomMin;
}

double ZoomControl::getZoomMinReal()
{
	return this->zoomMinReal;
}

void ZoomControl::setZoomMin(double zoomMin)
{
	this->zoomMinReal = zoomMin;
	this->zoomMin = zoomMin * this->zoom100Value;
}

void ZoomControl::pageSizeChanged(size_t page)
{
	updateZoomPresentationValue(page);
	updateZoomFitValue(page);
}

void ZoomControl::pageSelected(size_t page)
{
	updateZoomPresentationValue(page);
	updateZoomFitValue(page);
}

bool ZoomControl::onScrolledwindowMainScrollEvent(GtkWidget* widget, GdkEventScroll* event, ZoomControl* zoom)
{
	guint state = event->state & gtk_accelerator_get_default_mod_mask();

	// do not handle e.g. ALT + Scroll (e.g. Compiz use this shortcut for setting transparency...)
	if (state != 0 && (state & ~(GDK_CONTROL_MASK | GDK_SHIFT_MASK)))
	{
		return true;
	}

	if (state & GDK_CONTROL_MASK)
	{
		GtkWidget* topLevel = gtk_widget_get_toplevel(widget);
		int wx = 0;
		int wy = 0;
		gtk_widget_translate_coordinates(widget, topLevel, 0, 0, &wx, &wy);

		if (event->direction == GDK_SCROLL_UP ||
			(event->direction == GDK_SCROLL_SMOOTH && event->delta_y < 0))
		{
			zoom->zoomScroll(ZOOM_IN, event->x + wx, event->y + wy);
		}
		else if (event->direction == GDK_SCROLL_DOWN ||
			(event->direction == GDK_SCROLL_SMOOTH && event->delta_y > 0))
		{
			zoom->zoomScroll(ZOOM_OUT, event->x + wx, event->y + wy);
		}
		return true;
	}

	//TODO: Disabling scroll here is maybe a bit hacky
	if(zoom->isZoomPresentationMode())
	{
		//disable scroll while presentationMode
		return true;
	}

	return false;
}


// Todo: try to connect this function with the "expose_event", it would be way cleaner and we dont need to allign/layout
//       the pages manually, but it only works with the top Widget (GtkWindow) for now this works fine
//       see https://stackoverflow.com/questions/1060039/gtk-detecting-window-resize-from-the-user
bool ZoomControl::onWidgetSizeChangedEvent(GtkWidget* widget, GdkRectangle* allocation, ZoomControl* zoom)
{
	g_assert_true(widget != zoom->view->getWidget());

	Rectangle r(allocation->x, allocation->y, allocation->width, allocation->height);

	zoom->updateZoomPresentationValue();
	zoom->updateZoomFitValue(r);

	auto layout = gtk_xournal_get_layout(zoom->view->getWidget());
	layout->layoutPages(allocation->width, allocation->height);
	gtk_widget_queue_resize(zoom->view->getWidget());

	return true;
}
