/*
 * Xournal++
 *
 * Draw stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

class Stroke;

class StrokeView {
public:
    StrokeView(cairo_t* cr, Stroke* s, int startPoint, double scaleFactor, bool noAlpha);
    ~StrokeView() = default;

public:
    void paint(bool dontRenderEditingStroke);

    /**
     * Change cairo source, used to draw highlighter transparent,
     * but only if not currently drawing and so on (yes, complicated)
     */
    void changeCairoSource(bool markAudioStroke);

private:
    void drawFillStroke();
    void applyDashed(double offset);
    static void drawEraseableStroke(cairo_t* cr, Stroke* s);

    /**
     * No pressure sensitivity, one line is drawn
     */
    void drawNoPressure();

    /**
     * Draw a stroke with pressure, for this multiple
     * lines with different widths needs to be drawn
     */
    void drawWithPressure();

    /**
     * Draw a calligraphic stroke
     */
    void drawCalligraphicOnePolygon(double nibAngle, double thickness);

    void draw_path_calligraphic(cairo_path_t* path, double angle, double thickness, bool fill);

    void stroke_calligraphic(double angle, double thickness);

    void outline_calligraphic(double angle, double thickness);

    void draw_calligraphic(double angle, double thickness);

private:
    cairo_t* cr;
    Stroke* s;

    int startPoint;
    double scaleFactor;
    bool noAlpha;
};
