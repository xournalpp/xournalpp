#include "ScribbleEraser.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include "model/Document.h"
#include "model/Element.h"
#include "model/Layer.h"
#include "model/Point.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/DeleteUndoAction.h"
#include "util/Range.h"

#include "ScribbleGestureRecognizer.h"

namespace {
using Position = xoj::util::Point<double>;
using Geometry = ScribbleGestureRecognizer::Geometry;
// Bound temporary ink expansion and narrow-phase work. Exhaustion preserves the entire gesture,
// including any targets already confirmed earlier in the scan.
constexpr size_t MAX_PIECES = 32768;
constexpr size_t MAX_PAIR_TESTS = 250000;
constexpr double MAX_COORDINATE = 1e6;
struct ContactBudget {
    size_t bounds = 2000000;
    size_t exact = MAX_PAIR_TESTS;
};

auto boundsOverlap(const auto& a, const auto& b) -> bool {
    // sqrt(2) also encloses square caps at every angle.
    const double margin = std::sqrt(2.0) * (a.radius + b.radius);
    return std::max(a.a.x, a.b.x) + margin >= std::min(b.a.x, b.b.x) &&
           std::max(b.a.x, b.b.x) + margin >= std::min(a.a.x, a.b.x) &&
           std::max(a.a.y, a.b.y) + margin >= std::min(b.a.y, b.b.y) &&
           std::max(b.a.y, b.b.y) + margin >= std::min(a.a.y, a.b.y);
}

struct InkPiece {
    Position a;
    Position b;
    double radius;
    StrokeCapStyle::Value startCap;
    StrokeCapStyle::Value endCap;
};

struct InkPath {
    std::vector<InkPiece> pieces;
    std::vector<Position> filling;
};

auto dot(Position a, Position b) -> double { return a.x * b.x + a.y * b.y; }
auto cross(Position a, Position b) -> double { return a.x * b.y - a.y * b.x; }

auto normalize(const Point& point, const Geometry& frame) -> std::optional<Position> {
    const Position value{(point.x - frame.origin.x) / frame.scale, (point.y - frame.origin.y) / frame.scale};
    if (!std::isfinite(value.x) || !std::isfinite(value.y) || std::abs(value.x) > MAX_COORDINATE ||
        std::abs(value.y) > MAX_COORDINATE) {
        return std::nullopt;
    }
    return value;
}

auto buildInk(const Stroke& stroke, const Geometry& frame, InkPath& result) -> bool {
    const auto& points = stroke.getPointVector();
    const int capValue = stroke.getStrokeCapStyle();
    if (!std::isfinite(stroke.getWidth()) || stroke.getWidth() <= 0 || capValue < StrokeCapStyle::ROUND ||
        capValue > StrokeCapStyle::SQUARE || stroke.getFill() < -1 || stroke.getFill() > 255 ||
        points.size() > MAX_PIECES) {
        return false;
    }
    if (points.size() < 2 || stroke.getColor().alpha == 0) {
        return true;
    }
    std::vector<Position> positions;
    std::vector<double> radii;
    positions.reserve(points.size());
    radii.reserve(points.size() - 1);
    const bool pressure = stroke.hasPressure() && stroke.getToolType() != StrokeTool::HIGHLIGHTER;
    for (size_t i = 0; i < points.size(); ++i) {
        auto position = normalize(points[i], frame);
        // Stored pressure belongs to the segment starting at this point. Loaded
        // and split strokes use NO_PRESSURE on the final, segment-less point.
        const bool unusedFinalPressure = i + 1 == points.size() && points[i].z == Point::NO_PRESSURE;
        if (!position || (pressure && !unusedFinalPressure && (!std::isfinite(points[i].z) || points[i].z <= 0))) {
            return false;
        }
        positions.push_back(*position);
        if (i + 1 < points.size()) {
            const double radius = (pressure ? points[i].z : stroke.getWidth()) / (2 * frame.scale);
            if (!std::isfinite(radius) || radius <= 0 || radius > MAX_COORDINATE) {
                return false;
            }
            radii.push_back(radius);
        }
    }
    if (stroke.getFill() > 0) {
        result.filling = positions;
    }

    std::vector<double> dashes;
    double period = 0;
    if (stroke.getLineStyle().getDashes().size() > MAX_PIECES) {
        return false;
    }
    for (double dash: stroke.getLineStyle().getDashes()) {
        if (!std::isfinite(dash) || dash < 0) {
            return false;
        }
        dash /= frame.scale;
        period += dash;
        dashes.push_back(dash);
    }
    if (!dashes.empty() && (!std::isfinite(period) || period <= 0)) {
        return false;
    }

    // Pressure contours use round ends. Near width changes, use an inscribed width rather than
    // the largest neighboring pressure, so a broad bounding box cannot cause false contact.
    const auto cap = pressure ? StrokeCapStyle::ROUND : static_cast<StrokeCapStyle::Value>(capValue);
    size_t dashIndex = 0;
    double remaining = dashes.empty() ? 0 : dashes.front();
    bool on = true;
    bool dashStarted = true;
    size_t steps = 0;
    const auto advanceDash = [&]() {
        dashIndex = (dashIndex + 1) % dashes.size();
        on = !on;
        remaining = dashes[dashIndex];
        dashStarted = true;
    };
    for (size_t i = 0; i + 1 < positions.size(); ++i) {
        double radius = radii[i];
        if (pressure) {
            if (i > 0) {
                radius = std::min(radius, radii[i - 1]);
            }
            if (i + 1 < radii.size()) {
                radius = std::min(radius, radii[i + 1]);
            }
        }
        const Position a = positions[i];
        const Position b = positions[i + 1];
        const double length = a.distance(b);
        if (dashes.empty()) {
            if (length > 0 || cap == StrokeCapStyle::ROUND) {
                result.pieces.push_back({a, b, radius, i == 0 ? cap : StrokeCapStyle::ROUND,
                                         i + 2 == positions.size() ? cap : StrokeCapStyle::ROUND});
            }
        } else {
            double offset = 0;
            while (offset < length) {
                if (++steps > MAX_PIECES) {
                    return false;
                }
                if (remaining == 0) {
                    if (on && cap == StrokeCapStyle::ROUND) {
                        const Position point = a + (b - a) * (offset / length);
                        result.pieces.push_back({point, point, radius, cap, cap});
                    }
                    advanceDash();
                    continue;
                }
                const double distance = std::min(remaining, length - offset);
                const double next = offset + distance;
                if (next <= offset) {
                    return false;
                }
                const bool dashEnds = distance == remaining;
                if (on) {
                    result.pieces.push_back({a + (b - a) * (offset / length), a + (b - a) * (next / length), radius,
                                             offset == 0 && i > 0 && !dashStarted ? StrokeCapStyle::ROUND : cap,
                                             !dashEnds && i + 2 < positions.size() ? StrokeCapStyle::ROUND : cap});
                }
                offset = next;
                remaining -= distance;
                dashStarted = false;
                if (dashEnds) {
                    advanceDash();
                }
            }
        }
        if (result.pieces.size() > MAX_PIECES) {
            return false;
        }
    }
    return true;
}

struct Distance {
    double value;
    double parameter;
};

auto pointDistance(Position point, Position a, Position b) -> Distance {
    const Position direction = b - a;
    const double squaredLength = dot(direction, direction);
    const double parameter = squaredLength > 0 ? std::clamp(dot(point - a, direction) / squaredLength, 0.0, 1.0) : 0;
    return {point.distance(a + direction * parameter), parameter};
}

auto segmentDistance(Position a, Position b, Position c, Position d) -> Distance {
    const Position ab = b - a;
    const Position cd = d - c;
    const double denominator = cross(ab, cd);
    if (denominator != 0) {
        const double t = cross(c - a, cd) / denominator;
        const double u = cross(c - a, ab) / denominator;
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            return {0, t};
        }
    }
    Distance best{pointDistance(a, c, d).value, 0};
    const Distance fromEnd{pointDistance(b, c, d).value, 1};
    if (fromEnd.value < best.value) {
        best = fromEnd;
    }
    for (Position end: {c, d}) {
        const Distance candidate = pointDistance(end, a, b);
        if (candidate.value < best.value) {
            best = candidate;
        }
    }
    return best;
}

