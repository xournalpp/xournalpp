/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cmath>
#include <string>
#include <vector>

#include "AbstractInputHandler.h"
#include "Point.h"
#include "XournalType.h"

class InputContext;

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

private:
    void sequenceStart(InputEvent const& event);
    void scrollMotion(InputEvent const& event);
    void zoomStart();
    void zoomMotion(InputEvent const& event);
    void zoomEnd();

public:
    explicit TouchInputHandler(InputContext* inputContext);
    ~TouchInputHandler() override = default;

    bool handleImpl(InputEvent const& event) override;
    void onUnblock() override;
};
