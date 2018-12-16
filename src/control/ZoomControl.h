/*
 * Xournal++
 *
 * Controls the zoom level
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>
#include <vector>

//Hardcode max and min zoom
//TODO:this should probably be user-adjustable in future
#define DEFAULT_ZOOM_MAX 5
#define DEFAULT_ZOOM_MIN 0.3
#define DEFAULT_ZOOM_STEP 0.1
#define MIN_ZOOM_STEP 0.05
#define ZOOM_EPSILON 0.0001

class ZoomListener
{
public:
	virtual void zoomChanged(double lastZoom) = 0;
	virtual void zoomRangeValuesChanged();

	virtual ~ZoomListener();
};

class ZoomControl
{
public:
	ZoomControl();
	virtual ~ZoomControl();

	void zoomIn();
	void zoomOut();

	void zoomFit();
	void zoom100();

	double getZoom();
	void setZoom(double zoom);

	void setZoom100(double zoom);
	void setZoomFit(double zoom);

	double getZoomFit();
	double getZoom100();

	void addZoomListener(ZoomListener* listener);

	void initZoomHandler(GtkWidget* widget);

	// TODO: Naming and getter / setter!
	// Current zoom center
	double getZoomCenterX();
	void setZoomCenterX(double zoomCenterX);

	double getZoomCenterY();
	void setZoomCenterY(double zoomCenterY);

	double getZoomStep();
	void setZoomStep(double zoomStep);

	double getZoomMax();
	void setZoomMax(double zoomMax);

	double getZoomMin();
	void setZoomMin(double zoomMin);

protected:
	void fireZoomChanged(double lastZoom);
	void fireZoomRangeValueChanged();

	static bool onScrolledwindowMainScrollEvent(GtkWidget* widget, GdkEventScroll* event, ZoomControl* zoom);

private:
	XOJ_TYPE_ATTRIB;

	std::vector<ZoomListener*> listener;

	double zoom;

	double lastZoomValue;

	bool zoomFitMode;

	double zoom100Value;
	double zoomFitValue;

	double zoomCenterX;
	double zoomCenterY;

	double zoomStep;
	double zoomMax;
	double zoomMin;
};
