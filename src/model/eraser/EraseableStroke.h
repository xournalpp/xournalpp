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
#include <mutex>
#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "model/Point.h"
#include "model/Spline.h"

#include "UnionOfIntervals.h"
#include "XournalType.h"

class Range;
class Stroke;

class EraseableStroke {
public:
    EraseableStroke(Stroke* stroke);
    virtual ~EraseableStroke() = default;

public:
    /**
     * Returns a repaint rectangle or nullptr, the rectangle is own by the caller
     */
    virtual Range* beginErasure(double x, double y, double halfEraserSize, Range* range = nullptr);
    virtual Range* erase(double x, double y, double halfEraserSize, Range* range = nullptr);

    //     GList* getStroke(Stroke* original);
    std::vector<Stroke*> getStrokes();

    virtual void draw(cairo_t* cr);

protected:
    //     void erase(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list);
    //     static bool erasePart(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list,
    //                           bool* deleteStrokeAfter);

    void addRepaintRect(double x, double y, double width, double height);

protected:
    Range* repaintRect = nullptr;

    Stroke* stroke = nullptr;

    UnionOfIntervals<Spline::Parameter> remainingSections{};
    std::mutex sectionsMutex;
};
/*
class EraseableStrokeWithPressure: public EraseableStroke {
    EraseableStrokeWithPressure(Stroke* stroke);
    virtual ~EraseableStrokeWithPressure() = default;

private:
    class ParametrizedPoint {
    public:
        Point point;
        double t;
    };
    std::unordered_map<size_t, std::vector<ParametrizedPoint>> points;
};
*/
