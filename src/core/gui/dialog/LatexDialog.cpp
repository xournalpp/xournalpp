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

#include <algorithm>  // for max, min
#include <cmath>
#include <sstream>    // for operator<<, basic_ostream
#include <utility>    // for move

#include <glib.h>              // for g_free, gpointer, guint
#include <poppler-document.h>  // for poppler_document_get_n_p...
#include <poppler-page.h>      // for poppler_page_get_size

#ifdef USE_GTK_SOURCEVIEW
#include <gtksourceview/gtksource.h>  // for GTK_SOURCE_VIEW, gtk_sou...
#endif

#include "control/LatexController.h"
#include "control/settings/LatexSettings.h"  // for LatexSettings
#include "gui/Builder.h"
#include "model/Font.h"  // for XojFont
#include "model/TexImage.h"
#include "util/Range.h"
#include "util/StringUtils.h"  // for replace_pair, StringUtils
#include "util/gtk4_helper.h"
#include "util/raii/CStringWrapper.h"

class GladeSearchpath;

// Default background color of the preview.
const Color DEFAULT_PREVIEW_BACKGROUND = Colors::white;

constexpr auto UI_FILE_NAME = "texdialog.ui";
constexpr auto UI_DIALOG_ID = "texDialog";

constexpr auto TEX_BOX_WIDGET_NAME = "texBox";
constexpr auto PREVIEW_WIDGET_ID = "texImage";

LatexDialog::LatexDialog(GladeSearchpath* gladeSearchPath, std::unique_ptr<LatexController> ctrl):
        texCtrl(std::move(ctrl)),
        cssProvider(gtk_css_provider_new(), xoj::util::adopt),
        previewBackgroundColor{DEFAULT_PREVIEW_BACKGROUND} {
    texCtrl->dlg = this;

    Builder builder(gladeSearchPath, UI_FILE_NAME);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_ID)));

    previewDrawingArea = GTK_DRAWING_AREA(builder.get(PREVIEW_WIDGET_ID));
    btOk = GTK_BUTTON(builder.get("btOk"));
    texErrorLabel = GTK_LABEL(builder.get("texErrorLabel"));
    compilationOutputTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(builder.get("texCommandOutputText")));

#ifdef USE_GTK_SOURCEVIEW
    this->texBox = gtk_source_view_new();
#else
    this->texBox = gtk_text_view_new();
#endif

    gtk_widget_set_name(this->texBox, TEX_BOX_WIDGET_NAME);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(this->texBox), true);

    this->textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(this->texBox));
    const auto& settings = texCtrl->settings;
    if (texCtrl->initialTex.empty()) {
        gtk_text_buffer_set_text(this->textBuffer, settings.defaultText.c_str(), -1);
        // Preselect the default
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(this->textBuffer, &start, &end);
        gtk_text_buffer_select_range(this->textBuffer, &start, &end);
    } else {
        gtk_text_buffer_set_text(this->textBuffer, texCtrl->initialTex.c_str(), -1);
    }

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(builder.get("texBoxContainer")), this->texBox);


#ifdef USE_GTK_SOURCEVIEW
    // We own neither the languageManager, the styleSchemeManager, nor the sourceLanguage.
    // Do not attempt to free them.
    GtkSourceStyleSchemeManager* styleSchemeManager = gtk_source_style_scheme_manager_get_default();
    GtkSourceLanguageManager* lm = gtk_source_language_manager_get_default();

    // Select the TeX highlighting scheme
    GtkSourceLanguage* lang = gtk_source_language_manager_guess_language(lm, "file.tex", nullptr);
    std::string styleSchemeId = settings.sourceViewThemeId;
    GtkSourceStyleScheme* styleScheme =
            gtk_source_style_scheme_manager_get_scheme(styleSchemeManager, styleSchemeId.c_str());

    if (settings.sourceViewSyntaxHighlight) {
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(this->textBuffer), lang);
    }

    gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(this->texBox), settings.sourceViewAutoIndent);
    gtk_source_view_set_indent_on_tab(GTK_SOURCE_VIEW(this->texBox), settings.sourceViewAutoIndent);

    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(this->texBox), settings.sourceViewShowLineNumbers);

    if (styleScheme) {
        gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(this->textBuffer), styleScheme);
    }
