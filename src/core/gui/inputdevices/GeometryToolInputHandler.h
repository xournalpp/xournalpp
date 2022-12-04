/*
 * Xournal++
 *
 * Input handler for the geometry tool.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <map>     // for map
#include <vector>  // for vector

#include <gdk/gdk.h>  // for GdkEventSequence

#include "util/DispatchPool.h"  // for xoj::util::Listener
#include "util/Point.h"         // for Point

#include "InputContext.h"  // for InputContext::DeviceType, InputContext

class Stroke;
class GeometryToolController;
struct InputEvent;

/**
 * @brief Input handler for the geometry tool
 *
 * The geometry tool is designed to be used with touch/keyboard with one hand and mouse/stylus with the other hand
 * Keyboard and Touch are used to move the geometry tool (and the page), to rotate it and change its size, whereas
 * Stylus/Mouse are used to draw/annotate.
 *
 * The touch handling part is adopted from the TouchInputHandler class
 */
class GeometryToolInputHandler: public xoj::util::Listener<GeometryToolInputHandler> {

protected:
    /**
     * @brief the XournalView attached to this handler
     */
    XournalView* xournal;

    /**
     * @brief the geometry tool controller
     *
     */
    GeometryToolController* controller;

    double height;
    double rotation = 0.;
    double translationX;
    double translationY;

    /**
     * @brief saves which devices are blocked, so they don't need to be handled
     *        This is currently used for the automatic hand recognition, that can be turned on in the settings
     */
    std::map<InputContext::DeviceType, bool> isBlocked{};

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
     * @brief The current positions of the first (primary) finger and second (secondary) finger in page coordinates
     */
    utl::Point<double> priLastPageRel{-1.0, -1.0};
    utl::Point<double> secLastPageRel{-1.0, -1.0};

    /**
     * @brief The previous angle between the line through the positions of the two fingers and the x-axis
     */
    double lastAngle{0.0};

    /**
     * @brief The previous distance between the two fingers touching the screen
     */
    double lastDist{1.0};

    /**
     * @brief whether resizing the geometry tool can be blocked, because the distance between the fingers hasn't changed
     * enough
     */
    bool canBlockZoom{false};

    /**
     * @brief whether a scrolling for the hand tool takes place (after touching the geometry tool with the hand tool)
     */
    bool handScrolling{false};

    /**
     * @brief the line segments (strokes with two points) of the current layer, that are used for rotation snapping the
     * geometry tool
     */
    std::vector<Stroke*> lines;

protected:
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
     * @brief start rotating and zooming (after the second finger is put onto the screen)
     * only zooms, when zoom gestures are enabled
     */
    void rotateAndZoomStart();

    /**
     * @brief rotation and zooming method, only zooms when zoom gestures are enabled
     * @param event the current input event
     */
    void rotateAndZoomMotion(InputEvent const& event);

    /**
     * @brief handles input from the touchscreen for the geometry tool
     */
    bool handleTouchscreen(InputEvent const& event);

    /**
     * @brief handles input from the keyboard for the geometry tool
     */
    bool handleKeyboard(InputEvent const& event);

    /**
     * @brief handles input from mouse and stylus for the geometry tool
     */
    virtual bool handlePointer(InputEvent const& event) = 0;

    /**
     * @brief the document coordinates derived from an input event
     * @param event an input event
     */
    utl::Point<double> getCoords(InputEvent const& event);

    virtual double getMinHeight() const = 0;
    virtual double getMaxHeight() const = 0;

public:
    explicit GeometryToolInputHandler(XournalView* xournalView, GeometryToolController* controller, double h, double tx,
                                      double ty);
    virtual ~GeometryToolInputHandler();

    bool handle(InputEvent const& event);
    void blockDevice(InputContext::DeviceType deviceType);
    void unblockDevice(InputContext::DeviceType deviceType);

    /**
     * Listener interface
     */
    static constexpr struct UpdateValuesRequest {
    } UPDATE_VALUES = {};
    void on(UpdateValuesRequest, double h, double rot, double tx, double ty);
};
