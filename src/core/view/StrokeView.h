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

#include <cstdint>

#include <gtk/gtk.h>

class Stroke;

class StrokeView {
public:
    StrokeView(cairo_t* cr, Stroke* s);
    ~StrokeView() = default;

public:
    /**
     * @brief Paint the given stroke.
     * @param dontRenderEditingStroke If true, and if the stroke is currently being (partially) erased, then render only
     * the not-yet-erased parts. (Typically set to true, except for previews and export jobs)
     * @param markAudioStroke If true, the stroke is faded out if it has no audio playback attached.
     * @param noColor If true, paint as if on a colorblind mask (only the alpha values are painted).
     */
    void paint(bool dontRenderEditingStroke, bool markAudioStroke, bool noColor = false) const;

private:
    inline void pathToCairo() const;
    static void drawErasableStroke(cairo_t* cr, Stroke* s);

    /**
     * No pressure sensitivity, one line is drawn
     */
    void drawNoPressure() const;

    /**
     * Draw a stroke with pressure, for this multiple
     * lines with different widths needs to be drawn
     */
    void drawWithPressure() const;


private:
    cairo_t* cr;
    mutable cairo_t* crEffective;
    Stroke* s;

public:
    static constexpr uint8_t HIGHLIGHTER_ALPHA = 120;
    static constexpr double MINIMAL_ALPHA = 10;
};
