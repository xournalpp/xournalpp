/*
 * Xournal++
 *
 * Handle touchscreen zooming and panning.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gdk/gdk.h>  // for GdkEventSequence

#include "util/Point.h"  // for Point

#include "AbstractInputHandler.h"  // for AbstractInputHandler

class InputContext;
struct InputEvent;

class TouchInputHandler: public AbstractInputHandler {
private:
    GdkEventSequence* primarySequence{};
    GdkEventSequence* secondarySequence{};

    double startZoomDistance = 0.0;
    utl::Point<double> lastZoomScrollCenter{};

    utl::Point<double> priLastAbs{-1.0, -1.0};
    utl::Point<double> secLastAbs{-1.0, -1.0};

    utl::Point<double> priLastRel{-1.0, -1.0};
    utl::Point<double> secLastRel{-1.0, -1.0};

    bool canBlockZoom{false};

    bool detectingUndo = false;
    utl::Point<double> priStartAbs{-1.0, -1.0};
    utl::Point<double> secStartAbs{-1.0, -1.0};

private:
    void sequenceStart(InputEvent const& event);
    void scrollMotion(InputEvent const& event);
    void zoomStart();
    void zoomMotion(InputEvent const& event);
    void zoomEnd();
    void undoGestureStart();
    void undoGestureMotion(InputEvent const& event);
    void undoGestureCancel();
    void undoGestureEnd();

public:
    explicit TouchInputHandler(InputContext* inputContext);
    ~TouchInputHandler() override = default;

    bool handleImpl(InputEvent const& event) override;
    void onUnblock() override;
};
