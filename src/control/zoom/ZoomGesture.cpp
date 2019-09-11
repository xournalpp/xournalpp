#include "ZoomGesture.h"
#include "ZoomControl.h"

ZoomGesture::ZoomGesture(ZoomControl* zoomControl)
 : zoomControl(zoomControl)
{
}

ZoomGesture::~ZoomGesture()
{
	g_object_unref(this->gesture);
	this->gesture = nullptr;
}

void ZoomGesture::connect(GtkWidget* parent)
{
	this->gesture = gtk_gesture_zoom_new(parent);

	gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(this->gesture), GTK_PHASE_CAPTURE);

	g_signal_connect(this->gesture, "begin", G_CALLBACK(
			+[](GtkGesture* gesture, GdkEventSequence* sequence, ZoomGesture* self)
			{
	self->zoomBegin();
			}), this);

	g_signal_connect(this->gesture, "scale-changed", G_CALLBACK(
			+[](GtkGestureZoom* gesture, gdouble scale, ZoomGesture* self)
			{
	self->zoomChanged(scale);
			}), this);

	g_signal_connect(this->gesture, "end", G_CALLBACK(
			+[](GtkGesture* gesture, GdkEventSequence* sequence, ZoomGesture* self)
			{
	self->zoomEnd();
			}), this);
}

bool ZoomGesture::isGestureActive()
{
	return gestureActive;
}

void ZoomGesture::disable()
{
	this->enabled = false;
	this->gestureActive = false;

	// GTK should call this method anyway but this is to make sure
	zoomControl->endZoomSequence();
}

void ZoomGesture::enable()
{
	this->enabled = true;
}

void ZoomGesture::zoomBegin()
{
	if(!enabled || zoomControl->isZoomPresentationMode())
	{
		return;
	}

	gestureActive = true;

	double x = 0;
	double y = 0;
	// get center of bounding box
	gtk_gesture_get_bounding_box_center(GTK_GESTURE(gesture), &x, &y);

	Rectangle zoomSequenceRectangle = zoomControl->getVisibleRect();

	if(zoomControl->isZoomFitMode())
	{
		zoomControl->setZoomFitMode(false);
	}
	this->zoomControl->getZoom();
	zoomControl->startZoomSequence(x - zoomSequenceRectangle.x, y - zoomSequenceRectangle.y);
}

void ZoomGesture::zoomChanged(double zoom)
{
	if(!enabled || zoomControl->isZoomPresentationMode())
	{
		return;
	}

	if (gestureActive)
	{
		zoomControl->zoomSequenceChange(zoom, true);
	}
}

void ZoomGesture::zoomEnd()
{
	if(!enabled || zoomControl->isZoomPresentationMode())
	{
		return;
	}

	gestureActive = false;

	zoomControl->endZoomSequence();
}
