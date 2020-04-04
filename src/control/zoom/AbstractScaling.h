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

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <gtk/gtk.h>

#include "model/DocumentListener.h"

#include "Rectangle.h"
#include "XournalType.h"

class XournalView;
class Control;
class XojPageView;
class ScaleListener;
class DocumentListener;

class AbstractScaling: public DocumentListener {
public:
    AbstractScaling(Control* control);

    /**
     * Zoom one step
     *
     * @param zoomIn zoom in or out
     * @param x x position of focus to zoom
     * @param y y position of focus to zoom
     */
    void zoomOneStep(bool zoomIn, double x = -1, double y = -1);

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
    void zoomSequenceChange(double zoom, bool relative);

    /**
     * Clear all stored data from startZoomSequence()
     */
    void endZoomSequence();

    /**
     * Returns the current scale at which content is displayed.
     *
     * The returned value is always relative to the display dpi.
     * @return scale
     */
    virtual auto getScale() -> double const = 0;
    virtual auto setScale(double scale) -> void = 0;

private:
    Control* control = nullptr;
    std::vector<std::weak_ptr<ScaleListener>> listener{};
};
