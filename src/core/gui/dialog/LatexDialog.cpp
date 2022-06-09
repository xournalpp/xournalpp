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

#include "LatexDialog.h"

#include <utility>

#include "control/settings/Settings.h"

// Callbacks for gtk to render the dialog's preview.
extern "C" {
/**
 * @brief Called on 'size-allocate' signal.
 * @param widget is the event's target.
 * @param allocation provides the region given to the widget.
 */
static void resizePreviewCallback(GtkWidget* widget, GdkRectangle* allocation, gpointer _unused);
}

LatexDialog::LatexDialog(GladeSearchpath* gladeSearchPath): GladeGui(gladeSearchPath, "texdialog.glade", "texDialog") {
    GtkContainer* texBoxContainer = GTK_CONTAINER(get("texBoxContainer"));
    this->texBox = gtk_text_view_new();
    gtk_container_add(texBoxContainer, this->texBox);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(this->texBox), true);
    gtk_widget_show_all(GTK_WIDGET(texBoxContainer));

    this->textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(this->texBox));
    this->texTempRender = get("texImage");

    // increase the maximum length to something reasonable.
    // gtk_entry_set_max_length(GTK_TEXT_BUFFER(this->texBox), 500);

    // Background color for the temporary render, default is white because
    // on dark themed DE the LaTex is hard to read
    this->cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(this->cssProvider, "*{background-color:white;padding:10px;}", -1, nullptr);
    gtk_style_context_add_provider(gtk_widget_get_style_context(this->texTempRender),
                                   GTK_STYLE_PROVIDER(this->cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Connect to redraw events for the texImage.
    g_signal_connect(G_OBJECT(this->texTempRender), "draw", G_CALLBACK(drawPreviewCallback), this);
    g_signal_connect(G_OBJECT(this->texTempRender), "size-allocate", G_CALLBACK(resizePreviewCallback), nullptr);
}

LatexDialog::~LatexDialog() {
    if (this->tempRenderSource != nullptr) {
        g_clear_object(&this->tempRenderSource);
    }

    if (this->scaledRender != nullptr) {
        cairo_surface_destroy(this->scaledRender);
        this->scaledRender = nullptr;
    }
}

void LatexDialog::setFinalTex(std::string texString) { this->finalLatex = std::move(texString); }

auto LatexDialog::getFinalTex() -> std::string { return this->finalLatex; }

void LatexDialog::setTempRender(PopplerDocument* pdf) {
    if (poppler_document_get_n_pages(pdf) < 1) {
        return;
    }

    // If a previous render exists, destroy it
    if (this->scaledRender != nullptr) {
        cairo_surface_destroy(this->scaledRender);
        this->scaledRender = nullptr;
    }

    if (this->tempRenderSource != nullptr) {
        g_clear_object(&this->tempRenderSource);
    }

    this->tempRenderSource = poppler_document_get_page(pdf, 0);
    g_object_ref(this->tempRenderSource);

    renderScaledPreview();

    // Queue rendering the changed preview.
    gtk_widget_queue_draw(GTK_WIDGET(this->texTempRender));
}

auto LatexDialog::getPreviewScale(double srcWidth, double srcHeight) const -> double {
    // We need to get the width and height of texTempRender. This shouldn't
    // change the widget, so a const_cast here is appropriate.
    GtkWidget* widget = const_cast<GtkWidget*>(this->texTempRender);

    double widgetWidth = gtk_widget_get_allocated_width(widget);
    double widgetHeight = gtk_widget_get_allocated_height(widget);

    double xFillScaleFactor = widgetWidth / srcWidth;
    double yFillScaleFactor = widgetHeight / srcHeight;

    return std::min(xFillScaleFactor, yFillScaleFactor);
}

auto LatexDialog::getPreviewScale() const -> double {
    if (this->scaledRender == nullptr) {
        return 1.0;
    }

    cairo_surface_t* surface = const_cast<cairo_surface_t*>(this->scaledRender);
    int renderedWidth = cairo_image_surface_get_width(surface);
    int renderedHeight = cairo_image_surface_get_height(surface);

    // Empty previews: No scaling.
    if (renderedWidth == 0 || renderedHeight == 0) {
        return 1.0;
    }

    return getPreviewScale(renderedWidth, renderedHeight);
}

void LatexDialog::renderScaledPreview() {
    double zoom, pageWidth = 0, pageHeight = 0;
    poppler_page_get_size(this->tempRenderSource, &pageWidth, &pageHeight);

    zoom = getPreviewScale(pageWidth, pageHeight);

    this->scaledRender = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, static_cast<int>(pageWidth * zoom),
                                                    static_cast<int>(pageHeight * zoom));
    cairo_t* cr = cairo_create(this->scaledRender);

    cairo_scale(cr, zoom, zoom);
    poppler_page_render(this->tempRenderSource, cr);

    // Free resources.
    cairo_destroy(cr);
}

