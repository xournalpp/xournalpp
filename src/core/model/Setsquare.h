/*
 * Xournal++
 *
 * A setsquare model
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
 * @brief A class that models a setsquare
 *
 * The setsquare has the shape of a right-angled isosceles triangle and has a certain height, may be rotated and
 * translated. User coordinates are specified in cm.
 */


namespace xoj::util {
template <class T>
class DispatchPool;
}

namespace xoj::view {
class SetsquareView;
};

class SetsquareInputHandler;

namespace xoj::view {
class SetsquareView;
};


class Setsquare: public OverlayBase {
public:
    Setsquare();

    /**
     * @brief A setsquare specified by its height, rotation angle and translations in x- and y-directions
     * @param height the height of the setsquare
     * @param rotation the angle (in radian) around which the setsquare is rotated with respect to the x-axis
     * @param x the x-coordinate (in pt) of the mid point of the longest side of the setsquare
     * @param y the y-coordinate (in pt) of the mid point of the longest side of the setsquare
     */
    Setsquare(double height, double rotation, double x, double y);

    ~Setsquare();

    void notify() const;  // calls the update method of all observers

    // parameters used when initially displaying setsquare on a page
    static constexpr double INITIAL_HEIGHT = 8.0;
    static constexpr double INITIAL_X = 21. * HALF_CM;
    static constexpr double INITIAL_Y = 15. * HALF_CM;

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

    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SetsquareView>>& getViewPool() const;
    const std::shared_ptr<xoj::util::DispatchPool<SetsquareInputHandler>>& getHandlerPool() const;

private:
    /**
     * @brief the height of the setsquare
     */
    double height;

    /**
     * @brief the angle (in radian) around which the setsquare is rotated with respect to the x-axis
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

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::SetsquareView>> viewPool;
    std::shared_ptr<xoj::util::DispatchPool<SetsquareInputHandler>> handlerPool;

    /**
     * @brief Bounding box of the setsquare and stroke after its last update
     */
    mutable Range lastRepaintRange;
};
