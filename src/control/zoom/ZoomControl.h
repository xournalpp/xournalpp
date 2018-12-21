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

#include <Rectangle.h>

#include <gtk/gtk.h>
#include <vector>

// Hardcode max and min zoom
// this should probably be user-adjustable in future
#define MAX_ZOOM 5
#define MIN_ZOOM 0.3

class XournalView;
class XojPageView;
class ZoomListener;

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

	/**
	 * Set the current zoom, does not preserve the current page position.
	 * Use startZoomSequence() / zoomSequnceChange() / endZoomSequence() to preserve position
	 */
	void setZoom(double zoom);

	void setZoom100(double zoom);
	void setZoomFit(double zoom);

	double getZoomFit();
	double getZoom100();

	void addZoomListener(ZoomListener* listener);

	void initZoomHandler(GtkWidget* widget, XournalView* view);

	/**
	 * Call this before any zoom is done, it saves the current page and position
	 *
	 * @param centerX Zoom Center X (use -1 for center of the visible rect)
	 * @param centerY Zoom Center Y (use -1 for center of the visible rect)
	 */
	void startZoomSequence(double centerX, double centerY);

	/**
	 * Change the zoom within a Zoom sequnce (startZoomSequence() / endZoomSequence())
	 *
	 * @param zoom Current zoom value
	 * @param realative If the zoom is realative to the start value (for Gesture)
	 */
	void zoomSequnceChange(double zoom, bool realative);

	/**
	 * Clear all stored data from startZoomSequence()
	 */
	void endZoomSequence();

	/**
	 * Zoom to correct position on zooming
	 */
	void scrollToZoomPosition(XojPageView* view, double lastZoom);

protected:
	void fireZoomChanged(double lastZoom);
	void fireZoomRangeValueChanged();

	bool onScrolledwindowMainScrollEvent(GdkEventScroll* event);

private:
	XOJ_TYPE_ATTRIB;

	XournalView* view;

	std::vector<ZoomListener*> listener;

	double zoom;

	double lastZoomValue;

	bool zoomFitMode;

	double zoom100Value;
	double zoomFitValue;



	// TODO: Naming and getter / setter!
	// Current zoom center
	gdouble zoom_center_x;
	gdouble zoom_center_y;

	/**
	 * Zoom at zoom sequence start
	 */
	double zoomSequenceStart;

	/**
	 * Zoom sequence rectangle
	 */
	Rectangle zoomSequenceRectangle;
};
