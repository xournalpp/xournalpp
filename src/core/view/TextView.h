/*
 * Xournal++
 *
 * Displays a text Element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include <pango/pangocairo.h>  // for PangoLayout, cairo_t

#include "View.h"  // for ElementView

class Text;
class XojPdfRectangle;

class xoj::view::TextView: public xoj::view::ElementView {
public:
    TextView(const Text* t);
    virtual ~TextView();

    /**
     * Draws a Text model to a cairo surface
     */
    void draw(const Context& ctx) const override;

    /**
     * Calculates the size of a Text model
     */
    static void calcSize(const Text* t, double& width, double& height);

    /**
     * Searches text within a Text model
     */
    static std::vector<XojPdfRectangle> findText(const Text* t, const std::string& search);

    /**
     * Initialize a Pango layout
     */
    static PangoLayout* initPango(cairo_t* cr, const Text* t);

    /**
     * Sets the font name from Text model
     */
    static void updatePangoFont(PangoLayout* layout, const Text* t);

private:
    const Text* text;
};
