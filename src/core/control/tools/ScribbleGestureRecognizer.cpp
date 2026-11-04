#include "ScribbleGestureRecognizer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include "model/Point.h"
#include "util/Point.h"

namespace {
using Position = xoj::util::Point<double>;
constexpr size_t SAMPLE_COUNT = 64;
constexpr double MIN_EXTENT = 4.0;  // Logical page units, independent of zoom.

// Conservative starting limits; candidate classification alone must never trigger deletion.
constexpr double MIN_RESAMPLED_LENGTH_RATIO = 0.85;
constexpr double MIN_UNEVEN_PASS_RATIO = 0.2;

auto normalize(const Point& point, Position origin, double scale) -> Position {
    return {(point.x - origin.x) / scale, (point.y - origin.y) / scale};
}

auto resample(std::span<const Point> points, Position origin, double scale, double length,
              std::array<Position, SAMPLE_COUNT>& samples) -> bool {
    Position previous = normalize(points.front(), origin, scale);
    samples.front() = previous;
    double traversed = 0;
    size_t nextSample = 1;
    for (const Point& point: points.subspan(1)) {
        const Position current = normalize(point, origin, scale);
        const double segmentLength = current.distance(previous);
        if (segmentLength > 0) {
            while (nextSample + 1 < SAMPLE_COUNT) {
                const double distance = length * (static_cast<double>(nextSample) / (SAMPLE_COUNT - 1));
                if (distance > traversed + segmentLength) {
                    break;
                }
                const double ratio = std::clamp((distance - traversed) / segmentLength, 0.0, 1.0);
                samples[nextSample++] = previous + (current - previous) * ratio;
            }
        }
        traversed += segmentLength;
        previous = current;
    }
    samples.back() = previous;
    return nextSample == SAMPLE_COUNT - 1;
}

auto hasRepeatedPasses(const std::array<double, SAMPLE_COUNT>& projections, double extent,
                       const ScribbleSettings& settings) -> bool {
    const double sensitivity = settings.sensitivity / 100.0;
    const double reversalDistance = std::lerp(0.2, 0.1, sensitivity) * extent;
    const double minPassLength = std::lerp(0.8, 0.4, sensitivity) * extent;
    double passStart = projections.front();
    double extreme = passStart;
    double commonMin = -std::numeric_limits<double>::infinity();
    double commonMax = std::numeric_limits<double>::infinity();
    size_t passes = 0;
    int direction = 0;
    bool firstPass = true;

    const auto finishPass = [&](double end, bool boundary) {
        if (std::abs(end - passStart) < minPassLength) {
            // Pen-down/up can happen halfway through a pass. Ignore only those boundary
            // fragments; short interior passes still reject the gesture.
            return boundary;
        }
        commonMin = std::max(commonMin, std::min(passStart, end));
        commonMax = std::min(commonMax, std::max(passStart, end));
        ++passes;
        return true;
    };

    for (size_t i = 1; i < projections.size(); ++i) {
        const double value = projections[i];
        if (direction == 0) {
            if (std::abs(value - passStart) >= reversalDistance) {
                direction = value > passStart ? 1 : -1;
                extreme = value;
            }
        } else if ((direction > 0 && value > extreme) || (direction < 0 && value < extreme)) {
            extreme = value;
        } else if (static_cast<double>(direction) * (extreme - value) >= reversalDistance) {
            if (!finishPass(extreme, firstPass)) {
                return false;
            }
            firstPass = false;
            passStart = extreme;
            extreme = value;
            direction = -direction;
        }
    }
    return direction != 0 && finishPass(extreme, true) && passes >= static_cast<size_t>(settings.minimumPasses) &&
           commonMax - commonMin >= minPassLength;
}

auto analyzeUnevenScrub(std::span<const Point> points, Position origin, double scale, double length,
                        const ScribbleSettings& settings) -> std::optional<ScribbleGestureRecognizer::Geometry> {
    const double sensitivity = settings.sensitivity / 100.0;
    // Position covariance follows the drift of a scrub across a word. Use the
    // arc-length-weighted movement axis for dense, uneven back-and-forth input.
    double xx = 0;
    double xy = 0;
    double yy = 0;
    Position previous = normalize(points.front(), origin, scale);
    for (const Point& point: points.subspan(1)) {
        const Position current = normalize(point, origin, scale);
        const Position delta = current - previous;
        const double distance = current.distance(previous);
        if (distance > 0) {
            xx += delta.x * delta.x / distance;
            xy += delta.x * delta.y / distance;
            yy += delta.y * delta.y / distance;
        }
        previous = current;
    }
    if (std::hypot(xx - yy, 2 * xy) < std::lerp(0.95, 0.75, sensitivity) * (xx + yy)) {
        return std::nullopt;
    }
    const double angle = 0.5 * std::atan2(2 * xy, xx - yy);
    const Position axis{std::cos(angle), std::sin(angle)};
    const auto projection = [&](const Point& point) {
        const Position position = normalize(point, origin, scale);
        return position.x * axis.x + position.y * axis.y;
    };
    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -minimum;
    double transverseMin = minimum;
    double transverseMax = maximum;
    for (const Point& point: points) {
        const Position position = normalize(point, origin, scale);
        const double value = projection(point);
        minimum = std::min(minimum, value);
        maximum = std::max(maximum, value);
        const double transverse = position.y * axis.x - position.x * axis.y;
        transverseMin = std::min(transverseMin, transverse);
        transverseMax = std::max(transverseMax, transverse);
    }
    const double extent = maximum - minimum;
    if (extent < MIN_EXTENT / scale || length < (settings.minimumPasses + 2) * extent ||
        transverseMax - transverseMin > (sensitivity <= 0.5 ? std::lerp(1.25, 4.0, 2 * sensitivity) :
                                                              std::lerp(4.0, 16.0, 2 * sensitivity - 1)) *
                                                extent) {
        return std::nullopt;
    }
    double start = projection(points.front());
    double extreme = start;
    int direction = 0;
    size_t passes = 0;
    std::array<double, SAMPLE_COUNT> passLengths{};
    const auto countPass = [&]() {
        if (std::abs(extreme - start) >= MIN_UNEVEN_PASS_RATIO * extent) {
            if (passes < passLengths.size()) {
                passLengths[passes] = std::abs(extreme - start);
            }
            ++passes;
        }
    };
    for (const Point& point: points.subspan(1)) {
        const double value = projection(point);
        if (direction == 0) {
            if (std::abs(value - start) >= std::lerp(0.2, 0.1, sensitivity) * extent) {
                direction = value > start ? 1 : -1;
                extreme = value;
            }
        } else if (direction * (value - extreme) > 0) {
            extreme = value;
        } else if (direction * (extreme - value) >= std::lerp(0.2, 0.1, sensitivity) * extent) {
            countPass();
            if (passes > SAMPLE_COUNT) {
                return std::nullopt;
            }
            start = extreme;
            extreme = value;
            direction = -direction;
        }
    }
    countPass();
    // Moving scrubs need extra passes at lower sensitivity. Keep
    // ordinary hatching, short zigzags and unbounded oscillations as drawing.
    if (passes < static_cast<size_t>(settings.minimumPasses + std::lround(std::lerp(12.0, 0.0, sensitivity))) ||
        passes > SAMPLE_COUNT) {
        return std::nullopt;
    }
    // Confirm target revisits against a typical pass, rather than the entire
    // drift/trailing tail of an uneven scrub.
    std::nth_element(passLengths.begin(), passLengths.begin() + passes / 2, passLengths.begin() + passes);
    return ScribbleGestureRecognizer::Geometry{origin, axis, scale, passLengths[passes / 2]};
}
}  // namespace

