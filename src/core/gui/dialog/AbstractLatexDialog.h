/*
 * Xournal++
 *
 * Latex implementation
 *
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <memory>  // for unique_ptr

#include <poppler.h>  // for PopplerDocument

#include "gui/Builder.h"              // for Builder
#include "util/Color.h"               // for Color
#include "util/raii/GObjectSPtr.h"    // for GObjectSPtr
#include "util/raii/GtkWindowUPtr.h"  // for GtkWindowUPtr
#include "view/Mask.h"                // for Mask

class LatexController;

class AbstractLatexDialog {
public:
    AbstractLatexDialog() = delete;
    AbstractLatexDialog(const AbstractLatexDialog& other) = delete;
    AbstractLatexDialog& operator=(const AbstractLatexDialog& other) = delete;
    AbstractLatexDialog(std::unique_ptr<LatexController> texCtrl);
    virtual ~AbstractLatexDialog();

public:
    virtual void setCompilationStatus(bool isTexValid, bool isCompilationDone, const std::string& compilationOutput);

    virtual std::string getBufferContents() = 0;

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

    inline GtkWindow* getWindow() const { return window.get(); }

protected:
    /**
     * @brief Preview DrawingArea draw function
     */
    static void previewDrawFunc(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height,
                                AbstractLatexDialog* self);

    /**
     * @brief populate the widgets stored in AbstractLatexDialog from the given builder with their
     * standard names.
     */
    void populateStandardWidgetsFromBuilder(Builder& builder);

    /**
     * Connect the cancel button to cancelEditing, the Ok button to insertTexImage as well as the
     * draw handler for the preview area.
     */
    void connectStandardSignals();

protected:
    std::unique_ptr<LatexController> texCtrl;
    xoj::util::GtkWindowUPtr window;
    GtkButton* btOk;
    GtkButton* btCancel;

    // Preview
    GtkDrawingArea* previewDrawingArea;
    xoj::view::Mask previewMask;

    /**
     * Source page from which we render the preview.
     */
    xoj::util::GObjectSPtr<PopplerPage> previewPdfPage;

    GtkLabel* texErrorLabel;
    GtkTextBuffer* compilationOutputTextBuffer;

    /**
     * Background color for the LaTeX preview.
     */
    Color previewBackgroundColor;
};
