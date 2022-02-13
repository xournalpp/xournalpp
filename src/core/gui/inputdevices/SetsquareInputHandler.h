/*
 * Xournal++
 *
 * Input handler for the setsquare.
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

#include "util/Point.h"

#include "AbstractInputHandler.h"

class InputContext;

/**
 * @brief Input handler for the setsquare
 * The setsquare is designed to be used with touch/keyboard with one hand and mouse/stylus with the other hand
 * Keyboard and Touch are used to move the setsquare (and the page), to rotate it and change its size, whereas
 * Stylus/Mouse are used to draw/annotate and in particular to draw lines at the longest side of the setsquare.
 *
 * The touch handling part is adopted from the TouchInputHandler class
 */
class SetsquareInputHandler: public AbstractInputHandler {
private:
    /**
     * @brief the touch event sequence corresponding to the first finger touching the screen
     */
    GdkEventSequence* primarySequence{};
    /**
     * @brief the touch event sequence corresponding to the second finger touching the screen
     */
    GdkEventSequence* secondarySequence{};

    /**
     * @brief the distance between the two fingers touching the screen at the start of a zoom sequence
     */
    double startZoomDistance{0.0};

    /**
     * @brief the previous mid point between the two fingers in a zoom sequence
     */
    utl::Point<double> lastZoomScrollCenter{};

    /**
     * @brief The current absolute and relative positions of the first (primary) finger and second (secondary) finger
     */
    utl::Point<double> priLastAbs{-1.0, -1.0};
    utl::Point<double> secLastAbs{-1.0, -1.0};
    utl::Point<double> priLastRel{-1.0, -1.0};
    utl::Point<double> secLastRel{-1.0, -1.0};

    /**
     * @brief The previous angle between the line through the positions of the two fingers and the x-axis
     */
    double lastAngle{0.0};

    /**
     * @brief The previous distance between the two fingers touching the screen
     */
    double lastDist{1.0};

    /**
     * @brief whether resizing the setsquare can be blocked, because the distance between the fingers hasn't changed
     * enough
     */
    bool canBlockZoom{false};

    /**
     * @brief whether a scrolling for the hand tool takes place (after touching the setsquare with the hand tool)
     */
    bool handScrolling{false};

    /**
     * @brief the line segments (strokes with two points) of the current layer, that are used for rotation snapping the
     * setsquare
     */
    std::vector<Stroke*> lines;

private:
    /**
     * @brief initializes an input sequence (right after the first or second finger is put onto the screen)
     * @param event the input event initializing the sequence
     */
    void sequenceStart(InputEvent const& event);

    /**
     * @brief scrolling method
     * @param event the current input event
     */
    void scrollMotion(InputEvent const& event);

    /**
     * @brief start zooming (after the second finger is put onto the screen)
     */
    void zoomStart();

    /**
     * @brief zoooming method
     * @param event the current input event
     */
    void zoomMotion(InputEvent const& event);

    /**
     * @brief handles input from the touchscreen for the setsquare
     */
    bool handleTouchscreen(InputEvent const& event);

    /**
     * @brief handles input from the keyboard for the setsquare
     */
    bool handleKeyboard(InputEvent const& event);

    /**
     * @brief handles input from mouse and stylus for the setsquare
     */
    bool handlePointer(InputEvent const& event);

    /**
     * @brief the document coordinates derived from an input event
     * @param event an input event
     */
    utl::Point<double> getCoords(InputEvent const& event);

    /**
     * @brief the document coordinates derived from relative view coordinates
     * @param relative x coordinate
     * @param relative y coordinate
     */
    utl::Point<double> getCoords(double x, double y);

public:
    explicit SetsquareInputHandler(InputContext* inputContext);
    ~SetsquareInputHandler() override = default;

    bool handleImpl(InputEvent const& event) override;
    void onUnblock() override;
};
