#include "ZoomControl.h"

const double zoomStep = 0.2;
// TODO: AA: type check

ZoomControl::ZoomControl() {
	this->listener = NULL;
	this->zoom = 1.0;
	this->zoom100Value = 1.0;
	this->zoomFitValue = 1.0;
}

ZoomControl::~ZoomControl() {
	g_list_free(this->listener);
}

void ZoomControl::addZoomListener(ZoomListener * listener) {
	this->listener = g_list_append(this->listener, listener);
}

void ZoomControl::initZoomHandler(GtkWidget * widget) {
	g_signal_connect(widget, "scroll_event", G_CALLBACK(onScrolledwindowMainScrollEvent), this);
}

void ZoomControl::fireZoomChanged(double lastZoom) {
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
	for (GList * l = this->listener; l != NULL; l = l->next) {
		ZoomListener * z = (ZoomListener *) l->data;
		z->zoomRangeValuesChanged();
	}
}

double ZoomControl::getZoom() {
	return this->zoom;
}

void ZoomControl::setZoom(double zoom) {
	double lastZoom = zoom;
	this->zoom = zoom;
	fireZoomChanged(lastZoom);
}

void ZoomControl::setZoom100(double zoom) {
	this->zoom100Value = zoom;
	fireZoomRangeValueChanged();
}

void ZoomControl::setZoomFit(double zoom) {
	this->zoomFitValue = zoom;
	fireZoomRangeValueChanged();
}

double ZoomControl::getZoomFit() {
	return this->zoomFitValue;
}

double ZoomControl::getZoom100() {
	return this->zoom100Value;
}

void ZoomControl::zoom100() {
	double lastZoom = this->zoom;
	this->zoom = this->zoom100Value;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomFit() {
	double lastZoom = this->zoom;
	this->zoom = this->zoomFitValue;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomIn() {
	double lastZoom = this->zoom;
	this->zoom += zoomStep;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomOut() {
	double lastZoom = zoom;
	zoom -= zoomStep;
	fireZoomChanged(lastZoom);
}

bool ZoomControl::onScrolledwindowMainScrollEvent(GtkWidget * widget, GdkEventScroll * event, ZoomControl * zoom) {
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