auto ScribbleGestureRecognizer::analyze(std::span<const Point> points, ScribbleSettings settings)
        -> std::optional<Geometry> {
    settings = settings.normalized();
    const double sensitivity = settings.sensitivity / 100.0;
    if (points.size() < static_cast<size_t>(settings.minimumPasses + 1)) {
        return std::nullopt;
    }

    Position minimum{points.front().x, points.front().y};
    Position maximum = minimum;
    for (const Point& point: points) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
            return std::nullopt;
        }
        minimum.x = std::min(minimum.x, point.x);
        minimum.y = std::min(minimum.y, point.y);
        maximum.x = std::max(maximum.x, point.x);
        maximum.y = std::max(maximum.y, point.y);
    }
    const double scale = std::max(maximum.x - minimum.x, maximum.y - minimum.y);
    if (!std::isfinite(scale) || scale < MIN_EXTENT / 2) {
        return std::nullopt;
    }

    // Work in bounded coordinates so covariance and distances cannot overflow on large page coordinates.
    double length = 0;
    Position previous = normalize(points.front(), minimum, scale);
    for (const Point& point: points.subspan(1)) {
        const Position current = normalize(point, minimum, scale);
        length += current.distance(previous);
        previous = current;
    }
    if (length == 0 || length > std::numeric_limits<double>::max() / scale) {
        return std::nullopt;
    }

    std::array<Position, SAMPLE_COUNT> samples;
    if (!resample(points, minimum, scale, length, samples)) {
        return analyzeUnevenScrub(points, minimum, scale, length, settings);
    }
    Position mean{};
    double resampledLength = 0;
    for (size_t i = 0; i < samples.size(); ++i) {
        mean += samples[i] / static_cast<double>(SAMPLE_COUNT);
        if (i > 0) {
            resampledLength += samples[i].distance(samples[i - 1]);
        }
    }
    // Reject under-resolved paths rather than classify a different, simplified trajectory.
    if (resampledLength < MIN_RESAMPLED_LENGTH_RATIO * length) {
        return analyzeUnevenScrub(points, minimum, scale, length, settings);
    }

    double xx = 0;
    double xy = 0;
    double yy = 0;
    for (const Position& sample: samples) {
        const Position centered = sample - mean;
        xx += centered.x * centered.x;
        xy += centered.x * centered.y;
        yy += centered.y * centered.y;
    }
    const double trace = xx + yy;
    if (trace <= 0 || std::hypot(xx - yy, 2 * xy) < std::lerp(0.95, 0.75, sensitivity) * trace) {
        return analyzeUnevenScrub(points, minimum, scale, length, settings);
    }
    const double angle = 0.5 * std::atan2(2 * xy, xx - yy);
    const Position axis{std::cos(angle), std::sin(angle)};

    std::array<double, SAMPLE_COUNT> projections;
    for (size_t i = 0; i < samples.size(); ++i) {
        projections[i] = samples[i].x * axis.x + samples[i].y * axis.y;
    }
    double minProjection = std::numeric_limits<double>::infinity();
    double maxProjection = -std::numeric_limits<double>::infinity();
    double transverseMin = std::numeric_limits<double>::infinity();
    double transverseMax = -std::numeric_limits<double>::infinity();
    // Preserve excursions that might fall between samples, including short off-axis spikes.
    for (const Point& point: points) {
        const Position position = normalize(point, minimum, scale);
        const double projection = position.x * axis.x + position.y * axis.y;
        minProjection = std::min(minProjection, projection);
        maxProjection = std::max(maxProjection, projection);
        const double transverse = position.y * axis.x - position.x * axis.y;
        transverseMin = std::min(transverseMin, transverse);
        transverseMax = std::max(transverseMax, transverse);
    }
    const double extent = maxProjection - minProjection;
    if (extent < MIN_EXTENT / scale || transverseMax - transverseMin > std::lerp(0.1, 0.4, sensitivity) * extent ||
        length < (settings.minimumPasses - 0.5) * extent) {
        return analyzeUnevenScrub(points, minimum, scale, length, settings);
    }
    if (!hasRepeatedPasses(projections, extent, settings)) {
        return analyzeUnevenScrub(points, minimum, scale, length, settings);
    }
    return Geometry{minimum, axis, scale, extent};
}

auto ScribbleGestureRecognizer::isCandidate(std::span<const Point> points, ScribbleSettings settings) -> bool {
    return analyze(points, settings).has_value();
}
