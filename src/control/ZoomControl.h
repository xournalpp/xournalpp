/*
 * Xournal++
 *
 * Controls the zoom level
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <gtk/gtk.h>

#include <XournalType.h>

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

protected:
	void fireZoomChanged(double lastZoom);
	void fireZoomRangeValueChanged();

	static bool onScrolledwindowMainScrollEvent(GtkWidget* widget,
												GdkEventScroll* event, ZoomControl* zoom);

private:
	XOJ_TYPE_ATTRIB;

	GList* listener;

	double zoom;

	double lastZoomValue;

	bool zoomFitMode;

	double zoom100Value;
	double zoomFitValue;
};
