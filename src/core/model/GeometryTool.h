/*
 * Xournal++
 *
 * A geometry tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_matrix_t

#include "model/OverlayBase.h"
#include "model/Stroke.h"
#include "util/DispatchPool.h"

constexpr static double HALF_CM = 14.17;
constexpr static double CM = 2. * HALF_CM;

/**
 * @brief A class that models a geometry tool
 *
 * The geometry tool has a certain height/size, may be rotated and
 * translated. User coordinates are specified in cm.
 */

namespace xoj::util {
template <class T>
class DispatchPool;
}

namespace xoj::view {
class GeometryToolView;
};

class GeometryToolHandler;

class GeometryTool: public OverlayBase {
public:
    /**
     * @brief A geometry tool specified by its height, rotation angle and translations in x- and y-directions
     * @param height the height (size parameter) of the geometry tool
     * @param rotation the angle (in radian) around which the geometry tool is rotated with respect to the x-axis
     * @param x the x-coordinate (in pt) of the rotation center
     * @param y the y-coordinate (in pt) of the rotation center
     */
    GeometryTool(double height, double rotation, double x, double y);

    virtual ~GeometryTool();

    void setHeight(double height);
    double getHeight() const;
    void setRotation(double rotation);
    double getRotation() const;
    void setTranslationX(double x);
    double getTranslationX() const;
    void setTranslationY(double y);
    double getTranslationY() const;

    void getMatrix(cairo_matrix_t& matrix) const;

    Stroke* getStroke() const;
    void setStroke(Stroke* s);

    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::GeometryToolView>>& getViewPool() const;
    const std::shared_ptr<xoj::util::DispatchPool<GeometryToolHandler>>& getHandlerPool() const;

    virtual void notify() const = 0;  // calls the update method of all observers

protected:
    /**
     * @brief the height of the geometry tool
     */
    double height;

    /**
     * @brief the angle (in radian) around which the geometry tool is rotated with respect to the x-axis
     */
    double rotation;

    /**
     * @brief the x-coordinate (in pt) of the rotation center
     */
    double translationX;

    /**
     * @brief the y-coordinate (in pt) of the rotation center
     */
    double translationY;

    Stroke* stroke = nullptr;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::GeometryToolView>> viewPool;
    std::shared_ptr<xoj::util::DispatchPool<GeometryToolHandler>> handlerPool;
};
