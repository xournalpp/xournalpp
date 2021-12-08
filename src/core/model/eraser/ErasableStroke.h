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

#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include <gtk/gtk.h>

#include "model/Point.h"
#include "util/Range.h"

struct ErasableStrokeEdge {
    ErasableStrokeEdge() = default;
    ErasableStrokeEdge(Point p1, Point p2): p1(p1), p2(p2){};
    Point p1;
    Point p2;
};

using PartList = std::vector<ErasableStrokeEdge>;

class Range;
class Stroke;

class ErasableStroke {
public:
    ErasableStroke(Stroke* stroke);

public:
    /**
     * Returns a repaint rectangle or nullptr, the rectangle is own by the caller
     */
    [[nodiscard]] Range erase(double x, double y, double halfEraserSize, Range range = Range());

    [[nodiscard]] std::vector<std::unique_ptr<Stroke>> getStroke(Stroke* original);

    void draw(cairo_t* cr);

private:
    [[nodiscard]] auto erase(double x, double y, double halfEraserSize, PartList::iterator const& partIter,
                             PartList& list) -> PartList::iterator;

    void addRepaintRect(double x, double y, double width, double height);

private:
    std::mutex partLock;
    PartList parts{};
    std::optional<Range> repaintRect{};

    Stroke* stroke = nullptr;
};
