/*
 * Xournal++
 *
 * Latex implementation
 *
 * @author W Brenna
 * http://wbrenna.ca
 *
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <cairo.h>               // for cairo_surface_t, cairo_t
#include <gtk/gtk.h>             // for GtkWidget, GtkTextBuffer, GtkWindow
#include <gtk/gtkcssprovider.h>  // for GtkCssProvider
#include <poppler.h>             // for PopplerDocument, PopplerPage

#include "gui/GladeGui.h"  // for GladeGui
#include "util/Color.h"    // for Color

class LatexSettings;
class GladeSearchpath;

class LatexDialog: public GladeGui {
public:
    LatexDialog() = delete;
    LatexDialog(const LatexDialog& other) = delete;
    LatexDialog& operator=(const LatexDialog& other) = delete;
    LatexDialog(GladeSearchpath* gladeSearchPath, const LatexSettings& settings);
    virtual ~LatexDialog();

public:
    /**
     * Show the dialog.
     */
    void show(GtkWindow* parent) override;

    /**
     * Show the dialog, optionally selecting the text field contents by default.
     */
    void show(GtkWindow* parent, bool selectTex);

    // Set and retrieve text from text box
    void setFinalTex(std::string texString);
    std::string getFinalTex();

    /**
     * Set temporary Tex render and queue a re-draw.
     * @param pdf PDF document with rendered TeX.
     */
    void setTempRender(PopplerDocument* pdf);

    /**
     * Set TeX preview background color to the given color.
     * @param color New preview background color.
     */
    void setPreviewBackgroundColor(Color color);

    // Necessary for the controller in order to connect the 'text-changed'
    // signal handler
    GtkTextBuffer* getTextBuffer();

    /**
     * @return The contents of the formula input text buffer.
     */
    std::string getBufferContents();

private:
    /**
     * @brief Get the factor by which a preview should be scaled to fit in
     *    the preview widget.
     * @param srcWidth is the width of the unscaled preview.
     * @param srcHeight is the height of the unscaled preview.
     * @return An appropriate scale factor for a preview of the given dimension.
     */
    double getPreviewScale(double srcWidth, double srcHeight) const;

    /**
     * @return An appropriate scale factor for this' scaledRender.
     *         Returns a scale factor of 1.0 if the cached render is null
     *         or has zero size.
     */
    double getPreviewScale() const;

    /**
     * @brief Render a reasonably-scaled preview for the current preview
     *  PDF to this' internal rendering surface.
     * This does not queue a re-draw and may, therefore, be called by the draw
     * preview callback itself.
     */
    void renderScaledPreview();

    /**
     * @brief Initialize or re-initialize CSS. Applies styling to the editor,
     * preview, etc.
     */
    void setupCSS();

private:
    /**
     * @brief Called on 'draw' signal.
     * @param widget is the target of the event; the GtkDrawingArea we're rendering to.
     * @param cr is drawn to
     * @param self The LatexDialog that is the source and target of the callback.
     */
    static void drawPreviewCallback(GtkWidget* widget, cairo_t* cr, LatexDialog* self);

private:
    // Temporary render
    GtkWidget* texTempRender;
    cairo_surface_t* scaledRender = nullptr;
    GtkCssProvider* cssProvider;

    // Text field
    GtkWidget* texBox;
    GtkTextBuffer* textBuffer;

    /**
     * Source page from which we render the preview.
     */
    PopplerPage* tempRenderSource = nullptr;

    /**
     * The final LaTeX string to save once the dialog is closed.
     */
    std::string finalLatex;

    /**
     * Background color for the LaTeX preview.
     */
    Color previewBackgroundColor;

    /**
     * Constant CSS for the tex box.
     */
    std::string texBoxCss;
};
