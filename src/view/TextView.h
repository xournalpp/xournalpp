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

#include <gtk/gtk.h>

#include "pdf/base/XojPdfPage.h"

class Text;

class TextView {
private:
    TextView();
    virtual ~TextView();

public:
    static void setDpi(int dpi);

    /**
     * Calculates the size of a Text model
     */
    static void calcSize(const Text* t, double& width, double& height);

    /**
     * Draws a Text modle to a cairo surface
     */
    static void drawText(cairo_t* cr, const Text* t);

    /**
     * Searches text within a Text model, returns XojPopplerRectangle, have to been freed
     */
    static vector<XojPdfRectangle> findText(const Text* t, string& search);

    /**
     * Initialize a Pango layout
     */
    static PangoLayout* initPango(cairo_t* cr, const Text* t);

    /**
     * Sets the font name from Text model
     */
    static void updatePangoFont(PangoLayout* layout, const Text* t);
};
