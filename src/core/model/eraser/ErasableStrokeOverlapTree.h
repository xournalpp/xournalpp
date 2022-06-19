/*
 * Xournal++
 *
 * A stroke which is temporary used if you erase a part
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <utility>  // for pair
#include <vector>   // for vector, vector<>::iterator

#include <cairo.h>  // for cairo_t

#include "util/Rectangle.h"  // for Rectangle

#include "ErasableStroke.h"  // for ErasableStroke::SubSection, ErasableStroke
#include "config-debug.h"    // for DEBUG_ERASABLE_STROKE_BOXES

class Point;
class Range;
class Stroke;

class ErasableStroke::OverlapTree {
public:
    OverlapTree() = default;

    /**
     * @brief Populate the tree, so that it corresponds to the given section of the given stroke.
     */
    void populate(const SubSection& section, const Stroke& stroke);

    bool isPopulated() const;

#ifdef DEBUG_ERASABLE_STROKE_BOXES
    /**
     * @brief Add to a vector rectangles which altogether contain every overlap between the subsection corresponding
     * to this tree and the subsection corresponding to another tree.
     * @param other The other tree
     * @param halfWidth half the width of the stroke
     * @param overlapBoxes the Rectangle vector to which we push
     * @param cr A cairo context in which we paint the rectangles, for debug purposes
     */
    void addOverlapsToRange(const OverlapTree& other, double halfWidth, Range& range, cairo_t* cr = nullptr) const;
#else
    /**
     * @brief Add to a vector rectangles which altogether contain every overlap between the subsection corresponding
     * to this tree and the subsection corresponding to another tree.
     * @param other The other tree
     * @param halfWidth half the width of the stroke
     * @param overlapBoxes the Rectangle vector to which we push
     */
    void addOverlapsToRange(const OverlapTree& other, double halfWidth, Range& range) const;
#endif

private:
    /**
     * Root, node or leaf of a binary tree used for searching for overlaps between subsections
     */
    class Node {
    public:
        Node() = default;
#ifdef DEBUG_ERASABLE_STROKE_BOXES
        /**
         * @brief Add to a vector rectangles which altogether contain every overlap between the subsection corresponding
         * to this node and the subsection corresponding to another node.
         * @param other The other node
         * @param halfWidth half the width of the stroke
         * @param overlapBoxes the Rectangle vector to which we push
         * @param cr A cairo context in which we paint the rectangles, for debug purposes
         */
        void addOverlapsToRange(const Node& other, double halfWidth, Range& range, cairo_t* cr) const;
#else
        /**
         * @brief Add to a vector rectangles which altogether contain every overlap between the subsection corresponding
         * to this node and the subsection corresponding to another node.
         * @param other The other node
         * @param halfWidth half the width of the stroke
         * @param overlapBoxes the Rectangle vector to which we push
         */
        void addOverlapsToRange(const Node& other, double halfWidth, Range& range) const;
#endif

        /**
         * Bounding box of the subsection represented by the node.
         * WARNING: The bounding box does not take the stroke thickness into account
         */
        double minX;
        double maxX;
        double minY;
        double maxY;

        /**
         * Descendants, corresponding to the two halves of the subsection represented by the node
         */
        std::pair<Node, Node>* children = nullptr;

        /**
         * @brief Initialize the bounding box (minX, maxX, minY, maxY) when the node corresponds to a single segment
         * @param p1 The first endpoint of the segment
         * @param p2 The second endpoint of the segment
         */
        void initializeOnSegment(const Point& p1, const Point& p2);

        /**
         * @brief Compute the node's bounding box by taking the union of the children's boxes
         */
        void computeBoxFromChildren();

        /**
         * @brief Get a rectangle from the box (minX, maxX, minY, maxY), with an additional padding
         * @param padding Padding added to the box (typically, half the stroke's width)
         */
        xoj::util::Rectangle<double> toRectangle(double padding) const;
    };

    Node root;
    std::vector<std::pair<Node, Node>> data;
    bool populated = false;

    class Populator {
    public:
        Populator(std::vector<std::pair<Node, Node>>& data, const Stroke& stroke): data(data), stroke(stroke) {}
        void populate(const SubSection& section, Node& root);

    private:
        std::vector<std::pair<Node, Node>>& data;
        const Stroke& stroke;
        std::vector<std::pair<Node, Node>>::iterator nextFreeSlot;

        std::pair<Node, Node>* getNextFreeSlot();

        /**
         * @brief Create a root corresponding to the subsection:
         *      firstPoint -- pts[min] -- ... -- pts[max] -- lastPoint
         */
        void populateNode(Node& node, const Point& firstPoint, size_t min, size_t max, const Point& lastPoint,
                          const std::vector<Point>& pts);

        /**
         * @brief Create a subtree corresponding to the subsection:
         *      firstPoint -- pts[min] -- ... -- pts[max]
         */
        void populateNode(Node& node, const Point& firstPoint, size_t min, size_t max, const std::vector<Point>& pts);

        /**
         * @brief Create a subtree corresponding to the subsection:
         *      pts[min] -- ... -- pts[max] -- lastPoint
         */
        void populateNode(Node& node, size_t min, size_t max, const Point& lastPoint, const std::vector<Point>& pts);

        /**
         * @brief Create a subtree corresponding to the subsection:
         *      pts[min] -- ... -- pts[max]
         */
        void populateNode(Node& node, size_t min, size_t max, const std::vector<Point>& pts);
    };
};
