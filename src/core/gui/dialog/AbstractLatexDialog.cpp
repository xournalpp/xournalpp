#include "gui/dialog/AbstractLatexDialog.h"

#include <algorithm>  // for max, min
#include <memory>

#include "control/LatexController.h"
#include "model/TexImage.h"
#include "util/Range.h"

// Default background color of the preview.
const Color DEFAULT_PREVIEW_BACKGROUND = Colors::white;

AbstractLatexDialog::AbstractLatexDialog(std::unique_ptr<LatexController> ctrl):
        texCtrl{std::move(ctrl)}, previewBackgroundColor{DEFAULT_PREVIEW_BACKGROUND} {
    texCtrl->dlg = this;
}

AbstractLatexDialog::~AbstractLatexDialog() { texCtrl->dlg = nullptr; };

void AbstractLatexDialog::setCompilationStatus(bool isTexValid, bool isCompilationDone,
                                               const std::string& compilationOutput) {
    gtk_widget_set_sensitive(GTK_WIDGET(btOk), isTexValid && isCompilationDone);

    // Show error warning only if LaTeX is invalid.
    gtk_widget_set_opacity(GTK_WIDGET(texErrorLabel), isTexValid ? 0 : 1);

    // Update the output pane.
    gtk_text_buffer_set_text(compilationOutputTextBuffer, compilationOutput.c_str(), -1);
}

void AbstractLatexDialog::setTempRender(PopplerDocument* pdf) {
    if (poppler_document_get_n_pages(pdf) < 1) {
        return;
    }

    this->previewPdfPage.reset(poppler_document_get_page(pdf, 0), xoj::util::adopt);
    this->previewMask.reset();

    // Queue rendering the changed preview.
    gtk_widget_queue_draw(GTK_WIDGET(this->previewDrawingArea));
}

void AbstractLatexDialog::setPreviewBackgroundColor(Color newColor) { this->previewBackgroundColor = newColor; }

void AbstractLatexDialog::previewDrawFunc(GtkDrawingArea*, cairo_t* cr, int width, int height,
                                          AbstractLatexDialog* self) {

    // If we have nothing to render, do nothing!
    if (!self->previewPdfPage) {
        return;
    }

    Range pageRange(0, 0, 0, 0);
    poppler_page_get_size(self->previewPdfPage.get(), &pageRange.maxX, &pageRange.maxY);

    const double zoom =
            std::min(static_cast<double>(width) / pageRange.maxX, static_cast<double>(height) / pageRange.maxY);


    if (!self->previewMask.isInitialized() || self->previewMask.getZoom() < zoom) {
        // Need to rerender the preview
        self->previewMask = xoj::view::Mask(gtk_widget_get_scale_factor(GTK_WIDGET(self->previewDrawingArea)),
                                            pageRange, zoom, CAIRO_CONTENT_COLOR_ALPHA);
        poppler_page_render(self->previewPdfPage.get(), self->previewMask.get());
    }


    Util::cairo_set_source_rgbi(cr, self->previewBackgroundColor);
    cairo_paint(cr);

    // Center the rendering
    const double extraHorizontalSpace = std::max(0.0, width - pageRange.maxX * zoom);
    const double extraVerticalSpace = std::max(0.0, height - pageRange.maxY * zoom);
    cairo_translate(cr, 0.5 * extraHorizontalSpace, 0.5 * extraVerticalSpace);
    cairo_scale(cr, zoom, zoom);
    self->previewMask.paintTo(cr);
}

void AbstractLatexDialog::populateStandardWidgetsFromBuilder(Builder& builder) {
    previewDrawingArea = GTK_DRAWING_AREA(builder.get("texImage"));
    btOk = GTK_BUTTON(builder.get("btOk"));
    btCancel = GTK_BUTTON(builder.get("btCancel"));
    texErrorLabel = GTK_LABEL(builder.get("texErrorLabel"));
    compilationOutputTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(builder.get("texCommandOutputText")));
}

void AbstractLatexDialog::connectStandardSignals() {
    g_signal_connect(btCancel, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
                         auto* self = static_cast<AbstractLatexDialog*>(d);
                         self->texCtrl->cancelEditing();
                         gtk_window_close(self->window.get());
                     }),
                     this);
    g_signal_connect(btOk, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
                         auto* self = static_cast<AbstractLatexDialog*>(d);
                         self->texCtrl->insertTexImage();
                         gtk_window_close(self->window.get());
                     }),
                     this);

    gtk_drawing_area_set_draw_func(this->previewDrawingArea, GtkDrawingAreaDrawFunc(previewDrawFunc), this, nullptr);
    if (texCtrl->temporaryRender) {
        // Use preexisting pdf
        setTempRender(texCtrl->temporaryRender->getPdf());
    }
}
