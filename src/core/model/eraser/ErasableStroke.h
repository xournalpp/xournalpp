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

#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include <gtk/gtk.h>

#include "ErasableStrokePart.h"

using PartList = std::list<ErasableStrokePart>;

class Range;
class Stroke;

class ErasableStroke {
public:
    ErasableStroke(Stroke* stroke);

public:
    /**
     * Returns a repaint rectangle or nullptr, the rectangle is own by the caller
     */
    Range* erase(double x, double y, double halfEraserSize, Range* range = nullptr);

    std::vector<std::unique_ptr<Stroke>> getStroke(Stroke* original);

    void draw(cairo_t* cr);

private:
    bool erase(double x, double y, double halfEraserSize, PartList::iterator& part, PartList& list);
    static bool erasePart(double x, double y, double halfEraserSize, PartList::iterator& part, PartList& list,
                          bool* deleteStrokeAfter);

    void addRepaintRect(double x, double y, double width, double height);

private:
    std::mutex partLock;
    PartList parts{};

    Range* repaintRect = nullptr;

    Stroke* stroke = nullptr;
};
