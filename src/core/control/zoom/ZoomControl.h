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

#include <cstddef>  // for size_t
#include <vector>   // for vector

#include <gdk/gdk.h>  // for GdkEvent, GdkEventScroll, GdkEve...
#include <gtk/gtk.h>  // for GtkWidget

#include "model/DocumentListener.h"  // for DocumentListener
#include "util/Point.h"              // for Point
#include "util/Rectangle.h"          // for Rectangle

constexpr auto DEFAULT_ZOOM_MAX{7};
constexpr auto DEFAULT_ZOOM_MIN{0.3};
constexpr auto DEFAULT_ZOOM_STEP{0.1};
constexpr auto DEFAULT_ZOOM_STEP_SCROLL{0.01};

enum ZoomDirection : bool { ZOOM_OUT = false, ZOOM_IN = true };

class XournalView;
class Control;
class ZoomListener;

class ZoomControl: public DocumentListener {
public:
    ZoomControl() = default;
    ~ZoomControl() override = default;

    /**
     * Zoom one step
     *
     * @param direction direction to zoom in or out
     * @param zoomCenter position of zoom focus
     */
    void zoomOneStep(ZoomDirection direction, xoj::util::Point<double> zoomCenter);
    void zoomOneStep(ZoomDirection direction);

    /**
     * Zoom one step
     *
     * @param direction to zoom in or out
     * @param zoomCenter position of zoom focus
     */
    void zoomScroll(double delta, xoj::util::Point<double> zoomCenter);

    /**
     * Zoom so that the page fits the current size of the window
     */
    void setZoomFitMode(bool isZoomFitMode);
    bool isZoomFitMode() const;

    /**
     * Zoom so that the document completely fits the View.
     */
    void setZoomPresentationMode(bool isZoomPresentationMode);
    bool isZoomPresentationMode() const;

    /**
     * Zoom so that the displayed page on the screen has the same size as the real size
     * The dpi has to be set correctly
     */
    void zoom100();

    /**
     * @return zoom value depending zoom100Value
     */
    double getZoom() const;

    /**
     * @return real zoom value in percent
     */
    double getZoomReal() const;

    /**
     * Set the current zoom, does not preserve the current page position.
     * Use startZoomSequence() / zoomSequnceChange() / endZoomSequence() to preserve position
     * e.g. use zoomOneStep function
     *
     * @param zoomI zoom value depending zoom100Value
     */
    void setZoom(double zoomI);

    /**
     * Updates the when dpi is changed.
     * updates zoomMax, zoomMin, zoomStepBig, zoomStepScroll
     *
     * @param zoom100Val zoom value depending zoom100Value
     */
    void setZoom100Value(double zoom100Val);

    /**
     * @return zoom value for zoom 100% depending zoom100Value
     */
    double getZoom100Value() const;

    /**
     * Updates when, the window size changes
     * @param zoom zoom value depending zoom100Value
     */
    bool updateZoomFitValue(size_t pageNo = 0);

    /**
     * @return zoom value for zoom fit depending zoom100Value
     */
    double getZoomFitValue() const;

    bool updateZoomPresentationValue(size_t pageNo = 0);

    void addZoomListener(ZoomListener* listener);
    void removeZoomListener(ZoomListener* listener);

    void initZoomHandler(GtkScrolledWindow* scrolledWindow, XournalView* v, Control* c);

    /**
     * Call this before any zoom is done, it saves the current page and position
     *
     * @param zoomCenter position of zoom focus in GtkScrolledWindow's pixel coordinates. That is, absolute coordinates
     * (not scaling with the document) translated to the top left corner of the drawing area.
     */
    void startZoomSequence(xoj::util::Point<double> zoomCenter);

    /**
     * Call this before any zoom is done, it saves the current page and position
     * zooms to the center of the visible rect
     */
    void startZoomSequence();

    /**
     * Change the zoom within a Zoom sequence (startZoomSequence() / endZoomSequence())
     *
     * @param zoom Zoom value
     * @param relative If true, multiplies the current zoom value by `zoom`. Otherwise replaces it with `zoom`.
     */
    void zoomSequenceChange(double zoom, bool relative);

    /**
     * Change the zoom and zoomCenter within a Zoom sequence (startZoomSequence() / endZoomSequence())
     * Used while touch pinch event
     *
     * @param relativeZoom Multiplies the current zoom value by this value
     * @param newZoomCenter New center in absolute coordinate (e.g. GtkScrolledWindow's coordinates)
     */
    void zoomSequenceChange(double relativeZoom, xoj::util::Point<double> newZoomCenter);

    /// Clear all stored data from startZoomSequence()
    void endZoomSequence();

    /// Revert and end the current zoom sequence
    void cancelZoomSequence();

    /// Check if we are between calls to `startZoomSequence()` and `endZoomSequence()`
    bool isZoomSequenceActive() const;

    /**
     * Zoom to correct position on zooming.
     * This function should only be called during a zoom sequence.
     */
    xoj::util::Point<double> getScrollPositionAfterZoom() const;

    /// Get visible rect on xournal view, for Zoom Gesture
    xoj::util::Rectangle<double> getVisibleRect();

    void setZoomStep(double zoomStep);

    void setZoomStepScroll(double zoomStep);

protected:
    void fireZoomChanged();
    void fireZoomRangeValueChanged();

    void pageSizeChanged(size_t page) override;
    void pageSelected(size_t page) override;

private:
    void zoomFit();
    void zoomPresentation();

    /**
     * Get this->zoom changed by a step in the given direction.
     *
     * @param direction Direction to change the zoom (step direction).
     * @param stepSize Size of the step to take
     * @return The new zoom if a step is taken in the given direction.
     */
    double withZoomStep(ZoomDirection direction, double stepSize) const;

private:
    XournalView* view = nullptr;
    Control* control = nullptr;
    std::vector<ZoomListener*> listener;

    /**
     * current Zoom value
     * depends dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoom = 1.0;
    bool zoomFitMode = false;
    bool zoomPresentationMode = false;

    /// Zoom value for 100% depends on the dpi
    double zoom100Value = 1.0;
    double zoomFitValue = 1.0;
    double zoomPresentationValue = 1.0;

    /// Position of the pointer in absolute coordinates (i.e. GtkScrolledWindow's coordinates)
    ///  -- Automatically updated by callback
    xoj::util::Point<double> pointerWidgetPosition;

    /// Base zoom on start, for relative zoom (Gesture)
    double zoomSequenceStart = -1;

    /// Zoom center position in ScrolledWindow coordinate space
    xoj::util::Point<double> zoomWidgetPos;

    /// Scroll position (weird value in unknown coordinate system) Todo: Settle this
    xoj::util::Point<double> scrollPosition;

    /// Size {x, y} of the pixels before the current page that do not scale.
    xoj::util::Point<double> unscaledPixels;

    /**
     * Zoomstep value for Ctrl - and Zoom In and Out Button
     * depends on dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoomStep = DEFAULT_ZOOM_STEP;

    /**
     * Zoomstep value for Ctrl-Scroll zooming
     * depends on dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoomStepScroll = DEFAULT_ZOOM_STEP_SCROLL;

    /**
     * Zoom maximal value
     * depends on dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoomMax = DEFAULT_ZOOM_MAX;

    /**
     * Zoom mininmal value
     * depends on dpi (REAL_PERCENTAGE_VALUE * zoom100Value)
     */
    double zoomMin = DEFAULT_ZOOM_MIN;

    size_t current_page = static_cast<size_t>(-1);
    size_t last_page = static_cast<size_t>(-1);
    bool isZoomFittingNow = false;
};
