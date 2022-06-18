/*
 * Xournal++
 *
 * Settings Dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_surface_t, cairo_t

class Shadow {
private:
    Shadow();
    virtual ~Shadow();

    static void drawShadowEdge(cairo_t* cr, int x, int y, int width, int height, const unsigned char* edge, double r,
                               double g, double b);
    void drawShadowImpl(cairo_t* cr, int x, int y, int width, int height);

    static void paintEdge(cairo_t* cr, cairo_surface_t* image, int x, int y, int width, int height);

    void drawShadowTop(cairo_t* cr, int x, int y, int width, double r, double g, double b);
    void drawShadowLeft(cairo_t* cr, int x, int y, int height, double r, double g, double b);
    void drawShadowRight(cairo_t* cr, int x, int y, int height, double r, double g, double b);
    void drawShadowBottom(cairo_t* cr, int x, int y, int width, double r, double g, double b);

public:
    /**
     * This is the public interface of this class
     */
    static void drawShadow(cairo_t* cr, int x, int y, int width, int height);
    static int getShadowBottomRightSize();
    static int getShadowTopLeftSize();

private:
    static Shadow* instance;

    cairo_surface_t* edgeTopLeft = nullptr;
    cairo_surface_t* edgeTopRight = nullptr;
    cairo_surface_t* edgeBottomLeft = nullptr;
    cairo_surface_t* edgeBottomRight = nullptr;

    cairo_surface_t* top = nullptr;
    cairo_surface_t* right = nullptr;
    cairo_surface_t* bottom = nullptr;
    cairo_surface_t* left = nullptr;
};
