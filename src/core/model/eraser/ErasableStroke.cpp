#include "ErasableStroke.h"

#include <algorithm>  // for max, min, copy, lower_bound
#include <cassert>    // for assert
#include <cstddef>    // for size_t, ptrdiff_t
#include <iterator>   // for next
#include <optional>   // for optional
#include <tuple>      // for forward_as_tuple

#include <glib.h>  // for g_warning

#include "model/Point.h"            // for Point
#include "model/Stroke.h"           // for Stroke, IntersectionParameter...
#include "util/Range.h"             // for Range
#include "util/SmallVector.h"       // for SmallVector
#include "util/UnionOfIntervals.h"  // for UnionOfIntervals

#include "ErasableStrokeOverlapTree.h"  // for ErasableStroke::OverlapTree
#include "PaddedBox.h"                  // for PaddedBox

using xoj::util::Rectangle;

ErasableStroke::ErasableStroke(const Stroke& stroke): stroke(stroke) {
    const auto& pts = this->stroke.getPointVector();
    closedStroke = pts.size() >= 3 && pts.front().lineLengthTo(pts.back()) < CLOSED_STROKE_DISTANCE;
}

#ifdef DEBUG_ERASABLE_STROKE_BOXES
ErasableStroke::~ErasableStroke() {
    if (this->surfDebug) {
        cairo_surface_destroy(this->surfDebug);
        this->surfDebug = nullptr;
    }
    if (this->crDebug) {
        cairo_destroy(this->crDebug);
        this->crDebug = nullptr;
    }
}
#else
ErasableStroke::~ErasableStroke() = default;
#endif

/**
 * Erasure works as follows:
 *  * Two squares: * the visible eraser square
 *                 * the same square with a padding depending on the stroke's width (see #997)
 *
 *  We erase every section of the stroke that comes in the box with its padding, provided it hits the box itself at
 *  some point.
 *  This means we erase a little outside the visible box, but only when the stroke enters or leaves the visible box.
 */
void ErasableStroke::beginErasure(const IntersectionParametersContainer& paddedIntersections, Range& range) {
    size_t n = this->stroke.getPointCount();
    if (n < 2) {
        return;
    }

    assert(paddedIntersections.size() % 2 == 0);

    UnionOfIntervals<PathParameter> erasedSections;
    erasedSections.appendData(paddedIntersections);

    // Now remaining sections
    erasedSections.complement({0, 0.0}, {n - 2, 1.0});

    const bool highlighter = this->stroke.getToolType() == StrokeTool::HIGHLIGHTER;
    const bool filled = this->stroke.getFill() != -1;
    if (highlighter || filled) {
        auto subsections = erasedSections.cloneToIntervalVector();
        if (filled) {
            if (subsections.size() == 1) {
                // We erased the stroke from its ends
                const auto& subsection = subsections.back();
                const Point& p1 = this->stroke.getPointVector().front();
                range.addPoint(p1.x, p1.y);
                const Point& p2 = this->stroke.getPointVector().back();
                range.addPoint(p2.x, p2.y);
                Point p = this->stroke.getPoint(subsection.min);
                range.addPoint(p.x, p.y);
                p = this->stroke.getPoint(subsection.max);
                range.addPoint(p.x, p.y);
            } else {
                // The stroke was split in two or more (and possibly shrank). Need to rerender its entire box.
                range.addPoint(this->stroke.getX(), this->stroke.getY());
                range.addPoint(this->stroke.getX() + this->stroke.getElementWidth(),
                               this->stroke.getY() + this->stroke.getElementHeight());
            }
        } else if (subsections.size() > 1) {
            /**
             * Highlighter and the stroke has been split in two or more subsections.
             * Rerender wherever those subsections overlap
             */
            addOverlapsToRange(subsections, range);
        }
    }

    {  // lock_guard range
        std::lock_guard<std::mutex> lock(this->sectionsMutex);
        this->remainingSections.swap(erasedSections);
    }  // release the mutex
}

