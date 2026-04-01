/*
 * Xournal++
 *
 * Handles input of strokes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cstddef>  // for size_t
#include <deque>    // for deque
#include <memory>   // for allocator, unique_ptr
#include <string>   // for operator+, char_traits

#include <glib.h>  // for guint32

#include "control/tools/StrokeHandler.h"         // for StrokeHandler
#include "gui/inputdevices/PositionInputData.h"  // for PositionInputData
#include "model/MathVect.h"                      // for MathVect2
#include "model/Point.h"                         // for Point
#include "util/CircularBuffer.h"                 // for CircularBuffer

class Settings;

namespace StrokeStabilizer {

/**
 * @brief A structure containing the event's information relevant for the stabilizers
 */
struct Event {
    Event() = default;
    Event(double x, double y, double pressure): x(x), y(y), pressure(pressure) {}
    Event(const PositionInputData& pos): x(pos.x), y(pos.y), pressure(pos.pressure) {}
    bool operator!=(const Event& ev) { return (x != ev.x) || (y != ev.y) || (pressure != ev.pressure); }
    double x{};
    double y{};
    double pressure{};
};

/**
 * @brief Base stabilizer class. Also used as default (no stabilization).
 */
class Base {
public:
    Base() = default;
    virtual ~Base() = default;

    /**
     * @brief Initialize the stabilizer
     * @param sH Pointer to the StrokeHandler instance handling the stroke
     * @param zoomValue The current zoom
     * @param pos The position of the button down event starting the stroke
     */
    void initialize(StrokeHandler* sH, double zoomValue, const PositionInputData& pos) {
        strokeHandler = sH;
        zoom = zoomValue;
        recordFirstEvent(pos);
    }

    /**
     * @brief Compute stabilized coordinates for the event and paints the obtained point
     * @param pos The MotionNotify event information
     */
    virtual void processEvent(const PositionInputData& pos) {
        strokeHandler->paintTo(Point(pos.x / zoom, pos.y / zoom, pos.pressure));
    }

    /**
     * @brief Fill the possible gap between the last painted point and the last event received by the stabilizer.
     *
     * Does nothing in the base class
     */
    virtual void finalizeStroke() {}

    [[maybe_unused]] virtual auto getInfo() -> std::string { return "No stabilizer"; }

protected:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     *
     * Does nothing in the base class
     */
    virtual inline void recordFirstEvent(const PositionInputData& pos){};

    /**
     * @brief Pointer to the StrokeHandler instance handling the stroke
     */
    StrokeHandler* strokeHandler;

    /**
     * @brief The zoom value to be applied on all painted points
     */
    double zoom;
};

/**
 * @brief Abstract base class for active stabilizers
 */
class Active: public Base {
public:
    Active(bool finalize): finalize(finalize) {}
    ~Active() override = default;

    /**
     * @brief Fill the possible gap between the last painted point and the last event received by the stabilizer.
     * It does so by creating a quadraticSplineTo the last event's coordinates
     */
    void finalizeStroke() override;

    /**
     * @brief Compute stabilized coordinates for the event and paints the obtained point
     * @param pos The MotionNotify event information
     */
    void processEvent(const PositionInputData& pos) override {
        Event ev(pos);
        averageAndPaint(ev, pos.timestamp);
    }

protected:
    /**
     * @brief Add a segment to the stroke ending at the parameters coordinates
     * @param ev The event whose coordinates determine the stroke's new endpoint
     */
    inline void drawEvent(const Event& ev) { strokeHandler->paintTo(Point(ev.x / zoom, ev.y / zoom, ev.pressure)); }

    /**
     * @brief Record the event corresponding to last painted point
     * @param ev The event corresponding to last painted point
     *
     * Does nothing in the base class.
     */
    virtual inline void setLastPaintedEvent(const Event& ev) {}

    /**
     * @brief For stabilizers with a buffer, clear the buffer and reinitialize it with the provided data
     * @param ev New event to push to the buffer
     * @param timestamp The timestamp of that event
     *
     * Does nothing in the base class
     */
    virtual inline void resetBuffer(Event& ev, guint32 timestamp) {}

