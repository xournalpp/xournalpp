/*
 * Xournal++
 *
 * Displays a Image Element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "View.h"

class Image;

class xoj::view::ImageView: public xoj::view::ElementView {
public:
    ImageView(const Image* image);
    virtual ~ImageView();

    /**
     * Draws an Image element
     */
    void draw(const Context& ctx) const override;

    /**
     * Draws only a part of the Image element, rest is affected by alpha
     * @param cr   cairo to draw with
     * @param xIgnoreP  ignore until this Percentage of Image is reached in x direction
     * @param yIgnoreP  ignore until this Percentage of Image is reached in y direction
     * @param xDrawP    draw until this Percentage of Image is reached in x direction, then ignore again
     * @param yDrawP    draw until this Percentage of Image is reached in y direction, then ignore again
     * @param alphaForIgnore    what opacity the ignored parts of the image should be
     */
    void drawPartial(cairo_t* cr, double xIgnoreP, double yIgnoreP, double xDrawP, double yDrawP,
                     double alphaForIgnore);

private:
    const Image* image;
};
