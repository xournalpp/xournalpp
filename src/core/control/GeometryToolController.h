/*
 * Xournal++
 *
 * A geometry tool controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "gui/PageView.h"
#include "model/GeometryTool.h"
#include "util/Point.h"

class Stroke;

/**
 * @brief A class that controls a geometry tool

 * The geometry tool can be moved, rotated and scaled
 * There are methods for translating coordinates
 * and methods to deal with the temporary stroke
 */

class GeometryToolController {
public:
    GeometryToolController(XojPageView* view, GeometryTool* geometryTool);
    virtual ~GeometryToolController();

public:
    /**
     * @brief translates the geometry tool in x- and y-direction
     * @param offset the translation vector (in document coordinates)
     */
    void translate(const xoj::util::Point<double>& offset);

    /**
     * @brief rotates the geometry tool around then given rotation center
     * @param da the rotation angle
     * @param center the rotation center (in document coordinates
     */
    void rotate(double da, const xoj::util::Point<double>& center);
    /// Rotates around the tool's origin
    void rotate(double da);

    /**
     * @brief resizes the geometry tool by the factor f with respect to a given scaling center
     * @param f the scaling factor
     * @param center the scaling center (in document coordinates)
     */
    void scale(double f, const xoj::util::Point<double>& center);
    /// Rescales around the tools origin
    void scale(double f);

    /**
     * @brief marks the origin with a "x" and puts the mark onto the current layer
     */
    void markOrigin();

    /**
     * @brief adds the stroke to the layer and rerenders the stroke area
     */
    void addStrokeToLayer();

    /**
     * @brief initializes the stroke by using the properties from the tool handler
     */
    void initializeStroke();

    /**
     * @brief the page view of the page with respect to which the geometry tool is initialized
     */
    XojPageView* getView() const;

    /**
     * @brief the page with respect to which the setsquare is initialized
     */
    const PageRef getPage() const;

    inline GeometryTool* getGeometryTool() const { return geometryTool; }

    virtual bool isInsideGeometryTool(double x, double y, double border = 0.0) const = 0;

    virtual GeometryToolType getType() const = 0;

protected:
    XojPageView* view;

    /**
     * @brief the underlying geometry tool
     */
    GeometryTool* geometryTool;

    /**
     * @brief The stroke drawn
     */
    std::unique_ptr<Stroke> stroke;
};
