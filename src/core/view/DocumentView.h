/*
 * Xournal++
 *
 * Paints a page to a cairo context
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "model/Element.h"
#include "model/ElementContainer.h"
#include "model/Image.h"
#include "model/PageRef.h"
#include "model/Stroke.h"
#include "model/TexImage.h"
#include "model/Text.h"


class EditSelection;
class MainBackgroundPainter;

class DocumentView {
public:
    DocumentView();
    virtual ~DocumentView();

public:
    /**
     * Draw the full page, usually you would like to call this method
     * @param page The page to draw
     * @param cr Draw to thgis context
     * @param dontRenderEditingStroke false to draw currently drawing stroke
     * @param hidePdfBackground true to hide the PDF background
     * @param hideImageBackground true to hide the PDF background
     * @param hideRulingBacground true to hide the ruling background
     */
    void drawPage(PageRef page, cairo_t* cr, bool dontRenderEditingStroke, bool hidePdfBackground = false,
                  bool hideImageBackground = false, bool hideRulingBackground = false);

    void limitArea(double x, double y, double width, double height);

    void drawSelection(cairo_t* cr, ElementContainer* container);

    /**
     * Mark stroke with Audio
     */
    void setMarkAudioStroke(bool markAudioStroke);

    // API for special drawing, usually you won't call this methods
public:
    /**
     * Drawing first step
     * @param page The page to draw
     * @param cr Draw to thgis context
     * @param dontRenderEditingStroke false to draw currently drawing stroke
     */
    void initDrawing(PageRef page, cairo_t* cr, bool dontRenderEditingStroke);

    /**
     * Draw the background
     */
    void drawBackground(bool hidePdfBackground = false, bool hideImageBackground = false,
                        bool hideRulingBackground = false);

    /**
     * Draw background if there is no background shown, like in GIMP etc.
     */
    void drawTransparentBackgroundPattern();

    /**
     * Last step in drawing
     */
    void finializeDrawing();

private:
    cairo_t* cr = nullptr;
    PageRef page = nullptr;
    double width = 0;
    double height = 0;
    bool dontRenderEditingStroke = false;
    bool markAudioStroke = false;

    double lX = -1;
    double lY = -1;
    double lWidth = -1;
    double lHeight = -1;

    MainBackgroundPainter* backgroundPainter;
};