auto rounded(const InkPiece& piece) -> bool {
    return piece.startCap == StrokeCapStyle::ROUND && piece.endCap == StrokeCapStyle::ROUND;
}

struct InkShape {
    Position origin;
    Position axis;
    double length;
    double first;
    double last;
    double radius;
    std::array<Position, 4> corners{};
    std::array<Position, 2> circles{};
    size_t circleCount = 0;
};

auto shapeOf(const InkPiece& piece) -> InkShape {
    const double length = piece.a.distance(piece.b);
    const Position axis = length > 0 ? (piece.b - piece.a) / length : Position{1, 0};
    const Position normal{-axis.y, axis.x};
    const double first = piece.startCap == StrokeCapStyle::SQUARE ? -piece.radius : 0;
    const double last = length + (piece.endCap == StrokeCapStyle::SQUARE ? piece.radius : 0);
    InkShape shape{piece.a, axis, length, first, last, piece.radius};
    shape.corners = {piece.a + axis * first - normal * piece.radius, piece.a + axis * last - normal * piece.radius,
                     piece.a + axis * last + normal * piece.radius, piece.a + axis * first + normal * piece.radius};
    if (piece.startCap == StrokeCapStyle::ROUND) {
        shape.circles[shape.circleCount++] = piece.a;
    }
    if (piece.endCap == StrokeCapStyle::ROUND && (length > 0 || shape.circleCount == 0)) {
        shape.circles[shape.circleCount++] = piece.b;
    }
    return shape;
}

