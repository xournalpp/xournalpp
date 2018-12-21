#include "ZoomGesture.h"
#include "ZoomControl.h"

ZoomGesture::ZoomGesture(GtkWidget* parent, ZoomControl* zoomControl)
 : zoomControl(zoomControl),
   gestureActive(false)
{
	XOJ_INIT_TYPE(ZoomGesture);

	this->gesture = gtk_gesture_zoom_new(parent);

	gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(this->gesture), GTK_PHASE_CAPTURE);

	g_signal_connect(this->gesture, "begin", G_CALLBACK(
		+[](GtkGesture* gesture, GdkEventSequence* sequence, ZoomGesture* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, ZoomGesture);
			self->zoomBegin();
		}), this);

	g_signal_connect(this->gesture, "scale-changed", G_CALLBACK(
		+[](GtkGestureZoom* gesture, gdouble scale, ZoomGesture* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, ZoomGesture);
			self->zoomChanged(scale);
		}), this);

	g_signal_connect(this->gesture, "end", G_CALLBACK(
		+[](GtkGesture* gesture, GdkEventSequence* sequence, ZoomGesture* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, ZoomGesture);
			self->zoomEnd();
		}), this);
}

ZoomGesture::~ZoomGesture()
{
	XOJ_CHECK_TYPE(ZoomGesture);

	g_object_unref(this->gesture);
	this->gesture = NULL;

	XOJ_RELEASE_TYPE(ZoomGesture);
}

bool ZoomGesture::isGestureActive()
{
	XOJ_CHECK_TYPE(ZoomGesture);

	return gestureActive;
}

void ZoomGesture::zoomBegin()
{
	XOJ_CHECK_TYPE(ZoomGesture);

	gestureActive = true;

	double x = 0;
	double y = 0;
	// get center of bounding box
	gtk_gesture_get_bounding_box_center(GTK_GESTURE(gesture), &x, &y);

	zoomControl->startZoomSequence(x, y);
}

void ZoomGesture::zoomChanged(double zoom)
{
	XOJ_CHECK_TYPE(ZoomGesture);

	zoomControl->zoomSequnceChange(zoom, true);
}

void ZoomGesture::zoomEnd()
{
	XOJ_CHECK_TYPE(ZoomGesture);

	gestureActive = false;

	zoomControl->endZoomSequence();
}
