/**
 * Xournal++
 *
 * A template for intersecting paths with padded boxes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/eraser/PaddedBox.h"
#include "util/Assert.h"
#include "util/Interval.h"

#include "Path.h"

#define DEBUG_ERASER(f)

// template <typename SegmentType>
// auto Path::intersectWithPaddedBoxTemplate(const PaddedBox& box, size_t firstIndex, size_t lastIndex,
//                                           Path::SegmentIteratable<const SegmentType> segments) const
//         -> IntersectionParametersContainer {
//     xoj_assert(firstIndex <= lastIndex && lastIndex < segments.size());
//
//     const auto innerBox = box.getInnerRectangle();
//     const auto outerBox = box.getOuterRectangle();
//
//     size_t index = firstIndex;
//     bool wentInsideInner = false;
//     bool isInsideOuter = false;
//     IntersectionParametersContainer result;
//
//     auto it = segments.iteratorAt(firstIndex);
//
//     /**
//      * Initialize with the half tangent at the first point
//      */
//     {  // Scope for initialization at first point
//         auto [firstPoint, controlPoint] = it->getLeftHalfTangent();
//         isInsideOuter = firstPoint.get().isInside(outerBox);
//
//         if (firstPoint.get().isInside(innerBox)) {
//             /**
//              * The stroke starts in the eraser box
//              * OR
//              * The half line starting from the second point and passing by the first point intersects the eraser
//              * and the first point is in the padded eraser box
//              *
//              * Erasing the tip of the stroke. Add a fake intersection parameter
//              */
//             result.emplace_back(firstIndex, 0.0);
//             wentInsideInner = true;
//         } else if (isInsideOuter) {
//             std::optional<Interval<double>> innerLineIntersections =
//                     intersectLineWithRectangle(firstPoint, controlPoint, innerBox);
//             if (innerLineIntersections && innerLineIntersections.value().max <= 0.0) {
//                 /**
//                  * The half tangent at the first point intersects the eraser and the first point is in the padding.
//                  *
//                  * Erasing the tip of the stroke. Add a fake intersection parameter
//                  */
//                 wentInsideInner = true;
//             }
//             result.emplace_back(firstIndex, 0.0);
//         }
//     }  // end of scope
//
//     auto endIt = segments.iteratorAt(lastIndex + 1);
//     for (; it != endIt; ++it, ++index) {
//         auto outerIntersections = it->intersectWithRectangle(outerBox);
//         if (!outerIntersections.empty() || it->firstKnot.isInside(outerBox) || it->secondKnot.isInside(outerBox)) {
//             // Some part of the segment lies inside the padded box
//             auto innerIntersections = it->intersectWithRectangle(innerBox);
//             auto itInner = innerIntersections.begin();
//             auto itInnerEnd = innerIntersections.end();
//             auto itOuter = outerIntersections.begin();
//             auto itOuterEnd = outerIntersections.end();
//             while (itOuter != itOuterEnd) {
//                 if (itInner != itInnerEnd && *itInner < *itOuter) {
//                     wentInsideInner = true;
//                     ++itInner;
//                 } else {
//                     if (isInsideOuter) {
//                         // Leaving the padded box
//                         if (wentInsideInner) {
//                             /**
//                              * The stroke went in and out of the padded box and touched the eraser box.
//                              * Erase this section
//                              */
//                             result.emplace_back(index, *itOuter);
//                             wentInsideInner = false;
//                         } else {
//                             /**
//                              * The stroke went in and out of the padding without touching the eraser box.
//                              * This section should not be erased
//                              */
//                             if (!result.empty()) {
//                                 result.pop_back();
//                             }
//                         }
//                         isInsideOuter = false;
//                     } else {
//                         // Going in the padded box
//                         result.emplace_back(index, *itOuter);
//                         isInsideOuter = true;
//                     }
//                     ++itOuter;
//                 }
//             }
//             if (itInner != itInnerEnd) {
//                 wentInsideInner = true;
//             }
//         }
//     }
//
//     if (result.size() % 2) {
//         // Same as with the first point
//         --it;  // Back to the last segment
//         auto [controlPoint, lastPoint] = it->getRightHalfTangent();
//
//         if (lastPoint.get().isInside(innerBox)) {
//             /**
//              * The stroke ends in the eraser box
//              *
//              * Erasing the tip of the stroke. Add a fake intersection parameter
//              */
//             result.emplace_back(index - 1, 1.0);
//             wentInsideInner = true;
//         } else {
//             xoj_assert(lastPoint.get().isInside(outerBox));
//             // The last point has to be in the padding!
//             std::optional<Interval<double>> innerLineIntersections =
//                     intersectLineWithRectangle(controlPoint, lastPoint, innerBox);
//             if (innerLineIntersections && innerLineIntersections.value().min > 1.0) {
//                 /**
//                  * The half tangent at the last point intersects the eraser and the last point is in the padding.
//                  *
//                  * Erasing the tip of the stroke. Add a fake intersection parameter
//                  */
//                 result.emplace_back(index - 1, 1.0);
//                 wentInsideInner = true;
//             } else {
//                 /**
//                  * The stroke ends in the padding but its prolongation does not hit the eraser box
//                  *
//                  * Drop the last intersection (corresponding to going in the padding).
//                  */
//                 result.pop_back();
//             }
//         }
//     }
//     return result;
// }


