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

#define DEFAULT_ZOOM_MAX 7
#define DEFAULT_ZOOM_MIN 0.3
#define DEFAULT_ZOOM_STEP 0.1
#define DEFAULT_ZOOM_STEP_SCROLL 0.01
#define ZOOM_IN true
#define ZOOM_OUT false

class XournalView;
class XojPageView;
class ZoomListener;

class ZoomControl
{
public:
	ZoomControl();
	virtual ~ZoomControl();

	/**
	 * Zoom one step
	 *
	 * @param zoomIn zoom in or out
	 * @param x x position of focus to zoom
	 * @param y y position of focus to zoom
	 */
	void zoomOneStep(bool zoomIn, double x = -1, double y = -1);

	/**
	 * Zoom one step
	 *
	 * @param zoomIn zoom in or out
	 * @param x x position of focus to zoom
	 * @param y y position of focus to zoom
	 */
	void zoomScroll(bool zoomIn, double x, double y);

	/**
	 * Zoom so that the page fits the current size of the window
	 */
	void zoomFit();

	/**
	 * Zoom so that the displayed page on the screen has the same size as the real size
	 * The dpi has to be set correctly
	 */
	void zoom100();

	/**
	 * @return zoom value depending zoom100Value
	 */
	double getZoom();

	/**
	 * @return real zoom value in percent
	 */
	double getZoomReal();

	/**
	 * Set the current zoom, does not preserve the current page position.
	 * Use startZoomSequence() / zoomSequnceChange() / endZoomSequence() to preserve position
	 * e.g. use zoomOneStep function
	 *
	 * @param zoom zoom value depending zoom100Value
	 */
	void setZoom(double zoom);

	/**
	 * Updates the when dpi is changed.
	 * updates zoomMax, zoomMin, zoomStepBig, zoomStepScroll
	 *
	 * @param zoom zoom value depending zoom100Value
	 */
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
	double getZoomStepReal();
	void setZoomStep(double zoomStep);

	double getZoomStepScroll();
	double getZoomStepScrollReal();
	void setZoomStepScroll(double zoomStep);

	double getZoomMax();
	double getZoomMaxReal();
	void setZoomMax(double zoomMax);

	double getZoomMin();
	double getZoomMinReal();
	void setZoomMin(double zoomMin);

protected:
	void fireZoomChanged();
	void fireZoomRangeValueChanged();

	bool onScrolledwindowMainScrollEvent(GdkEventScroll* event);

private:
	XOJ_TYPE_ATTRIB;

	XournalView* view;

	std::vector<ZoomListener*> listener;

	/**
	 * current Zoom value
	 * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
	 */
	double zoom;

	/**
	 * for zoom sequence start zoom value
	 * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
	 */
	double lastZoomValue;

	bool zoomFitMode;

	/**
	 * Zoom value for 100% depends on the dpi
	 */
	double zoom100Value;
	double zoomFitValue;

	/**
	 * Base zoom on start, for relative zoom (Gesture)
	 */
	double zoomSequenceStart;

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
	 * Cursorposition x for Ctrl + Scroll
	 */
	double scrollCursorPositionX;
	/**
	 * Cursorposition y for Ctrl + Scroll
	 */
	double scrollCursorPositionY;

	/**
	 * Zoomstep value for Ctrl - and Zoom In and Out Button
	 * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
	 */
	double zoomStep;
	/**
	 * Real zoomstep value for Ctrl + and Zoom In and Out Button
	 */
	double zoomStepReal;

	/**
	 * Zoomstep value for Ctrl-Scroll zooming
	 * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
	 */
	double zoomStepScroll;
	/**
	 * Real zoomstep value for Ctrl-Scroll zooming
	 */
	double zoomStepScrollReal;


	/**
	 * Zoom maximal value
	 * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
	 */
	double zoomMax;
	/**
	 * Real zoom maximal value
	 */
	double zoomMaxReal;


	/**
	 * Zoom mininmal value
	 * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
	 */
	double zoomMin;
	/**
	 * Real zoom mininmal value
	 * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
	 */
	double zoomMinReal;
};