auto LatexDialog::getTextBuffer() -> GtkTextBuffer* { return this->textBuffer; }

auto LatexDialog::getBufferContents() -> std::string {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(this->textBuffer, &start, &end);
    gchar* chars = gtk_text_buffer_get_text(this->textBuffer, &start, &end, false);
    std::string s = chars;
    g_free(chars);
    return s;
}

void LatexDialog::drawPreviewCallback(GtkWidget* widget, cairo_t* cr, LatexDialog* self) {
    GtkStyleContext* context = gtk_widget_get_style_context(widget);

    guint widgetWidth = gtk_widget_get_allocated_width(widget);
    guint widgetHeight = gtk_widget_get_allocated_height(widget);

    gtk_render_background(context, cr, 0, 0, widgetWidth, widgetHeight);

    // If we have nothing to render, do nothing!
    if (!self->tempRenderSource) {
        return;
    }

    double scaleFactor = self->getPreviewScale();

    // If possible, try not to make the preview larger than rendered.
    if (scaleFactor > 1.0) {
        self->renderScaledPreview();

        // Don't increase the size of the preview if we're at the maximum scale.
        scaleFactor = std::max(1.0, self->getPreviewScale());
    }

    int renderedWidth = cairo_image_surface_get_width(self->scaledRender);
    int renderedHeight = cairo_image_surface_get_height(self->scaledRender);

    // Don't render empty previews.
    if (renderedWidth == 0 || renderedHeight == 0) {
        return;
    }

    // Center the image horizontally and vertically.
    double translateX = 0.0, translateY = 0.0;

    double extraHorizontalSpace = std::max(0.0, widgetWidth - renderedWidth * scaleFactor);
    double extraVerticalSpace = std::max(0.0, widgetHeight - renderedHeight * scaleFactor);

    translateX = extraHorizontalSpace / 2.0;
    translateY = extraVerticalSpace / 2.0;

    cairo_matrix_t matOriginal;
    cairo_matrix_t matTransformed;
    cairo_get_matrix(cr, &matOriginal);
    cairo_get_matrix(cr, &matTransformed);

    matTransformed.xx = scaleFactor;
    matTransformed.yy = scaleFactor;
    matTransformed.xy = 0;
    matTransformed.yx = 0;
    matTransformed.x0 += translateX;
    matTransformed.y0 += translateY;

    cairo_set_matrix(cr, &matTransformed);
    cairo_set_source_surface(cr, self->scaledRender, 0, 0);
    cairo_paint(cr);

    cairo_set_matrix(cr, &matOriginal);
}

static void resizePreviewCallback(GtkWidget* widget, GdkRectangle* allocation, gpointer _unused) {
    gtk_widget_queue_draw(widget);
}

void LatexDialog::show(GtkWindow* parent) { this->show(parent, false); }

void LatexDialog::show(GtkWindow* parent, bool selectText) {
    gtk_text_buffer_set_text(this->textBuffer, this->finalLatex.c_str(), -1);
    if (selectText) {
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(this->textBuffer, &start, &end);
        gtk_text_buffer_select_range(this->textBuffer, &start, &end);
    }

    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    int res = gtk_dialog_run(GTK_DIALOG(this->window));
    this->finalLatex = res == GTK_RESPONSE_OK ? this->getBufferContents() : "";

    gtk_widget_hide(this->window);
}
