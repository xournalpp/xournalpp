#include "ZoomControl.h"

const double zoomStep = 0.2;

ZoomControl::ZoomControl() {
	listener = NULL;
	zoom = 1.0;
	zoom100Value = 1.0;
	zoomFitValue = 1.0;
}

ZoomControl::~ZoomControl() {
	g_list_free(listener);
}

void ZoomControl::addZoomListener(ZoomListener * listener) {
	this->listener = g_list_append(this->listener, listener);
}

void ZoomControl::initZoomHandler(GladeGui * gui) {
	g_signal_connect(gui->get("scrolledwindowMain"), "scroll_event", G_CALLBACK(onScrolledwindowMainScrollEvent), this);

}

void ZoomControl::fireZoomChanged(double lastZoom) {
	if (zoom < 0.3) {
		zoom = 0.3;
	}

	if (zoom > 5) {
		zoom = 5;
	}

	for (GList * l = listener; l != NULL; l = l->next) {
		ZoomListener * z = (ZoomListener *) l->data;
		z->zoomChanged(lastZoom);
	}
}

void ZoomControl::fireZoomRangeValueChanged() {
	for (GList * l = listener; l != NULL; l = l->next) {
		ZoomListener * z = (ZoomListener *) l->data;
		z->zoomRangeValuesChanged();
	}
}

double ZoomControl::getZoom() {
	return zoom;
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
	double lastZoom = zoom;
	zoom = zoom100Value;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomFit() {
	double lastZoom = zoom;
	zoom = zoomFitValue;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomIn() {
	double lastZoom = zoom;
	zoom += zoomStep;
	fireZoomChanged(lastZoom);
}

void ZoomControl::zoomOut() {
	double lastZoom = zoom;
	zoom -= zoomStep;
	fireZoomChanged(lastZoom);
}

bool ZoomControl::onScrolledwindowMainScrollEvent(GtkWidget *widget, GdkEventScroll *event, ZoomControl * zoom) {
	guint state;

	state = event->state & gtk_accelerator_get_default_mod_mask();

	if (state == GDK_CONTROL_MASK) {
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

