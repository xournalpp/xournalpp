#include "StrokeStabilizer.h"

#include <algorithm>  // for min
#include <cfloat>
#include <iterator>  // for begin, end
#include <list>      // for list, operator!=
#include <numeric>   // for accumulate
#include <vector>    // for vector

#include "control/settings/Settings.h"           // for Settings
#include "control/tools/StrokeStabilizerEnum.h"  // for Preprocessor, Averag...
#include "model/SplineSegment.h"                 // for SplineSegment
#include "model/Stroke.h"                        // for Stroke

/**
 * StrokeStabilizer::get
 */
auto StrokeStabilizer::get(Settings* settings) -> std::unique_ptr<StrokeStabilizer::Base> {

    AveragingMethod averagingMethod = settings->getStabilizerAveragingMethod();
    Preprocessor preprocessor = settings->getStabilizerPreprocessor();

    if (averagingMethod == AveragingMethod::ARITHMETIC) {
        if (preprocessor == Preprocessor::DEADZONE) {
            return std::make_unique<StrokeStabilizer::ArithmeticDeadzone>(
                    settings->getStabilizerFinalizeStroke(), settings->getStabilizerBuffersize(),
                    settings->getStabilizerDeadzoneRadius(), settings->getStabilizerCuspDetection());
        }

        if (preprocessor == Preprocessor::INERTIA) {
            return std::make_unique<StrokeStabilizer::ArithmeticInertia>(
                    settings->getStabilizerFinalizeStroke(), settings->getStabilizerBuffersize(),
                    settings->getStabilizerDrag(), settings->getStabilizerMass());
        }

        return std::make_unique<StrokeStabilizer::Arithmetic>(settings->getStabilizerFinalizeStroke(),
                                                              settings->getStabilizerBuffersize());
    }

    if (averagingMethod == AveragingMethod::VELOCITY_GAUSSIAN) {
        if (preprocessor == Preprocessor::DEADZONE) {
            return std::make_unique<StrokeStabilizer::VelocityGaussianDeadzone>(
                    settings->getStabilizerFinalizeStroke(), settings->getStabilizerSigma(),
                    settings->getStabilizerDeadzoneRadius(), settings->getStabilizerCuspDetection());
        }

        if (preprocessor == Preprocessor::INERTIA) {
            return std::make_unique<StrokeStabilizer::VelocityGaussianInertia>(
                    settings->getStabilizerFinalizeStroke(), settings->getStabilizerSigma(),
                    settings->getStabilizerDrag(), settings->getStabilizerMass());
        }

        return std::make_unique<StrokeStabilizer::VelocityGaussian>(settings->getStabilizerFinalizeStroke(),
                                                                    settings->getStabilizerSigma());
    }

    if (preprocessor == Preprocessor::DEADZONE) {
        return std::make_unique<StrokeStabilizer::Deadzone>(settings->getStabilizerFinalizeStroke(),
                                                            settings->getStabilizerDeadzoneRadius(),
                                                            settings->getStabilizerCuspDetection());
    }

    if (preprocessor == Preprocessor::INERTIA) {
        return std::make_unique<StrokeStabilizer::Inertia>(
                settings->getStabilizerFinalizeStroke(), settings->getStabilizerDrag(), settings->getStabilizerMass());
    }

    /**
     * Defaults to no stabilization
     */
    return std::make_unique<StrokeStabilizer::Base>();
}

/**
 * StrokeStabilizer::Active
 */
void StrokeStabilizer::Active::finalizeStroke() {
    if (finalize) {
        rebalanceStrokePressures();
        Event ev = getLastEvent();
        quadraticSplineTo(ev);
    }
}

void StrokeStabilizer::Active::quadraticSplineTo(const Event& ev) {
    /**
     * Using the last two points of the stroke, draw a spline quadratic segment to the coordinates of ev.
     */
    Stroke* stroke = strokeHandler->getStroke();
    int pointCount = stroke->getPointCount();
    if (pointCount <= 0) {
        return;
    }

    if (pointCount == 1) {
        /**
         * Draw a line segment
         */
        drawEvent(ev);
        return;
    }

    /**
     * Draw a quadratic spline segment, with first tangent vector parallel to AB
     */
    Point B = stroke->getPoint(pointCount - 1);
    Point A = stroke->getPoint(pointCount - 2);
    Point C(ev.x / zoom, ev.y / zoom);

    MathVect vAB = {B.x - A.x, B.y - A.y};
    MathVect vBC = {C.x - B.x, C.y - B.y};
    const double squaredNormBC = vBC.dx * vBC.dx + vBC.dy * vBC.dy;
    const double normBC = std::sqrt(squaredNormBC);
    const double normAB = vAB.norm();

    if (normBC < DBL_EPSILON) {
        return;
    }
    if (normAB < DBL_EPSILON) {
        g_warning("Last two points of stroke coincide. ");
        drawEvent(ev);
        return;
    }

    /**
     * The first argument of std::min would give a symmetric quadratic spline segment.
     * The std::min and its second argument ensure the spline segment stays reasonably close to its nodes
     */
    double distance = std::min(std::abs(squaredNormBC * normAB / (2 * MathVect::scalarProduct(vAB, vBC))), normBC);

    // Quadratic control point
    Point Q = B.lineTo(A, -distance);

    /**
     * The quadratic control point is converted into two cubic control points
     */
    Point fp = B.relativeLineTo(Q, 2.0 / 3.0);
    Point sp = C.relativeLineTo(Q, 2.0 / 3.0);

    /**
     * Set the pressure values. Only the usual pen strokes are pressure sensitive
     */
    bool usePressure = ev.pressure != Point::NO_PRESSURE && stroke->getToolType().isPressureSensitive();
    if (usePressure) {
        C.z = ev.pressure * stroke->getWidth();
        double coeff = normBC / 2 + distance;  // Very rough estimation of the spline's length
        B.z = (coeff * A.z + normAB * C.z) / (normAB + coeff);
        stroke->setLastPressure(B.z);
    }

    SplineSegment spline(B, fp, sp, C);
    /**
     * TODO Add support for spline segments in Stroke and replace this point sequence by a single spline segment
     */
    std::list<Point> pointsToPaint = spline.toPointSequence(usePressure);

    pointsToPaint.pop_front();  // Point B has already been painted

    for (auto&& point: pointsToPaint) { strokeHandler->drawSegmentTo(point); }
    C.z = ev.pressure;  // Normal state after having added a segment. Useful?
    strokeHandler->drawSegmentTo(C);
}


