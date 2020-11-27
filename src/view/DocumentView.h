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
#include "model/Image.h"
#include "model/PageRef.h"
#include "model/Stroke.h"
#include "model/TexImage.h"
#include "model/Text.h"

#include "ElementContainer.h"
#include "XournalType.h"

class Control;
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
     * @param cr Draw to this context
     * @param dontRenderEditingStroke false to draw currently drawing stroke
     * @param hideBackground true to hide the background
     * @param control to get the settings, in case they modify the way the page is drawn
     */
    void drawPage(PageRef page, cairo_t* cr, bool dontRenderEditingStroke, bool hideBackground = false, Control* control = nullptr);


    void drawStroke(cairo_t* cr, Stroke* s, int startPoint = 0, double scaleFactor = 1, bool changeSource = true,
                    bool noAlpha = false) const;

    static void applyColor(cairo_t* cr, Stroke* s);
    static void applyColor(cairo_t* cr, Color c, uint8_t alpha = 0xff);
    static void applyColor(cairo_t* cr, Element* e, uint8_t alpha = 0xff);

    void limitArea(double x, double y, double width, double height);

    void drawSelection(cairo_t* cr, ElementContainer* container, Control* control = nullptr);

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
    void drawBackground();

    /**
     * Draw background if there is no background shown, like in GIMP etc.
     */
    void drawTransparentBackgroundPattern();

    /**
     * Draw a single layer
     * @param cr Draw to thgis context
     * @param l The layer to draw
     * @param control To get the settings for thw drawing (it can be nullptr)
     */
    void drawLayer(cairo_t* cr, Layer* l, Control* control);

    /**
     * Last step in drawing
     */
    void finializeDrawing();

private:
    static void drawText(cairo_t* cr, Text* t, bool autoDetectHyperLinks);
    static void drawImage(cairo_t* cr, Image* i);
    static void drawTexImage(cairo_t* cr, TexImage* texImage);

    void drawElement(cairo_t* cr, Element* e, Control* control) const;

    void paintBackgroundImage();

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
