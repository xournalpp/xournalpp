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
//this should probably be user-adjustable in future
#define MAX_ZOOM 5
#define MIN_ZOOM 0.3

class ZoomListener
{
public:
	virtual void zoomChanged(double lastZoom) = 0;
	virtual void zoomRangeValuesChanged();
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
	gdouble zoom_center_x;
	gdouble zoom_center_y;

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
};