auto insideBody(const InkShape& shape, Position point) -> bool {
    const Position delta = point - shape.origin;
    const double projection = dot(delta, shape.axis);
    return shape.length > 0 && projection >= shape.first && projection <= shape.last &&
           std::abs(cross(shape.axis, delta)) <= shape.radius;
}

auto parameterOn(const InkShape& shape, Position point) -> double {
    return shape.length > 0 ? std::clamp(dot(point - shape.origin, shape.axis) / shape.length, 0.0, 1.0) : 0;
}

auto closestOnBody(const InkShape& shape, Position point) -> Position {
    const Position delta = point - shape.origin;
    const double x = std::clamp(dot(delta, shape.axis), shape.first, shape.last);
    const double y = std::clamp(cross(shape.axis, delta), -shape.radius, shape.radius);
    return shape.origin + shape.axis * x + Position{-shape.axis.y, shape.axis.x} * y;
}

auto contact(const InkPiece& gesture, const InkPiece& target) -> std::optional<double> {
    if (rounded(gesture) && rounded(target)) {
        const Distance distance = segmentDistance(gesture.a, gesture.b, target.a, target.b);
        return distance.value <= gesture.radius + target.radius ? std::optional(distance.parameter) : std::nullopt;
    }
    const InkShape g = shapeOf(gesture);
    const InkShape t = shapeOf(target);
    if (g.length > 0 && t.length > 0) {
        for (Position corner: g.corners) {
            if (insideBody(t, corner)) {
                return parameterOn(g, corner);
            }
        }
        for (Position corner: t.corners) {
            if (insideBody(g, corner)) {
                return parameterOn(g, corner);
            }
        }
        for (size_t i = 0; i < g.corners.size(); ++i) {
            const Position a = g.corners[i];
            const Position b = g.corners[(i + 1) % g.corners.size()];
            for (size_t j = 0; j < t.corners.size(); ++j) {
                const Distance distance = segmentDistance(a, b, t.corners[j], t.corners[(j + 1) % t.corners.size()]);
                if (distance.value == 0) {
                    return parameterOn(g, a + (b - a) * distance.parameter);
                }
            }
        }
    }
    for (size_t i = 0; i < t.circleCount && g.length > 0; ++i) {
        const Position point = closestOnBody(g, t.circles[i]);
        if (point.distance(t.circles[i]) <= t.radius) {
            return parameterOn(g, point);
        }
    }
    for (size_t i = 0; i < g.circleCount; ++i) {
        if (t.length > 0 && g.circles[i].distance(closestOnBody(t, g.circles[i])) <= g.radius) {
            return parameterOn(g, g.circles[i]);
        }
        for (size_t j = 0; j < t.circleCount; ++j) {
            if (g.circles[i].distance(t.circles[j]) <= g.radius + t.radius) {
                return parameterOn(g, g.circles[i]);
            }
        }
    }
    return std::nullopt;
}

