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

#define DEFAULT_ZOOM_MAX 7
#define DEFAULT_ZOOM_MIN 0.3
#define DEFAULT_ZOOM_STEP 0.04

class XournalView;
class XojPageView;
class ZoomListener;

class ZoomControl
{
public:
	ZoomControl();
	virtual ~ZoomControl();

	void zoomIn(double x = -1, double y = -1);
	void zoomOut(double x = -1, double y = -1);

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
	 * Change the zoom within a Zoom sequence (startZoomSequence() / endZoomSequence())
	 *
	 * @param zoom Current zoom value
	 * @param relative If the zoom is relative to the start value (for Gesture)
	 */
	void zoomSequnceChange(double zoom, bool relative);

	/**
	 * Clear all stored data from startZoomSequence()
	 */
	void endZoomSequence();

	/**
	 * Zoom to correct position on zooming
	 */
	void scrollToZoomPosition(XojPageView* view);

	/**
	 * Get visible rect on xournal view, for Zoom Gesture
	 */
	Rectangle getVisibleRect();

	double getZoomStep();
	void setZoomStep(double zoomStep);

	double getZoomMax();
	void setZoomMax(double zoomMax);

	double getZoomMin();
	void setZoomMin(double zoomMin);

protected:
	void fireZoomChanged();
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

	/**
	 * Zoom point on widget, will not be zoomed!
	 */
	double zoomWidgetPosX;

	/**
	 * Zoom point on widget, will not be zoomed!
	 */
	double zoomWidgetPosY;

	/**
	 * Scroll position to scale
	 */
	double scrollPositionX;

	/**
	 * Scroll position to scale
	 */
	double scrollPositionY;

	/**
	 * Base zoom on start, for relative zoom (Gesture)
	 */
	double zoomSequenceStart;

	double zoomStep;
	double zoomMax;
	double zoomMin;
};