/**
 * StrokeStabilizer::Deadzone
 */
void StrokeStabilizer::Deadzone::recordFirstEvent(const PositionInputData& pos) {
    lastEvent = Event(pos);
    lastPaintedEvent = lastEvent;
}

void StrokeStabilizer::Deadzone::processEvent(const PositionInputData& pos) {

    /**
     * Record the event for the stroke finisher
     */
    lastEvent = Event(pos);

    MathVect movement = {lastEvent.x - lastPaintedEvent.x, lastEvent.y - lastPaintedEvent.y};
    double ratio = deadzoneRadius / movement.norm();

    if (ratio >= 1) {
        /**
         * The event occurred inside the deadzone. Invalidate it.
         * Flush the contingent buffer for sharper change of directions
         */
        resetBuffer(lastPaintedEvent, pos.timestamp);
        return;
    }

    if (cuspDetection && (MathVect::scalarProduct(movement, lastLiveDirection) < 0)) {
        /**
         * lastLiveDirection != 0 and the angle between movement and lastLiveDirection is greater than 90°
         * We have a clear change of direction. This is a cusp. Draw the entire cusp
         */

        /**
         * Paint a segment from the stroke's last point to the tip of the cusp.
         */
        //         drawEvent(lastLiveEvent);
        quadraticSplineTo(lastLiveEvent);

        /**
         * Paint the way back from the tip of the cusp
         * To do so, we create an artificial point between lastEvent and lastLiveEvent
         */
        MathVect diff = {lastEvent.x - lastLiveEvent.x, lastEvent.y - lastLiveEvent.y};
        double diffNorm = diff.norm();
        double coeff = deadzoneRadius / diffNorm;

        lastLiveDirection.dx = coeff * diff.dx;
        lastLiveDirection.dy = coeff * diff.dy;

        lastPaintedEvent.x = lastEvent.x - lastLiveDirection.dx;
        lastPaintedEvent.y = lastEvent.y - lastLiveDirection.dy;
        lastPaintedEvent.pressure = coeff * lastLiveEvent.pressure + (1 - coeff) * lastEvent.pressure;

        drawEvent(lastPaintedEvent);

        lastLiveEvent = lastEvent;

        resetBuffer(lastPaintedEvent, pos.timestamp);
        return;
    }

    /**
     * Normal behaviour. Adjust the event's coordinates according to the deadzoneRadius
     */
    lastLiveEvent = lastEvent;
    lastLiveDirection = movement;

    Event ev(lastEvent.x - ratio * movement.dx, lastEvent.y - ratio * movement.dy, lastEvent.pressure);

    averageAndPaint(ev, pos.timestamp);
}

void StrokeStabilizer::Deadzone::rebalanceStrokePressures() {
    Stroke* stroke = strokeHandler->getStroke();
    int pointCount = stroke->getPointCount();
    if (pointCount >= 3) {
        /**
         * Smoothen a little bit the pressure variations
         */
        stroke->setSecondToLastPressure((stroke->getPoint(pointCount - 2).z + stroke->getPoint(pointCount - 3).z) / 2);
    }
}


/**
 * StrokeStabilizer::Inertia
 */
void StrokeStabilizer::Inertia::recordFirstEvent(const PositionInputData& pos) {
    lastEvent = Event(pos);
    lastPaintedEvent = lastEvent;
}

