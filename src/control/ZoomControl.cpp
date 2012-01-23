#include "ZoomControl.h"

const double zoomStep = 0.2;

ZoomControl::ZoomControl() {
	XOJ_INIT_TYPE(ZoomControl);

	this->listener = NULL;
	this->zoom = 1.0;
	this->lastZoomValue = 1.0;
	this->zoom100Value = 1.0;
	this->zoomFitValue = 1.0;
	this->zoomFitMode = true;
}

ZoomControl::~ZoomControl() {
	XOJ_CHECK_TYPE(ZoomControl);

	g_list_free(this->listener);
	this->listener = NULL;

	XOJ_RELEASE_TYPE(ZoomControl);
}

void ZoomControl::addZoomListener(ZoomListener * listener) {
	XOJ_CHECK_TYPE(ZoomControl);

	this->listener = g_list_append(this->listener, listener);
}

void ZoomControl::initZoomHandler(GtkWidget * widget) {
	XOJ_CHECK_TYPE(ZoomControl);

	g_signal_connect(widget, "scroll_event", G_CALLBACK(onScrolledwindowMainScrollEvent), this);
}

void ZoomControl::fireZoomChanged(double lastZoom) {
	XOJ_CHECK_TYPE(ZoomControl);

	if (this->zoom < 0.3) {
		this->zoom = 0.3;
	}

	if (this->zoom > 5) {
		this->zoom = 5;
	}

	for (GList * l = this->listener; l != NULL; l = l->next) {
		ZoomListener * z = (ZoomListener *) l->data;
		z->zoomChanged(lastZoom);
	}
}

void ZoomControl::fireZoomRangeValueChanged() {
	XOJ_CHECK_TYPE(ZoomControl);

	for (GList * l = this->listener; l != NULL; l = l->next) {
		ZoomListener * z = (ZoomListener *) l->data;
		z->zoomRangeValuesChanged();
	}
}

double ZoomControl::getZoom() {
	XOJ_CHECK_TYPE(ZoomControl);

	return this->zoom;
}

void ZoomControl::setZoom(double zoom) {
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = zoom;
	this->zoom = zoom;
	this->zoomFitMode = false;
	fireZoomChanged(lastZoom);
}

void ZoomControl::setZoom100(double zoom) {
	XOJ_CHECK_TYPE(ZoomControl);

	this->zoom100Value = zoom;
	fireZoomRangeValueChanged();
}

void ZoomControl::setZoomFit(double zoom) {
	XOJ_CHECK_TYPE(ZoomControl);

	this->zoomFitValue = zoom;
	fireZoomRangeValueChanged();

	if(this->zoomFitMode) {
		double lastZoom = this->zoom;
		this->zoom = this->zoomFitValue;
		fireZoomChanged(lastZoom);
	}
}

double ZoomControl::getZoomFit() {
	XOJ_CHECK_TYPE(ZoomControl);

	return this->zoomFitValue;
}

double ZoomControl::getZoom100() {
	XOJ_CHECK_TYPE(ZoomControl);

	return this->zoom100Value;
}

void ZoomControl::zoom100() {
	XOJ_CHECK_TYPE( ZoomControl);

	double lastZoom = this->zoom;
	this->zoom = this->zoom100Value;
	this->zoomFitMode = false;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomFit() {
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = this->zoom;
	this->zoom = this->zoomFitValue;
	this->zoomFitMode = true;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomIn() {
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = this->zoom;
	this->zoom += zoomStep;
	this->zoomFitMode = false;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomOut() {
	XOJ_CHECK_TYPE(ZoomControl);

	double lastZoom = this->zoom;
	this->zoom -= zoomStep;
	this->zoomFitMode = false;
	fireZoomChanged(lastZoom);
}

bool ZoomControl::onScrolledwindowMainScrollEvent(GtkWidget * widget, GdkEventScroll * event, ZoomControl * zoom) {
	XOJ_CHECK_TYPE_OBJ(zoom, ZoomControl);

	guint state = event->state & gtk_accelerator_get_default_mod_mask();

	// do not handle e.g. ALT + Scroll (e.g. Compiz use this shortcut for setting transparency...)
	if (state != 0 && state & ~(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
		return true;
	}

	if (state & GDK_CONTROL_MASK) {
		if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_LEFT) {
			zoom->zoomIn();
		} else {
			zoom->zoomOut();
		}

		return true;
	}

	// Shift + Wheel scrolls the in the perpendicular direction
	if (state & GDK_SHIFT_MASK) {
		if (event->direction == GDK_SCROLL_UP) {
			event->direction = GDK_SCROLL_LEFT;
		} else if (event->direction == GDK_SCROLL_LEFT) {
			event->direction = GDK_SCROLL_UP;
		} else if (event->direction == GDK_SCROLL_DOWN) {
			event->direction = GDK_SCROLL_RIGHT;
		} else if (event->direction == GDK_SCROLL_RIGHT) {
			event->direction = GDK_SCROLL_DOWN;
		}

		event->state &= ~GDK_SHIFT_MASK;
		state &= ~GDK_SHIFT_MASK;
	}

	return false;
}

void ZoomListener::zoomRangeValuesChanged() {
}

