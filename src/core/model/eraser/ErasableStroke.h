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

#include <mutex>
#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "model/Point.h"


class ErasableStrokePart;
class PartList;
class Range;
class Stroke;

class ErasableStroke {
public:
    ErasableStroke(Stroke* stroke);
    virtual ~ErasableStroke();

public:
    /**
     * Returns a repaint rectangle or nullptr, the rectangle is own by the caller
     */
    Range* erase(double x, double y, double halfEraserSize, Range* range = nullptr);

    GList* getStroke(Stroke* original);

    void draw(cairo_t* cr);

private:
    void erase(double x, double y, double halfEraserSize, ErasableStrokePart* part, PartList* list);
    static bool erasePart(double x, double y, double halfEraserSize, ErasableStrokePart* part, PartList* list,
                          bool* deleteStrokeAfter);

    void addRepaintRect(double x, double y, double width, double height);

private:
    std::mutex partLock;
    PartList* parts = nullptr;

    Range* repaintRect = nullptr;

    Stroke* stroke = nullptr;
};