template <typename SegmentType>
auto Path::intersectWithPaddedBoxTemplate(const PaddedBox& box, size_t firstIndex, size_t lastIndex,
                                          Path::SegmentIteratable<const SegmentType> segments) const
        -> IntersectionParametersContainer {
    xoj_assert(firstIndex <= lastIndex && lastIndex < segments.size());

    const auto innerBox = box.getInnerRectangle();
    const auto outerBox = box.getOuterRectangle();

    struct Flags {
        bool isInsideOuter;
        bool wentInsideInner;
    };

    auto initializeFlagsFromHalfTangentAtFirstKnot =
            [&outerBox, &innerBox](const Point& firstKnot, const Point& halfTangentControlPoint) -> Flags {
        if (firstKnot.isInside(innerBox)) {
            return {true, true};
        } else if (firstKnot.isInside(outerBox)) {
            std::optional<Interval<double>> innerLineIntersections =
                    intersectLineWithRectangle(firstKnot, halfTangentControlPoint, innerBox);
            // If the half tangent goes towards to inner box, say we've been inside the inner box
            return {true, innerLineIntersections && innerLineIntersections.value().max <= 0.0};
        }
        return {false, false};
    };

    size_t index = firstIndex;

    auto segmentIt = segments.iteratorAt(index);

    auto [firstPoint, controlPoint] = segmentIt->getLeftHalfTangent();
    Flags flags = initializeFlagsFromHalfTangentAtFirstKnot(firstPoint.get(), controlPoint.get());

    DEBUG_ERASER(auto debugstream = serdes_stream<std::stringstream>();
                 debugstream << "Stroke::intersectWithPaddedBox debug:\n"; debugstream << std::boolalpha;
                 debugstream << "| * flags.isInsideOuter              = " << flags.isInsideOuter << std::endl;
                 debugstream << "| * flags.wentInsideInner            = " << flags.wentInsideInner << std::endl;
                 debugstream << std::fixed;)

    IntersectionParametersContainer result;
    if (flags.isInsideOuter) {
        // We start inside the padded box. Add a fake intersection parameter
        result.emplace_back(firstIndex, 0.0);
    }

    auto processSegment = [&flags, &outerBox, &innerBox, &result DEBUG_ERASER(COMMA & debugstream)](
                                  const SegmentType& seg, size_t index) {
        DEBUG_ERASER(debugstream << "| * Segment " << std::setw(3) << index << std::setprecision(5);
                     debugstream << ": (" << std::setw(9) << firstKnot.x;
                     debugstream << " ; " << std::setw(9) << firstKnot.y;
                     debugstream << ") -- (" << std::setw(9) << secondKnot.x;
                     debugstream << " ; " << std::setw(9) << secondKnot.y;
                     debugstream << ")\n"
                                 << std::setprecision(17);  // High precision to detect numerical inaccuracy
        )

        auto outerIntersections = seg.intersectWithRectangle(outerBox);
        if (!outerIntersections.empty() || seg.firstKnot.isInside(outerBox) || seg.secondKnot.isInside(outerBox)) {
            // Some part of the segment lies inside the padded box
            auto innerIntersections = seg.intersectWithRectangle(innerBox);
            auto itInner = innerIntersections.begin();
            auto itInnerEnd = innerIntersections.end();

            auto skipInnerIntersectionsBelowValue = [&itInner, itInnerEnd](double upToValue) -> bool {
                bool skipSome = false;
                while (itInner != itInnerEnd && *itInner < upToValue) {
                    skipSome = true;
                    ++itInner;
                }
                return skipSome;
            };

            for (auto outerIntersection: outerIntersections) {
                flags.wentInsideInner |= skipInnerIntersectionsBelowValue(outerIntersection);

                DEBUG_ERASER(debugstream << "|  |  ** "
                                         << "wentInsideInner = " << flags.wentInsideInner << std::endl;)

                if (!flags.isInsideOuter || flags.wentInsideInner) {
                    result.emplace_back(index, outerIntersection);

                    DEBUG_ERASER(
                            if (!flags.isInsideOuter) {
                                debugstream << "|  |  ** going-in  (" << std::setw(3) << index << "," << std::setw(20)
                                            << outerIntersection << ")" << std::endl;
                            } if (flags.wentInsideInner) {
                                debugstream << "|  |  ** going-out (" << std::setw(3) << index << "," << std::setw(20)
                                            << outerIntersection << ")" << std::endl;
                            })
                } else {
                    xoj_assert(!result.empty());
                    DEBUG_ERASER(debugstream << "|  |  ** popping   (" << std::setw(3) << result.back().index << ","
                                             << std::setw(20) << result.back().t << ")" << std::endl;)
                    result.pop_back();
                }
                flags.wentInsideInner = false;
                flags.isInsideOuter = !flags.isInsideOuter;
            }
            if (itInner != itInnerEnd) {
                flags.wentInsideInner = true;
            }
        }
        DEBUG_ERASER(debugstream << "|  |__** result.size() = " << std::setw(3) << result.size() << std::endl;)
    };

    auto endSegmentIt = segments.iteratorAt(lastIndex + 1);
    for (; segmentIt != endSegmentIt; segmentIt++, index++) {
        processSegment(*segmentIt, index);
    }

    auto isHalfTangentAtLastKnotGoingTowardInnerBox =
            [&innerBox, &outerBox](const Point& lastKnot, const Point& halfTangentControlPoint) -> bool {
        xoj_assert(lastKnot.isInside(outerBox));
        std::optional<Interval<double>> innerLineIntersections =
                intersectLineWithRectangle(lastKnot, halfTangentControlPoint, innerBox);
        return innerLineIntersections && innerLineIntersections.value().max < 0.0;
    };

    /**
     * Due to numerical imprecision, we could get inconsistent results
     * (typically when a stroke's point lies on outerBox' boundary).
     * We try and detect those cases, and simply drop them
     */
    bool inconsistentResults = false;
    if (result.size() % 2) {
        // Not necessarily inconsistent: could be the stroke ends in outerBox
        --segmentIt;
        auto [controlPoint, lastPoint] = segmentIt->getRightHalfTangent();

        DEBUG_ERASER(debugstream << "|  |  Odd number of intersection points" << std::endl;)

        if (lastPoint.get().isInside(outerBox)) {
            if (flags.wentInsideInner ||
                isHalfTangentAtLastKnotGoingTowardInnerBox(lastPoint.get(), controlPoint.get())) {
                result.emplace_back(index - 1, 1.0);
                DEBUG_ERASER(debugstream << "|  |  ** pushing   (" << std::setw(3) << result.back().index << ","
                                         << std::setw(20) << result.back().t << ")" << std::endl;)
            } else {
                DEBUG_ERASER(debugstream << "|  |  ** popping   (" << std::setw(3) << result.back().index << ","
                                         << std::setw(20) << result.back().t << ")" << std::endl;)
                result.pop_back();
            }
        } else {
            inconsistentResults = true;
        }
    }

    // Returns true if the result is inconsistent
    auto checkSanity = [&outerBox, &segments DEBUG_ERASER(COMMA & debugstream)](
                               const IntersectionParametersContainer& res) -> bool {
        for (auto it1 = res.begin(), it2 = std::next(it1), end = res.end(); it1 != end; it1 += 2, it2 += 2) {
            const auto& paramStart = *it1;
            const auto& paramLast = *it2;
            DEBUG_ERASER(debugstream << "| SanityCheck on parameters (" << paramStart.index << " ; " << paramStart.t
                                     << ") -- (" << paramLast.index << " ; " << paramLast.t << ")" << std::endl;)
            Point testPoint;
            // Get a point on the stroke within the interval of parameters
            if (paramStart.index == paramLast.index) {
                testPoint = segments[paramStart.index].getPoint(0.5 * (paramStart.t + paramLast.t));
                DEBUG_ERASER(debugstream << "| -------- getting point (" << paramStart.index << " ; "
                                         << 0.5 * (paramStart.t + paramLast.t) << ")" << std::endl;)
            } else {
                testPoint = segments[(paramStart.index + paramLast.index + 1) / 2].firstKnot;
                DEBUG_ERASER(debugstream << "| -------- getting point (" << (paramStart.index + paramLast.index + 1) / 2
                                         << " ; 0.0)" << std::endl;)
            }
            if (!testPoint.isInside(outerBox)) {
                DEBUG_ERASER(debugstream << "| --------- It is not in !!" << std::endl;)
                return true;
            }
            DEBUG_ERASER(debugstream << "| --------- It is in." << std::endl;)
        }
        return false;
    };

    inconsistentResults = inconsistentResults || checkSanity(result);

    if (inconsistentResults) {
        DEBUG_ERASER(debugstream << "| Inconsistent results!\n"; debugstream << "| * innerBox = (";
                     debugstream << std::setprecision(5); debugstream << std::setw(9) << innerBox.x << " ; ";
                     debugstream << std::setw(9) << innerBox.x + innerBox.width << ") -- (";
                     debugstream << std::setw(9) << innerBox.y << " ; ";
                     debugstream << std::setw(9) << innerBox.y + innerBox.height << ")\n";
                     debugstream << "| * outerBox = ("; debugstream << std::setw(9) << outerBox.x << " ; ";
                     debugstream << std::setw(9) << outerBox.x + outerBox.width << ") -- (";
                     debugstream << std::setw(9) << outerBox.y << " ; ";
                     debugstream << std::setw(9) << outerBox.y + outerBox.height << ")\n";
                     std::cout << debugstream.str();)
        return {};
    }

    return result;
}