void ErasableStroke::erase(const PaddedBox& box, Range& range) {
    size_t n = (size_t)this->stroke.getPointCount();
    if (n < 2) {
        g_warning("Erasing empty stroke");
        return;
    }

    std::vector<SubSection> sections = this->getRemainingSubSectionsVector();

    if (sections.empty()) {
        /** Nothing left to erase! **/
        std::lock_guard<std::mutex> lock(this->boxesMutex);
        boundingBoxes.clear();
        return;
    }

    if (changesAtLastIteration) {
        // Remove the boxes of the cache corresponding to sections that no longer exist
        std::lock_guard<std::mutex> lock(this->boxesMutex);
        auto it = boundingBoxes.begin();
        for (auto itSection = sections.cbegin(), itSectionEnd = sections.cend();
             it != boundingBoxes.end() && itSection != itSectionEnd;) {
            if (*itSection < it->first) {
                ++itSection;
            } else if (*itSection > it->first) {
                it = boundingBoxes.erase(it);
            } else {
                ++it;
            }
        }
        boundingBoxes.erase(it, boundingBoxes.end());
    }

    /**
     * Determine which (intervals of) segments are still (partially) visible and have their bounding box intersecting
     * the eraser square
     *
     * This avoids computing a segment's intersections with the eraser box twice
     */
    std::vector<Interval<size_t>> indexIntervals;

    for (const SubSection& section: sections) {
        if (getSubSectionBoundingBox(section).intersects(box.getInnerRectangle())) {
            if (indexIntervals.empty()) {
                indexIntervals.emplace_back(section.min.index, section.max.index);
            } else {
                if (indexIntervals.back().max + 1 >= section.min.index) {
                    // Merge together two sections if the first ends in the segment in which the second begins
                    indexIntervals.back().envelop(section.max.index);
                } else {
                    indexIntervals.emplace_back(section.min.index, section.max.index);
                }
            }
        }
    }

    UnionOfIntervals<PathParameter> newErasedSections;

    for (auto& i: indexIntervals) {
        newErasedSections.appendData(this->stroke.intersectWithPaddedBox(box, i.min, i.max));
    }

    changesAtLastIteration = !newErasedSections.empty();
    if (changesAtLastIteration) {
        const bool highlighter = this->stroke.getToolType() == StrokeTool::HIGHLIGHTER;
        const bool filled = this->stroke.getFill() != -1;
        if (highlighter || filled) {
            /**
             * Detect if a section has been modified. If so, rerender whatever needs rerendering.
             */

            std::vector<SubSection> newRemainingSections;
            // Update the remaining sections
            newErasedSections.complement({0, 0.0}, {n - 2, 1.0});
            {  // lock_guard scope
                std::lock_guard<std::mutex> lock(sectionsMutex);
                remainingSections.intersect(newErasedSections.getData());
                newRemainingSections = remainingSections.cloneToIntervalVector();
            }  // Release the mutex

            auto itNewSection = newRemainingSections.begin();
            auto itNewSectionEnd = newRemainingSections.end();
            for (auto& section: sections) {
                // Find out which new sections belonged to the same section before
                std::vector<SubSection> subsections;
                while (itNewSection != itNewSectionEnd && itNewSection->max <= section.max) {
                    subsections.emplace_back(*itNewSection);
                    ++itNewSection;
                }
                if (subsections.empty()) {
                    // The section was entirely erased. No need for special rerendering.
                    continue;
                }
                if (filled) {
                    if (subsections.size() == 1) {
                        const auto& subsection = subsections.back();
                        if (subsection.min == section.min && subsection.max == section.max) {
                            // No modification
                            continue;
                        }
                        // The section shrank.
                        Point p = this->stroke.getPoint(section.min);
                        range.addPoint(p.x, p.y);
                        p = this->stroke.getPoint(section.max);
                        range.addPoint(p.x, p.y);
                        p = this->stroke.getPoint(subsection.min);
                        range.addPoint(p.x, p.y);
                        p = this->stroke.getPoint(subsection.max);
                        range.addPoint(p.x, p.y);
                        continue;
                    }
                    // The section was split in two or more (and possibly shrank). Need to rerender its entire box.
                    auto rect = this->getSubSectionBoundingBox(section);
                    range.addPoint(rect.x, rect.y);
                    range.addPoint(rect.x + rect.width, rect.y + rect.height);
                    break;
                }
                // Necessarily highlighter and not filled
                if (subsections.size() > 1) {
                    /**
                     * The section has been split in two (or more).
                     * Rerender wherever those subsections overlap.
                     */
                    addOverlapsToRange(subsections, range);
                }
            }
        } else {
            // Update the remaining sections
            newErasedSections.complement({0, 0.0}, {n - 2, 1.0});
            {  // lock_guard scope
                std::lock_guard<std::mutex> lock(sectionsMutex);
                remainingSections.intersect(newErasedSections.getData());
            }  // Release the mutex
        }
        box.addToRange(range);
    }
}

auto ErasableStroke::getStrokes() const -> std::vector<std::unique_ptr<Stroke>> {
    std::vector<SubSection> sections = this->getRemainingSubSectionsVector();

    std::vector<std::unique_ptr<Stroke>> strokes;
    strokes.reserve(sections.size());

    bool mergeFirstAndLast = this->closedStroke && sections.size() >= 2 &&
                             sections.front().min == PathParameter(0, 0.0) &&
                             sections.back().max == PathParameter(this->stroke.getPointCount() - 2, 1.0);

    auto sectionIt = sections.cbegin();
    auto sectionEndIt = sections.cend();

    if (mergeFirstAndLast) {
        /**
         * Clone the first and last sections as a single stroke
         */
        strokes.push_back(this->stroke.cloneCircularSectionOfClosedStroke(sections.back().min, sections.front().max));

        // Avoid cloning those sections again
        ++sectionIt;
        --sectionEndIt;
    }

    for (; sectionIt != sectionEndIt; ++sectionIt) {
        strokes.push_back(this->stroke.cloneSection(sectionIt->min, sectionIt->max));
    }

    return strokes;
}