    /**
     * @brief On stabilizers with a buffer, use it to average the input's position and paint the resulting point
     * @param ev An event to add to the mix
     * @param timestamp The event's timestamp
     *
     * In the base class, simply paint the point corresponding to the given event
     */
    virtual inline void averageAndPaint(const Event& ev, guint32 timestamp) {
        setLastPaintedEvent(ev);
        drawEvent(ev);
    }

    /**
     * @brief Add a quadratic spline segment from the end of the stroke to the point corresponding to the parameters, so
     * that the stroke is smooth at its former endpoint
     * @param ev Event whose coordinates determine the stroke's new endpoint
     */
    void quadraticSplineTo(const Event& ev);

private:
    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event
     */
    virtual inline Event getLastEvent() = 0;

    /**
     * @brief Tweak the stroke's pressure values upon finalizing the stroke
     *
     * Does nothing in the base class
     */
    virtual inline void rebalanceStrokePressures() {}

    /**
     * @brief Wether or not to finalize the stroke
     */
    bool finalize = true;
};


/*****************************
 *       PREPROCESSORS       *
 *****************************/

/**
 * @brief Class for deadzone stabilizer/preprocessor
 */
class Deadzone: virtual public Active {
public:
    Deadzone(bool finalize, double dzRadius, bool cuspDetection):
            Active(finalize), deadzoneRadius(dzRadius), cuspDetection(cuspDetection) {}
    ~Deadzone() override = default;

    /**
     * @brief Compute stabilized coordinates for the event and paints the obtained point
     * @param pos The MotionNotify event information
     */
    void processEvent(const PositionInputData& pos) override;

    [[maybe_unused]] auto getInfo() -> std::string override {
        return "Deadzone stabilizer with deadzoneRadius = " + std::to_string(deadzoneRadius) +
               ", cusp detection = " + (cuspDetection ? "on" : "off");
    }

protected:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     */
    void recordFirstEvent(const PositionInputData& pos) override;

    /**
     * @brief Record the event corresponding to last painted point
     * @param ev The event corresponding to last painted point
     */
    inline void setLastPaintedEvent(const Event& ev) override { lastPaintedEvent = ev; }

    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event
     */
    inline Event getLastEvent() override { return lastEvent; }

private:
    /**
     * @brief The deadzone radius
     */
    const double deadzoneRadius;

    /**
     * @brief Flag to turn on/off the cusp detection
     */
    const bool cuspDetection;

    /**
     * @brief The direction of the last event outside the deadzone
     */
    MathVect2 lastLiveDirection = {0, 0};

    /**
     * @brief The last event outside the deadzone
     */
    Event lastLiveEvent;

    /**
     * @brief The last (stabilized) painted point, center of the deadzone
     */
    Event lastPaintedEvent;

    /**
     * @brief The last event (anywhere), used by finishStroke
     */
    Event lastEvent;

    /**
     * @brief Tweak the stroke's pressure values upon finalizing the stroke
     */
    void rebalanceStrokePressures() override;
};

/**
 * @brief Class for inertial preprocessor
 *
 * Principle: Assign a mass to the pencil tip and simulate a spring between the tip and the pointer device.
 * Friction between the tip and the paper is added by using a drag coefficient.
 *
 * A too small drag can lead to unwanted oscillations
 */
class Inertia: virtual public Active {
public:
    Inertia(bool finalize, double drag, double mass):
            Active(finalize), mass(mass), oneMinusDrag(1 - drag), speed{0.0, 0.0} {}
    ~Inertia() override = default;

    /**
     * @brief Compute stabilized coordinates for the event and paints the obtained point
     * @param pos The MotionNotify event information
     */
    void processEvent(const PositionInputData& pos) override;

    [[maybe_unused]] auto getInfo() -> std::string override {
        return "Inertia stabilizer with mass = " + std::to_string(mass) +
               ", drag = " + std::to_string(1 - oneMinusDrag);
    }

protected:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     */
    void recordFirstEvent(const PositionInputData& pos) override;

