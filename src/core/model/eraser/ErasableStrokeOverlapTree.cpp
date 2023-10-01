#include "ErasableStrokeOverlapTree.h"

#include <algorithm>  // for max, min, minmax
#include <optional>   // for optional
#include <tuple>      // for tie, tuple

#include "model/Point.h"        // for Point
#include "model/Stroke.h"       // for Stroke
#include "model/path/Path.h"    // for Parameter, SubSection
#include "model/path/Spline.h"  // for Spline
#include "util/Assert.h"        // for xoj_assert
#include "util/Range.h"         // for Range

using xoj::util::Rectangle;

void ErasableStroke::OverlapTree::populate(const Path::SubSection& section, const Stroke& stroke) {
    Populator populator(this->data, stroke);
    populator.populate(section, this->root);
    populated = true;
}

bool ErasableStroke::OverlapTree::isPopulated() const { return populated; }

#ifdef DEBUG_ERASABLE_STROKE_BOXES
void ErasableStroke::OverlapTree::addOverlapsToRange(const ErasableStroke::OverlapTree& other, double halfWidth,
                                                     Range& range, cairo_t* cr) const {
#else
void ErasableStroke::OverlapTree::addOverlapsToRange(const ErasableStroke::OverlapTree& other, double halfWidth,
                                                     Range& range) const {
#endif
    xoj_assert(this->isPopulated() && other.isPopulated());

#ifdef DEBUG_ERASABLE_STROKE_BOXES
    this->root.addOverlapsToRange(other.root, halfWidth, range, cr);
#else
    this->root.addOverlapsToRange(other.root, halfWidth, range);
#endif
}

void ErasableStroke::OverlapTree::Populator::populate(const Path::SubSection& section, Node& root) {
    xoj_assert(stroke.path != nullptr);

    const Path& path = stroke.getPath();
    xoj_assert(section.min.index <= section.max.index && section.max.index < path.nbSegments());

    size_t nbLeaves = section.max.index - section.min.index + 1;
    this->data.resize(nbLeaves - 1);

    this->nextFreeSlot = this->data.begin();

    const double halfWidth = 0.5 * stroke.getWidth();

    auto pathType = path.getType();
    xoj_assert(pathType == Path::PIECEWISE_LINEAR || pathType == Path::SPLINE);

    if (section.min.index == section.max.index) {
        // The section spans on a single segment
        if (pathType == Path::PIECEWISE_LINEAR) {
            root.initializeOnSegment(path.getPoint(section.min), path.getPoint(section.max), halfWidth);
            return;
        }
        const Spline& spline = dynamic_cast<const Spline&>(path);
        root.initializeOnSegment(spline.getSegment(section.min.index).getSubsegment(section.min.t, section.max.t),
                                 halfWidth);
        return;
    }

    if (section.max.t == 0.0) {
        // Highly unlikely...
        if (section.min.index + 1 == section.max.index) {
            // The section spans on a single segment
            if (pathType == Path::PIECEWISE_LINEAR) {
                root.initializeOnSegment(path.getPoint(section.min), path.getPoint(section.max), halfWidth);
                return;
            }
            const Spline& spline = dynamic_cast<const Spline&>(path);
            root.initializeOnSegment(spline.getSegment(section.min.index).subdivide(section.min.t).second, halfWidth);
            return;
        }
        if (section.min.t == 0.0) {
            if (pathType == Path::PIECEWISE_LINEAR) {
                this->populateNode(root, section.min.index, section.max.index, path.getData());
                return;
            }
            const Spline& spline = dynamic_cast<const Spline&>(path);
            this->populateNode(root, section.min.index, section.max.index, spline.segments());
            return;
        }
        if (pathType == Path::PIECEWISE_LINEAR) {
            this->populateNode(root, path.getPoint(section.min), section.min.index + 1, section.max.index,
                               path.getData());
            return;
        }
        const Spline& spline = dynamic_cast<const Spline&>(path);
        this->populateNode(root, section.min, section.max.index, spline.segments());
        return;
    }

    if (section.min.t == 0.0) {
        // This happens when erasing the end of a stroke
        if (pathType == Path::PIECEWISE_LINEAR) {
            this->populateNode(root, section.min.index, section.max.index, path.getPoint(section.max), path.getData());
            return;
        }
        const Spline& spline = dynamic_cast<const Spline&>(path);
        this->populateNode(root, section.min.index, section.max, spline.segments());
        return;
    }

    if (pathType == Path::PIECEWISE_LINEAR) {
        this->populateNode(root, path.getPoint(section.min), section.min.index + 1, section.max.index,
                           path.getPoint(section.max), path.getData());
        return;
    }
    const Spline& spline = dynamic_cast<const Spline&>(path);
    this->populateNode(root, section, spline.segments());
}

auto ErasableStroke::OverlapTree::Populator::getNextFreeSlot() -> std::pair<Node, Node>* {
    xoj_assert(nextFreeSlot < data.end());
    return &*(nextFreeSlot++);
}

/**
 * Nodes for PiecewiseLinearPath
 */
void ErasableStroke::OverlapTree::Populator::populateNode(Node& node, const Point& firstPoint, size_t min, size_t max,
                                                          const Point& lastPoint, const std::vector<Point>& pts) {
    xoj_assert(min <= max && max < pts.size());
    /**
     * Split in two in the middle
     */
    size_t middle = (min + max) / 2;

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, firstPoint, min, middle, pts);
    this->populateNode(node.children->second, middle, max, lastPoint, pts);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(Node& node, const Point& firstPoint, size_t min, size_t max,
                                                          const std::vector<Point>& pts) {
    xoj_assert(min <= max && max < pts.size());
    if (min == max) {
        // The node corresponds to a single segment
        node.initializeOnSegment(firstPoint, pts[min], 0.5 * stroke.getWidth());
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (min + max) / 2;
    xoj_assert(middle >= min && middle < max);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, firstPoint, min, middle, pts);
    this->populateNode(node.children->second, middle, max, pts);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(Node& node, size_t min, size_t max, const Point& lastPoint,
                                                          const std::vector<Point>& pts) {
    xoj_assert(min <= max && max < pts.size());
    if (min == max) {
        // The node corresponds to a single segment
        node.initializeOnSegment(pts[min], lastPoint, 0.5 * stroke.getWidth());
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (min + max + 1) / 2;
    xoj_assert(middle > min && middle <= max);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, min, middle, pts);
    this->populateNode(node.children->second, middle, max, lastPoint, pts);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(Node& node, size_t min, size_t max,
                                                          const std::vector<Point>& pts) {
    xoj_assert(max > min);
    if (min + 1 == max) {
        // The node corresponds to a single segment
        node.initializeOnSegment(pts[min], pts[max], 0.5 * stroke.getWidth());
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (min + max) / 2;
    xoj_assert(middle > min && middle < max);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, min, middle, pts);
    this->populateNode(node.children->second, middle, max, pts);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Node::initializeOnSegment(const Point& p1, const Point& p2, const double halfWidth) {
    this->box = Range(p1.x, p1.y);
    this->box.addPoint(p2.x, p2.y);
    this->box.addPadding(halfWidth);
}

/**
 * Nodes for splines
 */
void ErasableStroke::OverlapTree::Populator::populateNode(
        Node& node, const Path::SubSection& section, const Path::SegmentIteratable<const SplineSegment>& segments) {
    xoj_assert(section.min.index < section.max.index && section.max.index < segments.size());
    /**
     * Split in two in the middle
     */
    size_t middle = (section.min.index + section.max.index + 1) / 2;

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, section.min, middle, segments);
    this->populateNode(node.children->second, middle, section.max, segments);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(
        Node& node, const Path::Parameter& startParam, size_t endIndex,
        const Path::SegmentIteratable<const SplineSegment>& segments) {
    xoj_assert(startParam.index <= endIndex && endIndex < segments.size());
    if (startParam.index + 1 == endIndex) {
        // The node corresponds to a single segment
        node.initializeOnSegment(segments[endIndex].subdivide(startParam.t).second, 0.5 * stroke.getWidth());
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (startParam.index + endIndex) / 2;
    xoj_assert(middle >= startParam.index && middle < endIndex);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, startParam, middle, segments);
    this->populateNode(node.children->second, middle, endIndex, segments);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(
        Node& node, size_t startIndex, const Path::Parameter& endParam,
        const Path::SegmentIteratable<const SplineSegment>& segments) {
    xoj_assert(startIndex <= endParam.index && endParam.index < segments.size());
    if (startIndex == endParam.index) {
        // The node corresponds to a single segment
        node.initializeOnSegment(segments[startIndex].subdivide(endParam.t).first, 0.5 * stroke.getWidth());
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (startIndex + endParam.index + 1) / 2;
    xoj_assert(middle > startIndex && middle <= endParam.index);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, startIndex, middle, segments);
    this->populateNode(node.children->second, middle, endParam, segments);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Populator::populateNode(
        Node& node, size_t startIndex, size_t endIndex, const Path::SegmentIteratable<const SplineSegment>& segments) {
    xoj_assert(endIndex > startIndex);
    if (startIndex + 1 == endIndex) {
        // The node corresponds to a single segment
        node.initializeOnSegment(segments[startIndex], 0.5 * stroke.getWidth());
        return;
    }
    /**
     * Split in two
     */
    size_t middle = (startIndex + endIndex) / 2;
    xoj_assert(middle > startIndex && middle < endIndex);

    node.children = getNextFreeSlot();

    this->populateNode(node.children->first, startIndex, middle, segments);
    this->populateNode(node.children->second, middle, endIndex, segments);
    node.computeBoxFromChildren();
}

void ErasableStroke::OverlapTree::Node::initializeOnSegment(const SplineSegment& segment, const double halfWidth) {
    this->box = segment.getThinBoundingBox();
    this->box.addPadding(halfWidth);
}

void ErasableStroke::OverlapTree::Node::computeBoxFromChildren() {
    xoj_assert(children != nullptr);
    box = children->first.box.unite(children->second.box);
}

#ifdef DEBUG_ERASABLE_STROKE_BOXES
void ErasableStroke::OverlapTree::Node::addOverlapsToRange(const Node& other, double halfWidth, Range& range,
                                                           cairo_t* cr) const {
#else
void ErasableStroke::OverlapTree::Node::addOverlapsToRange(const Node& other, double halfWidth, Range& range) const {
#endif
    if (this->box.intersect(other.box).empty()) {
        return;
    }
    if (other.children != nullptr) {
#ifdef DEBUG_ERASABLE_STROKE_BOXES
        other.children->first.addOverlapsToRange(*this, halfWidth, range, cr);
        other.children->second.addOverlapsToRange(*this, halfWidth, range, cr);
#else
        other.children->first.addOverlapsToRange(*this, halfWidth, range);
        other.children->second.addOverlapsToRange(*this, halfWidth, range);
#endif
        return;
    }
    /**
     * other is a leaf corresponding to a single segment of the stroke
     * Split *this until it's just a leaf
     */
    if (this->children != nullptr) {
#ifdef DEBUG_ERASABLE_STROKE_BOXES
        this->children->first.addOverlapsToRange(other, halfWidth, range, cr);
        this->children->second.addOverlapsToRange(other, halfWidth, range, cr);
#else
        this->children->first.addOverlapsToRange(other, halfWidth, range);
        this->children->second.addOverlapsToRange(other, halfWidth, range);
#endif
        return;
    }
    /**
     * Both this and other are leaves corresponding to single segments
     * Repaint the intersection of their bounding boxes
     */
    range = range.unite(this->box.intersect(other.box));

#ifdef DEBUG_ERASABLE_STROKE_BOXES
    paintDebugRect(rectThis, 'g', cr);
    paintDebugRect(rectOther, 'g', cr);
    paintDebugRect(rect, 'b', cr);
#endif
}