auto insideFilling(Position point, const std::vector<Position>& polygon) -> bool {
    int winding = 0;
    for (size_t i = 0; i < polygon.size(); ++i) {
        const Position a = polygon[i];
        const Position b = polygon[(i + 1) % polygon.size()];
        const double side = cross(b - a, point - a);
        if (a.y <= point.y && b.y > point.y && side > 0) {
            ++winding;
        } else if (a.y > point.y && b.y <= point.y && side < 0) {
            --winding;
        }
    }
    return winding != 0;
}

auto repeatedContact(const InkPath& gesture, const InkPath& target, const Geometry& frame, ContactBudget& budget,
                     const ScribbleSettings& settings) -> std::optional<bool> {
    const double sensitivity = settings.sensitivity / 100.0;
    const double returnExcursion = std::lerp(0.55, 0.15, sensitivity);
    const double minNormalExcursion = std::lerp(0.15, 0.05, sensitivity);
    std::optional<double> firstContact;
    double minimum = 0;
    double maximum = 0;
    std::optional<Position> targetNormal;
    double normalStart = 0;
    double normalMin = 0;
    double normalMax = 0;
    double normalExcursion = 0;
    for (const InkPiece& piece: gesture.pieces) {
        std::optional<double> parameter;
        std::optional<double> lastParameter;
        const InkPiece* contactedInk = nullptr;
        for (const InkPiece& ink: target.pieces) {
            if (budget.bounds == 0) {
                return std::nullopt;
            }
            --budget.bounds;
            if (!boundsOverlap(piece, ink)) {
                continue;
            }
            const size_t cost = rounded(piece) && rounded(ink) ? 2 : 32;
            if (budget.exact < cost) {
                return std::nullopt;
            }
            budget.exact -= cost;
            if (auto hit = contact(piece, ink)) {
                if (!parameter || *hit < *parameter) {
                    parameter = hit;
                    contactedInk = &ink;
                }
                // A straight gesture segment can enter and leave a closed shape.
                // Its later contact can confirm a return after an earlier reversal.
                const InkPiece reverse{piece.b, piece.a, piece.radius, piece.endCap, piece.startCap};
                if (auto last = contact(reverse, ink)) {
                    lastParameter = std::max(lastParameter.value_or(0), 1 - *last);
                }
            }
        }
        if (!target.filling.empty()) {
            if (budget.exact < 2 * target.filling.size() + 32) {
                return std::nullopt;
            }
            budget.exact -= 2 * target.filling.size() + 32;
            if (insideFilling(piece.a, target.filling)) {
                parameter = 0;
            }
            if (insideFilling(piece.b, target.filling)) {
                if (!parameter)
                    parameter = 1;
                lastParameter = 1;
            }
            const InkPiece closing{target.filling.back(), target.filling.front(), 0, StrokeCapStyle::BUTT,
                                   StrokeCapStyle::BUTT};
            if (auto hit = closing.a != closing.b ? contact(piece, closing) : std::nullopt;
                hit && (!parameter || *hit < *parameter)) {
                parameter = hit;
            }
            const InkPiece reverse{piece.b, piece.a, piece.radius, piece.endCap, piece.startCap};
            if (auto last = closing.a != closing.b ? contact(reverse, closing) : std::nullopt) {
                lastParameter = std::max(lastParameter.value_or(0), 1 - *last);
            }
        }
        if (firstContact) {
            const double start = dot(piece.a, frame.axis);
            minimum = std::min(minimum, start);
            maximum = std::max(maximum, start);
        }
        if (targetNormal) {
            const double value = dot(piece.a, *targetNormal);
            normalMin = std::min(normalMin, value);
            normalMax = std::max(normalMax, value);
        }
        if (parameter) {
            for (double value: {*parameter, lastParameter.value_or(*parameter)}) {
                const Position position = piece.a + (piece.b - piece.a) * value;
                const double projection = dot(position, frame.axis);
                if (!firstContact) {
                    firstContact = projection;
                    minimum = maximum = projection;
                    if (contactedInk) {
                        const Position direction = contactedInk->b - contactedInk->a;
                        const double length = contactedInk->a.distance(contactedInk->b);
                        if (length > std::numeric_limits<double>::epsilon()) {
                            targetNormal = Position{-direction.y / length, direction.x / length};
                            normalStart = normalMin = normalMax = dot(position, *targetNormal);
                            // Thin ink must not turn sub-pixel jitter during one visit into an erase.
                            normalExcursion =
                                    std::clamp(contactedInk->radius + piece.radius, minNormalExcursion * frame.extent,
                                               returnExcursion * frame.extent);
                        }
                    }
                } else {
                    // Repeated hits may land at different positions along the same
                    // target. Check real reversals along the scrub and across the
                    // contacted segment, including scrubbing within a thick line.
                    const double excursion = returnExcursion * frame.extent;
                    const double normal = targetNormal ? dot(position, *targetNormal) : 0;
                    const bool crossedBack =
                            targetNormal &&
                            ((normalMax - normalStart >= normalExcursion && normalMax - normal >= normalExcursion) ||
                             (normalStart - normalMin >= normalExcursion && normal - normalMin >= normalExcursion));
                    if (crossedBack || (maximum - *firstContact >= excursion && maximum - projection >= excursion) ||
                        (*firstContact - minimum >= excursion && projection - minimum >= excursion)) {
                        return true;
                    }
                }
                minimum = std::min(minimum, projection);
                maximum = std::max(maximum, projection);
                if (targetNormal) {
                    const double normal = dot(position, *targetNormal);
                    normalMin = std::min(normalMin, normal);
                    normalMax = std::max(normalMax, normal);
                }
            }
        }
        if (firstContact) {
            const double end = dot(piece.b, frame.axis);
            minimum = std::min(minimum, end);
            maximum = std::max(maximum, end);
        }
        if (targetNormal) {
            const double value = dot(piece.b, *targetNormal);
            normalMin = std::min(normalMin, value);
            normalMax = std::max(normalMax, value);
        }
    }
    return false;
}