std::vector<ErasableStroke::SubSection> ErasableStroke::getRemainingSubSectionsVector() const {
    std::lock_guard<std::mutex> lock(this->sectionsMutex);
    return this->remainingSections.cloneToIntervalVector();
}

bool ErasableStroke::isClosedStroke() const { return this->closedStroke; }

Rectangle<double> ErasableStroke::getSubSectionBoundingBox(const ErasableStroke::SubSection& section) const {

    std::lock_guard<std::mutex> lock(this->boxesMutex);

    //  First look for the box in the cache
    auto it = std::lower_bound(boundingBoxes.begin(), boundingBoxes.end(), section,
                               [](const std::pair<SubSection, Rectangle<double>>& cacheData,
                                  const SubSection& section) { return cacheData.first < section; });
    if (it != boundingBoxes.end() && section == it->first) {
        // There was already a box computed for this section
        return it->second;
    }

    // Need to compute the bounding box
    Point p = this->stroke.getPoint(section.min);
    double minX = p.x;
    double maxX = p.x;
    double minY = p.y;
    double maxY = p.y;

    auto data = this->stroke.getPointVector();
    auto endIt = std::next(data.cbegin(), (std::ptrdiff_t)section.max.index + 1);
    for (auto ptIt = std::next(data.cbegin(), (std::ptrdiff_t)section.min.index + 1); ptIt != endIt; ++ptIt) {
        minX = std::min(minX, ptIt->x);
        maxX = std::max(maxX, ptIt->x);
        minY = std::min(minY, ptIt->y);
        maxY = std::max(maxY, ptIt->y);
    }

    Point q = this->stroke.getPoint(section.max);
    minX = std::min(minX, q.x);
    maxX = std::max(maxX, q.x);
    minY = std::min(minY, q.y);
    maxY = std::max(maxY, q.y);

    /**
     * Add the stroke width
     * This is not quite accurate for stroke with pressure values
     */
    const double strokeWidth = this->stroke.getWidth();
    const double width = maxX - minX + strokeWidth;
    const double height = maxY - minY + strokeWidth;
    minX -= 0.5 * strokeWidth;
    minY -= 0.5 * strokeWidth;

    // Assign the computed rectangle to the cache
    it = boundingBoxes.emplace(it, std::piecewise_construct, std::forward_as_tuple(section),
                               std::forward_as_tuple(minX, minY, width, height));

    return it->second;
}

void ErasableStroke::addOverlapsToRange(const std::vector<SubSection>& subsections, Range& range) {

    // Will contain the intersection trees of the subsections
    std::vector<OverlapTree> overlapTrees(subsections.size());
    /**
     * For each given subsection, we compute a binary tree whose leaves correspond to individual segments in the
     * subsection and contain the thin bounding box of the segments.
     * The nodes contain the union of the bounding boxes of their children, so that the root itself contains the
     * bounding box of the subsection.
     *
     * To compute the overlaps between two subsections, we intersect the bounding boxes in their trees, until we reach
     * intersecting leaves.
     * See ErasableStroke::OverlapTree for the details.
     */

    const double halfWidth = 0.5 * this->stroke.getWidth();
    size_t i = 0;
    size_t j = 0;
    for (auto it1 = subsections.cbegin(), itEnd = subsections.cend(); it1 != itEnd; ++it1, ++i) {
        j = i + 1;
        for (auto it2 = std::next(it1); it2 != itEnd; ++it2, ++j) {
            if (getSubSectionBoundingBox(*it1).intersects(getSubSectionBoundingBox(*it2))) {
                // Compute the intersections trees if they have not yet been computed
                if (!overlapTrees[i].isPopulated()) {
                    overlapTrees[i].populate(*it1, this->stroke);
                }
                if (!overlapTrees[j].isPopulated()) {
                    overlapTrees[j].populate(*it2, this->stroke);
                }
#ifdef DEBUG_ERASABLE_STROKE_BOXES
                overlapTrees[i].addOverlapsToRange(overlapTrees[j], halfWidth, range, crDebug);
#else
                overlapTrees[i].addOverlapsToRange(overlapTrees[j], halfWidth, range);
#endif
            }
        }
    }
}

#ifdef DEBUG_ERASABLE_STROKE_BOXES
void ErasableStroke::paintDebugRect(const Rectangle<double>& rect, char color, cairo_t* cr) {
    if (cr == nullptr) {
        return;
    }
    if (color == 'r') {
        cairo_set_source_rgba(cr, 1, 0, 0, 0.8);
    } else if (color == 'g') {
        cairo_set_source_rgba(cr, 0, 1, 0, 0.8);
    } else if (color == 'b') {
        cairo_set_source_rgba(cr, 0, 0, 1, 0.8);
    } else {
        cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 0.8);
    }
    cairo_move_to(cr, rect.x, rect.y);
    cairo_line_to(cr, rect.x + rect.width, rect.y);
    cairo_line_to(cr, rect.x + rect.width, rect.y + rect.height);
    cairo_line_to(cr, rect.x, rect.y + rect.height);
    cairo_line_to(cr, rect.x, rect.y);
    cairo_stroke(cr);
}
#endif