    /**
     * @brief Record the event corresponding to last painted point
     * @param ev The event corresponding to last painted point
     */
    inline void setLastPaintedEvent(const Event& ev) override { lastPaintedEvent = ev; }

    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event
     */
    inline Event getLastEvent() override { return lastEvent; }

private:
    /**
     * @brief mass of the pen
     */
    const double mass;

    /**
     * @brief Friction coefficient
     */
    const double oneMinusDrag;

    /**
     * @brief Speed during the last step
     */
    MathVect2 speed;

    /**
     * @brief The last (stabilized) painted point, center of the deadzone
     */
    Event lastPaintedEvent;

    /**
     * @brief The last event (anywhere), used by finishStroke
     */
    Event lastEvent;

    /**
     * @brief Tweak the stroke's pressure values upon finalizing the stroke
     */
    void rebalanceStrokePressures() override;
};


/*****************************
 *         AVERAGERS         *
 *****************************/

/**
 * @brief Class for the Velocity-based Gaussian averaging stabilizer (a.k.a. Gimp-like)
 */
class VelocityGaussian: virtual public Active {
public:
    VelocityGaussian(bool finalize, double sigma): Active(finalize), twoSigmaSquared(2 * sigma * sigma) {}
    ~VelocityGaussian() override = default;

    [[maybe_unused]] auto getInfo() -> std::string override {
        return "Velocity-based gaussian weight stabilizer with "
               "2σ² = " +
               std::to_string(twoSigmaSquared);
    }

protected:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     */
    void recordFirstEvent(const PositionInputData& pos) override;

    /**
     * @brief Use the buffer to average the input's position and paint the resulting point
     * @param ev An event to add to the mix
     * @param timestamp The event's timestamp
     */
    void averageAndPaint(const Event& ev, guint32 timestamp) override;

    /**
     * @brief For stabilizers with a buffer, clear the buffer and reinitialize it with the provided data
     * @param ev New event to push to the buffer
     * @param timestamp The timestamp of that event
     */
    void resetBuffer(Event& ev, guint32 timestamp) override;

    /**
     * @brief Structure containing the event's information relevant to the VelocityGaussian stabilizer
     */
    struct VelocityEvent: public Event {
        VelocityEvent() = default;
        VelocityEvent(double x, double y, double pressure, double velocity):
                Event(x, y, pressure), velocity(velocity) {}
        VelocityEvent(const PositionInputData& pos, double velocity = 0): Event(pos), velocity(velocity) {}
        VelocityEvent(const Event& ev, double velocity = 0): Event(ev), velocity(velocity) {}
        double velocity{};
    };

    /**
     * @brief A queue containing the relevant information on the last events
     * The beginning of the queue contains the most recent event
     * The end of the queue contains the most ancient event stored
     */
    std::deque<VelocityEvent> eventBuffer;

private:
    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event
     */
    Event getLastEvent() override;

    /**
     * @brief The Gaussian parameter
     */
    const double twoSigmaSquared;

    /**
     * @brief Timestamp of the last event received. Used to compute the velocity of the next event
     */
    guint32 lastEventTimestamp;
};

class Arithmetic: virtual public Active {
public:
    Arithmetic(bool finalize, size_t buffersize): Active(finalize), bufferLength(buffersize), eventBuffer(buffersize) {}
    ~Arithmetic() override = default;

    [[maybe_unused]] auto getInfo() -> std::string override {
        return "Arithmetic stabilizer with bufferLength " + std::to_string(bufferLength);
    }

protected:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     */
    void recordFirstEvent(const PositionInputData& pos) override;

    /**
     * @brief Use the buffer to average the input's position and paint the resulting point
     * @param ev An event to add to the mix
     * @param timestamp The event's timestamp
     */
    void averageAndPaint(const Event& ev, guint32 timestamp) override;

    /**
     * @brief For stabilizers with a buffer, clear the buffer and reinitialize it with the provided data
     * @param ev New event to push to the buffer
     * @param timestamp The timestamp of that event
     */
    void resetBuffer(Event& ev, guint32 timestamp) override;

    /**
     * @brief The maximal length of the buffer
     */
    const size_t bufferLength;