auto validBounds(const xoj::util::Rectangle<double>& box) -> bool {
    return std::isfinite(box.x) && std::isfinite(box.y) && std::isfinite(box.width) && std::isfinite(box.height) &&
           box.width >= 0 && box.height >= 0;
}
}  // namespace

auto ScribbleEraser::erase(Document& document, const PageRef& page, const Stroke& gesture, Range& changedRange,
                           ScribbleSettings settings) -> std::unique_ptr<DeleteUndoAction> {
    if (!page || gesture.getToolType() != StrokeTool::PEN) {
        return nullptr;
    }
    settings = settings.normalized();
    const auto frame = ScribbleGestureRecognizer::analyze(gesture.getPointVector(), settings);
    InkPath path;
    if (!frame || !buildInk(gesture, *frame, path) || path.pieces.empty()) {
        return nullptr;
    }
    const auto& box = gesture.getBoundingBox();
    if (!validBounds(box)) {
        return nullptr;
    }
    std::unique_lock lock(document);
    const auto layerId = page->getSelectedLayerId();
    if (document.indexOf(page) == npos || layerId == 0 || layerId > page->getLayerCount() ||
        !page->isLayerVisible(layerId)) {
        return nullptr;
    }
    Layer* layer = page->getSelectedLayer();
    if (layer->indexOf(&gesture) != Element::InvalidIndex) {
        return nullptr;
    }
    InsertionOrderRef targets;
    Range range(box);
    ContactBudget budget;
    const auto& elements = layer->getElements();
    for (size_t i = 0; i < elements.size(); ++i) {
        if (elements[i]->getType() != ELEMENT_STROKE) {
            continue;
        }
        const auto& target = static_cast<const Stroke&>(*elements[i]);
        if (target.getPointCount() < 2) {
            continue;
        }
        if (target.getStrokeCapStyle() < StrokeCapStyle::ROUND || target.getStrokeCapStyle() > StrokeCapStyle::SQUARE) {
            return nullptr;
        }
        const auto& targetBox = target.getBoundingBox();
        if (!validBounds(targetBox)) {
            return nullptr;
        }
        if (!box.intersects(targetBox)) {
            continue;
        }
        InkPath ink;
        if (target.getErasable() || !buildInk(target, *frame, ink)) {
            return nullptr;
        }
        const auto hit = repeatedContact(path, ink, *frame, budget, settings);
        if (!hit) {
            return nullptr;
        }
        if (*hit) {
            targets.emplace_back(&target, static_cast<Element::Index>(i));
            range = range.unite(Range(targetBox));
        }
    }
    auto action = DeleteUndoAction::deleteElements(page, layer, targets);
    if (action) {
        changedRange = range;
    }
    return action;
}