#endif

    // Enable/disable word-wrap.
    GtkWrapMode texBoxWrapMode = GTK_WRAP_NONE;
    if (settings.editorWordWrap) {
        texBoxWrapMode = GTK_WRAP_WORD_CHAR;
    }

    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(this->texBox), texBoxWrapMode);

    gtk_drawing_area_set_draw_func(this->previewDrawingArea, GtkDrawingAreaDrawFunc(previewDrawFunc), this, nullptr);

    std::stringstream texBoxCssBuilder;
    if (settings.useCustomEditorFont) {
        std::string fontName = settings.editorFont.getName();

        // Escape "'" and "\" characters in the fontName
        StringUtils::replaceAllChars(fontName, {replace_pair('\\', "\\\\")});
        StringUtils::replaceAllChars(fontName, {replace_pair('\'', "\\'")});

        texBoxCssBuilder << "#" << TEX_BOX_WIDGET_NAME << " {";
        texBoxCssBuilder << "  font-size: " << settings.editorFont.getSize() << "pt;";
        texBoxCssBuilder << "  font-family: '" << fontName << "';";
        texBoxCssBuilder << " } ";

        gtk_css_provider_load_from_data(this->cssProvider.get(), texBoxCssBuilder.str().c_str(), -1);

        // Apply the CSS to both the texBox and the drawing area.
        gtk_style_context_add_provider(gtk_widget_get_style_context(GTK_WIDGET(this->previewDrawingArea)),
                                       GTK_STYLE_PROVIDER(this->cssProvider.get()),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        gtk_style_context_add_provider(gtk_widget_get_style_context(GTK_WIDGET(this->texBox)),
                                       GTK_STYLE_PROVIDER(this->cssProvider.get()),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    g_signal_connect(builder.get("btCancel"), "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
                         auto* self = static_cast<LatexDialog*>(d);
                         self->texCtrl->cancelEditing();
                         gtk_window_close(self->window.get());
                     }),
                     this);
    g_signal_connect(builder.get("btOk"), "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
                         auto* self = static_cast<LatexDialog*>(d);
                         self->texCtrl->insertTexImage();
                         gtk_window_close(self->window.get());
                     }),
                     this);

    if (texCtrl->temporaryRender) {
        // Use preexisting pdf
        this->setTempRender(texCtrl->temporaryRender->getPdf());
    }

    /*
     * Connect handleTexChanged() to the text buffer containing the latex code, to trigger rebuilds upon code changes.
     */
    g_signal_connect(this->getTextBuffer(), "changed", G_CALLBACK(+[](GtkTextBuffer*, gpointer ctrl) {
                         LatexController::handleTexChanged(static_cast<LatexController*>(ctrl));
                     }),
                     texCtrl.get());

    g_signal_connect(this->getWindow(), "close-request", G_CALLBACK(+[](GtkWidget*, gpointer d) -> gboolean {
                         auto self = static_cast<LatexDialog*>(d);
                         /**
                          * dlg->getTextBuffer() may survive the dialog. When copying all of its content, the
                          * clipboard owns a ref to the GtkTextBuffer. We must disconnect the signal to avoid
                          * dereferencing `self` after its destruction
                          */
                         g_signal_handlers_disconnect_by_data(self->getTextBuffer(), self->texCtrl.get());
                         return false;  // Let the callback from PopupWindowWrapper delete the dialog
                     }),
                     this);

    g_signal_connect(this->getWindow(), "show", G_CALLBACK(+[](GtkWidget*, gpointer d) {
                         auto* texCtrl = static_cast<LatexController*>(d);
                         if (!texCtrl->temporaryRender) {
                             // Trigger an asynchronous compilation if we are not using a preexisting TexImage
                             // Keep this after popup.show() so that if an error message is to be displayed (e.g.
                             // missing Tex executable), it'll appear on top of the LatexDialog.
                             LatexController::handleTexChanged(texCtrl);
                         }
                     }),
                     texCtrl.get());
}

LatexDialog::~LatexDialog() = default;

void LatexDialog::setCompilationStatus(bool isTexValid, bool isCompilationDone, const std::string& compilationOutput) {
    gtk_widget_set_sensitive(GTK_WIDGET(btOk), isTexValid && isCompilationDone);

    // Show error warning only if LaTeX is invalid.
    gtk_widget_set_opacity(GTK_WIDGET(texErrorLabel), isTexValid ? 0 : 1);

    // Update the output pane.
    gtk_text_buffer_set_text(compilationOutputTextBuffer, compilationOutput.c_str(), -1);
}

void LatexDialog::setTempRender(PopplerDocument* pdf) {
    if (poppler_document_get_n_pages(pdf) < 1) {
        return;
    }

    this->previewPdfPage.reset(poppler_document_get_page(pdf, 0), xoj::util::adopt);
    this->previewMask.reset();

    // Queue rendering the changed preview.
    gtk_widget_queue_draw(GTK_WIDGET(this->previewDrawingArea));
}

void LatexDialog::setPreviewBackgroundColor(Color newColor) { this->previewBackgroundColor = newColor; }

auto LatexDialog::getTextBuffer() -> GtkTextBuffer* { return this->textBuffer; }

auto LatexDialog::getBufferContents() -> std::string {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(this->textBuffer, &start, &end);
    auto content =
            xoj::util::OwnedCString::assumeOwnership(gtk_text_buffer_get_text(this->textBuffer, &start, &end, false));
    return content.get();
}

void LatexDialog::previewDrawFunc(GtkDrawingArea*, cairo_t* cr, int width, int height, LatexDialog* self) {

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