void StrokeStabilizer::Inertia::processEvent(const PositionInputData& pos) {

    /**
     * Record the event for the stroke finisher
     */
    lastEvent = Event(pos);

    /**
     * Compute the acceleration due to the spring action
     */
    MathVect springAcceleration = {(lastEvent.x - lastPaintedEvent.x) / mass,
                                   (lastEvent.y - lastPaintedEvent.y) / mass};

    speed.dx = speed.dx * oneMinusDrag + springAcceleration.dx;
    speed.dy = speed.dy * oneMinusDrag + springAcceleration.dy;

    Event ev(lastPaintedEvent.x + speed.dx, lastPaintedEvent.y + speed.dy, lastEvent.pressure);

    averageAndPaint(ev, pos.timestamp);
}

void StrokeStabilizer::Inertia::rebalanceStrokePressures() {
    Stroke* stroke = strokeHandler->getStroke();
    int pointCount = stroke->getPointCount();
    if (pointCount >= 3) {
        /**
         * Smoothen a little bit the pressure variations
         */
        stroke->setSecondToLastPressure((stroke->getPoint(pointCount - 2).z + stroke->getPoint(pointCount - 3).z) / 2);
    }
}


/**
 * StrokeStabilizer::Arithmetic
 */
void StrokeStabilizer::Arithmetic::recordFirstEvent(const PositionInputData& pos) {
    eventBuffer.assign(Event(pos));  // Fill the buffer with copies of Event(pos)
}

void StrokeStabilizer::Arithmetic::averageAndPaint(const Event& ev, guint32 timestamp) {
    /**
     * Push the event and overwrite the oldest event in the buffer
     */
    eventBuffer.push_front(ev);

    /**
     * Average the coordinates using an arithmetic mean
     */
    Event sum = std::accumulate(begin(eventBuffer), end(eventBuffer), Event(0, 0, 0), [](auto&& lhs, auto&& rhs) {
        return Event(lhs.x + rhs.x, lhs.y + rhs.y, lhs.pressure + rhs.pressure);
    });

    /**
     * Rescale the averaged coordinates and draw
     */
    double d = static_cast<double>(eventBuffer.size());
    sum.pressure /= d;
    sum.x /= d;
    sum.y /= d;
    setLastPaintedEvent(sum);
    drawEvent(sum);
}

auto StrokeStabilizer::Arithmetic::getLastEvent() -> Event { return eventBuffer.front(); }

void StrokeStabilizer::Arithmetic::resetBuffer(Event& ev, guint32 timestamp) {
    if (eventBuffer.back() != ev) {
        eventBuffer.assign(ev);  // Replace the entire content of the buffer with copies of ev
    }
}

/**
 * StrokeStabilizer::VelocityGaussian
 */
void StrokeStabilizer::VelocityGaussian::recordFirstEvent(const PositionInputData& pos) {
    eventBuffer.emplace_front(pos);
    lastEventTimestamp = pos.timestamp;
}

void StrokeStabilizer::VelocityGaussian::averageAndPaint(const Event& ev, guint32 timestamp) {

    /**
     * Compute the velocity (if possible) and push the event to eventBuffer
     */
    if (eventBuffer.empty()) {
        eventBuffer.emplace_front(ev);
    } else {
        /**
         * Issue: timestamps are in ms. They are not precise enough. Different events can have the same timestamp.
         */
        VelocityEvent& last = eventBuffer.front();
        guint32 timelaps = timestamp - lastEventTimestamp;
        if (timelaps == 0) {
            timelaps = 1;
        }
        eventBuffer.emplace_front(ev, std::hypot(ev.x - last.x, ev.y - last.y) / static_cast<double>(timelaps));
    }
    lastEventTimestamp = timestamp;

    /**
     * Average the coordinates using the gimp-like weights
     */
    Event weightedSum = {0, 0, 0};
    double weight;
    double sumOfWeights = 0;
    double sumOfVelocities = 0;

    auto it = eventBuffer.cbegin();
    for (; it != eventBuffer.cend(); ++it) {
        /**
         * The first weight is always 1
         */
        weight = exp(-sumOfVelocities * sumOfVelocities / twoSigmaSquared);
        if (weight < 0.01) {
            break;
        }
        sumOfVelocities += (*it).velocity;
        weightedSum.x += weight * (*it).x;
        weightedSum.y += weight * (*it).y;
        weightedSum.pressure += weight * (*it).pressure;
        sumOfWeights += weight;
    }
    eventBuffer.erase(it, eventBuffer.cend());

    weightedSum.x /= sumOfWeights;
    weightedSum.y /= sumOfWeights;
    weightedSum.pressure /= sumOfWeights;

    setLastPaintedEvent(weightedSum);
    drawEvent(weightedSum);
}

auto StrokeStabilizer::VelocityGaussian::getLastEvent() -> Event {
    if (eventBuffer.empty()) {
        g_warning("StrokeStabilizer::VelocityGaussian buffer empty. This should never be!");
        return Event(0, 0, 0);
    }
    return eventBuffer.front();
}

void StrokeStabilizer::VelocityGaussian::resetBuffer(Event& ev, guint32 timestamp) {
    if (eventBuffer.size() != 1 || lastEventTimestamp != timestamp || eventBuffer.front() != ev) {
        eventBuffer.clear();
        lastEventTimestamp = timestamp;
        eventBuffer.emplace_front(ev);
    }
}
