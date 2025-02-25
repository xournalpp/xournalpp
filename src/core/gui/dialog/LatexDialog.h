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

#include <functional>
#include <string>  // for string

#include <cairo.h>               // for cairo_t
#include <gtk/gtk.h>             // for GtkWidget, GtkTextBuffer, GtkWindow
#include <poppler.h>             // for PopplerDocument, PopplerPage

#include "util/Color.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/GtkWindowUPtr.h"
#include "view/Mask.h"

class LatexController;
class LatexSettings;
class GladeSearchpath;

class LatexDialog final {
public:
    LatexDialog() = delete;
    LatexDialog(const LatexDialog& other) = delete;
    LatexDialog& operator=(const LatexDialog& other) = delete;
    LatexDialog(GladeSearchpath* gladeSearchPath, std::unique_ptr<LatexController> texCtrl);
    ~LatexDialog();

public:
    void setCompilationStatus(bool isTexValid, bool isCompilationDone, const std::string& compilationOutput);

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

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    /**
     * @brief Preview DrawingArea draw function
     */
    static void previewDrawFunc(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, LatexDialog* self);

private:
    std::unique_ptr<LatexController> texCtrl;
    xoj::util::GtkWindowUPtr window;

    // Preview
    GtkDrawingArea* previewDrawingArea;
    xoj::view::Mask previewMask;

    // Text field
    xoj::util::GObjectSPtr<GtkCssProvider> cssProvider;  ///< For tex code display font and size
    GtkWidget* texBox;
    GtkButton* btOk;
    GtkLabel* texErrorLabel;
    GtkTextBuffer* compilationOutputTextBuffer;
    GtkTextBuffer* textBuffer;

    /**
     * Source page from which we render the preview.
     */
    xoj::util::GObjectSPtr<PopplerPage> previewPdfPage;

    /**
     * The final LaTeX string to save once the dialog is closed.
     */
    std::string finalLatex;

    /**
     * Background color for the LaTeX preview.
     */
    Color previewBackgroundColor;

    std::function<void()> callback;
};