    /**
     * @brief A circular buffer containing the relevant information on the last events
     * The front of the buffer contains the most recent event
     * The back of the buffer contains the most ancient event stored
     */
    CircularBuffer<Event> eventBuffer;

private:
    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event received
     */
    Event getLastEvent() override;
};


/*****************************
 *          HYBRIDS          *
 *****************************/

class ArithmeticDeadzone: public Arithmetic, public Deadzone {
public:
    ArithmeticDeadzone(bool finalize, size_t buffersize, double dzRadius, bool cuspDetection):
            Active(finalize), Arithmetic(finalize, buffersize), Deadzone(finalize, dzRadius, cuspDetection) {}
    ~ArithmeticDeadzone() override = default;

    [[maybe_unused]] auto getInfo() -> std::string override {
        return "Hybrid stabilizer:\n   * " + Arithmetic::getInfo() + "\n   * " + Deadzone::getInfo();
    }

private:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     */
    inline void recordFirstEvent(const PositionInputData& pos) override {
        Arithmetic::recordFirstEvent(pos);
        Deadzone::recordFirstEvent(pos);
    }
    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event received
     */
    inline Event getLastEvent() override { return Deadzone::getLastEvent(); }
};

class ArithmeticInertia: public Arithmetic, public Inertia {
public:
    ArithmeticInertia(bool finalize, size_t buffersize, double drag, double mass):
            Active(finalize), Arithmetic(finalize, buffersize), Inertia(finalize, drag, mass) {}
    ~ArithmeticInertia() override = default;

    [[maybe_unused]] auto getInfo() -> std::string override {
        return "Hybrid stabilizer:\n   * " + Arithmetic::getInfo() + "\n   * " + Inertia::getInfo();
    }

private:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     */
    inline void recordFirstEvent(const PositionInputData& pos) override {
        Arithmetic::recordFirstEvent(pos);
        Inertia::recordFirstEvent(pos);
    }
    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event received
     */
    inline Event getLastEvent() override { return Inertia::getLastEvent(); }
};

class VelocityGaussianDeadzone: public VelocityGaussian, public Deadzone {
public:
    VelocityGaussianDeadzone(bool finalize, double sigma, double dzRadius, bool cuspDetection):
            Active(finalize), VelocityGaussian(finalize, sigma), Deadzone(finalize, dzRadius, cuspDetection) {}
    ~VelocityGaussianDeadzone() override = default;

    [[maybe_unused]] auto getInfo() -> std::string override {
        return "Hybrid stabilizer:\n   * " + VelocityGaussian::getInfo() + "\n   * " + Deadzone::getInfo();
    }

private:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     */
    inline void recordFirstEvent(const PositionInputData& pos) override {
        VelocityGaussian::recordFirstEvent(pos);
        Deadzone::recordFirstEvent(pos);
    }
    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event received
     */
    inline Event getLastEvent() override { return Deadzone::getLastEvent(); }
};

class VelocityGaussianInertia: public VelocityGaussian, public Inertia {
public:
    VelocityGaussianInertia(bool finalize, double sigma, double drag, double mass):
            Active(finalize), VelocityGaussian(finalize, sigma), Inertia(finalize, drag, mass) {}
    ~VelocityGaussianInertia() override = default;

    [[maybe_unused]] auto getInfo() -> std::string override {
        return "Hybrid stabilizer:\n   * " + VelocityGaussian::getInfo() + "\n   * " + Inertia::getInfo();
    }

private:
    /**
     * @brief Upon initialization, record the first event for the stabilizer's benefits.
     * @param pos The first event
     */
    inline void recordFirstEvent(const PositionInputData& pos) override {
        VelocityGaussian::recordFirstEvent(pos);
        Inertia::recordFirstEvent(pos);
    }
    /**
     * @brief Get the last event received by the stabilizer
     * @return The last event received
     */
    inline Event getLastEvent() override { return Inertia::getLastEvent(); }
};

/**
 * @brief Stabilizer factory: create a stabilizer of the right kind
 * @param settings The Settings instance to read to determine what kind of stabilizer to create
 * @return A unique point to the create stabilizer instance.
 */
std::unique_ptr<Base> get(Settings* settings);

}  // namespace StrokeStabilizer
