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

#include "IntEdLatexDialog.h"

#include <sstream>  // for operator<<, basic_ostream
#include <utility>  // for move

#include <glib.h>              // for g_free, gpointer, guint
#include <poppler-document.h>  // for poppler_document_get_n_p...
#include <poppler-page.h>      // for poppler_page_get_size

#include "AbstractLatexDialog.h"

#ifdef ENABLE_GTK_SOURCEVIEW
#include <gtksourceview/gtksource.h>  // for GTK_SOURCE_VIEW, gtk_sou...
#endif

#include "control/LatexController.h"
#include "control/settings/LatexSettings.h"  // for LatexSettings
#include "gui/Builder.h"
#include "model/Font.h"        // for XojFont
#include "util/StringUtils.h"  // for replace_pair, StringUtils
#include "util/raii/CStringWrapper.h"

class GladeSearchpath;

constexpr auto UI_FILE_NAME = "intEdTexDialog.glade";
constexpr auto UI_DIALOG_ID = "intEdTexDialog";

constexpr auto TEX_BOX_WIDGET_NAME = "texBox";

IntEdLatexDialog::IntEdLatexDialog(GladeSearchpath* gladeSearchPath, std::unique_ptr<LatexController> ctrl):
        AbstractLatexDialog(std::move(ctrl)), cssProvider(gtk_css_provider_new(), xoj::util::adopt) {

    Builder builder(gladeSearchPath, UI_FILE_NAME);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_ID)));

    populateStandardWidgetsFromBuilder(builder);

#ifdef ENABLE_GTK_SOURCEVIEW
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

#if GTK_MAJOR_VERSION == 3
    // Widgets are visible by default in gtk4
    gtk_widget_show_all(GTK_WIDGET(texBox));
#endif

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(builder.get("texBoxContainer")), this->texBox);


#ifdef ENABLE_GTK_SOURCEVIEW
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

        gtk_css_provider_load_from_data(this->cssProvider.get(), texBoxCssBuilder.str().c_str(), -1, nullptr);

        // Apply the CSS to both the texBox and the drawing area.
        gtk_style_context_add_provider(gtk_widget_get_style_context(GTK_WIDGET(this->previewDrawingArea)),
                                       GTK_STYLE_PROVIDER(this->cssProvider.get()),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        gtk_style_context_add_provider(gtk_widget_get_style_context(GTK_WIDGET(this->texBox)),
                                       GTK_STYLE_PROVIDER(this->cssProvider.get()),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    connectStandardSignals();

    /*
     * Connect handleTexChanged() to the text buffer containing the latex code, to trigger rebuilds upon code changes.
     */
    g_signal_connect(this->getTextBuffer(), "changed", G_CALLBACK(+[](GtkTextBuffer*, gpointer ctrl) {
                         LatexController::handleTexChanged(static_cast<LatexController*>(ctrl));
                     }),
                     texCtrl.get());

    g_signal_connect(this->getWindow(), "delete-event", G_CALLBACK(+[](GtkWidget*, GdkEvent*, gpointer d) -> gboolean {
                         auto self = static_cast<IntEdLatexDialog*>(d);
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
                         auto texCtrl = static_cast<LatexController*>(d);
                         if (!texCtrl->temporaryRender) {
                             // Trigger an asynchronous compilation if we are not using a preexisting TexImage
                             // Keep this after popup.show() so that if an error message is to be displayed (e.g.
                             // missing Tex executable), it'll appear on top of the LatexDialog.
                             LatexController::handleTexChanged(texCtrl);
                         }
                     }),
                     texCtrl.get());
}

IntEdLatexDialog::~IntEdLatexDialog() = default;


auto IntEdLatexDialog::getTextBuffer() -> GtkTextBuffer* { return this->textBuffer; }

auto IntEdLatexDialog::getBufferContents() -> std::string {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(this->textBuffer, &start, &end);
    auto content =
            xoj::util::OwnedCString::assumeOwnership(gtk_text_buffer_get_text(this->textBuffer, &start, &end, false));
    return content.get();
}
