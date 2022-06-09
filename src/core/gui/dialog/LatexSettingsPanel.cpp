#include "LatexSettingsPanel.h"

#include <fstream>
#include <variant>

#include <gtksourceview/gtksource.h>

#include "control/latex/LatexGenerator.h"
#include "control/settings/Settings.h"
#include "util/PathUtil.h"
#include "util/i18n.h"

#include "filesystem.h"

LatexSettingsPanel::LatexSettingsPanel(GladeSearchpath* gladeSearchPath):
        GladeGui(gladeSearchPath, "latexSettings.glade", "latexSettingsPanel"),
        cbAutoDepCheck(GTK_TOGGLE_BUTTON(this->get("latexSettingsRunCheck"))),
        cbUseSystemFont(GTK_TOGGLE_BUTTON(this->get("cbUseSystemFont"))),
        globalTemplateChooser(GTK_FILE_CHOOSER(this->get("latexSettingsTemplateFile"))) {
    g_object_ref(this->cbAutoDepCheck);
    g_object_ref(this->cbUseSystemFont);
    g_object_ref(this->globalTemplateChooser);

    g_signal_connect(this->get("latexSettingsTestBtn"), "clicked",
                     G_CALLBACK(+[](GtkWidget*, LatexSettingsPanel* self) { self->checkDeps(); }), this);
    g_signal_connect(GTK_WIDGET(this->cbUseSystemFont), "toggled",
                     G_CALLBACK(+[](GtkWidget*, LatexSettingsPanel* self) { self->updateWidgetSensitivity(); }), this);

    GtkContainer* themeSelectionBoxContainer = GTK_CONTAINER(this->get("bxThemeSelectionContainer"));
    gtk_label_set_text(GTK_LABEL(this->get("lbSourceviewSettingsDescription")), _("LaTeX editor theme:"));
    this->sourceViewThemeSelector = gtk_source_style_scheme_chooser_button_new();

    gtk_container_add(themeSelectionBoxContainer, sourceViewThemeSelector);
    gtk_widget_show_all(GTK_WIDGET(themeSelectionBoxContainer));
    gtk_widget_show(this->get("bxGtkSourceviewMainSettings"));
}

LatexSettingsPanel::~LatexSettingsPanel() {
    g_object_unref(this->cbAutoDepCheck);
    g_object_unref(this->cbUseSystemFont);
    g_object_unref(this->globalTemplateChooser);
}

void LatexSettingsPanel::load(const LatexSettings& settings) {
    gtk_toggle_button_set_active(this->cbAutoDepCheck, settings.autoCheckDependencies);
    if (!settings.globalTemplatePath.empty()) {
        gtk_file_chooser_set_filename(this->globalTemplateChooser,
                                      Util::toGFilename(settings.globalTemplatePath).c_str());
    }
    gtk_entry_set_text(GTK_ENTRY(this->get("latexSettingsGenCmd")), settings.genCmd.c_str());

    std::string themeId = settings.sourceViewThemeId;
    GtkSourceStyleSchemeManager* themeManager = gtk_source_style_scheme_manager_get_default();
    GtkSourceStyleScheme* theme = gtk_source_style_scheme_manager_get_scheme(themeManager, themeId.c_str());

    if (theme) {
        gtk_source_style_scheme_chooser_set_style_scheme(GTK_SOURCE_STYLE_SCHEME_CHOOSER(this->sourceViewThemeSelector),
                                                         theme);
    }

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->get("cbShowLineNumbers")), settings.sourceViewShowLineNumbers);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->get("cbAutoIndent")), settings.sourceViewAutoIndent);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->get("cbSyntaxHighlight")), settings.sourceViewSyntaxHighlight);

    // Editor font
    std::string editorFontDescription{settings.editorFont.asString()};
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(this->get("selBtnEditorFont")), editorFontDescription.c_str());

    // Should we use the custom editor font?
    gtk_toggle_button_set_active(this->cbUseSystemFont, !settings.useCustomEditorFont);

    // Editor word wrap.
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->get("cbWordWrap")), settings.editorWordWrap);

    this->updateWidgetSensitivity();
}

void LatexSettingsPanel::save(LatexSettings& settings) {
    settings.autoCheckDependencies = gtk_toggle_button_get_active(this->cbAutoDepCheck);
    settings.globalTemplatePath = Util::fromGFilename(gtk_file_chooser_get_filename(this->globalTemplateChooser));
    settings.genCmd = gtk_entry_get_text(GTK_ENTRY(this->get("latexSettingsGenCmd")));

    GtkSourceStyleScheme* theme = gtk_source_style_scheme_chooser_get_style_scheme(
            GTK_SOURCE_STYLE_SCHEME_CHOOSER(this->sourceViewThemeSelector));
    settings.sourceViewThemeId = gtk_source_style_scheme_get_id(theme);

    settings.sourceViewShowLineNumbers =
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->get("cbShowLineNumbers")));
    settings.sourceViewAutoIndent = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->get("cbAutoIndent")));
    settings.sourceViewSyntaxHighlight =
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->get("cbSyntaxHighlight")));

    GtkFontChooser* fontSelector = GTK_FONT_CHOOSER(this->get("selBtnEditorFont"));
    std::string fontDescription{gtk_font_chooser_get_font(fontSelector)};
    settings.editorFont = fontDescription;
    settings.useCustomEditorFont = !gtk_toggle_button_get_active(this->cbUseSystemFont);

    settings.editorWordWrap = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->get("cbWordWrap")));
}

void LatexSettingsPanel::show(GtkWindow* parent) {}

void LatexSettingsPanel::checkDeps() {
    LatexSettings settings;
    this->save(settings);
    std::string msg;

    if (fs::is_regular_file(settings.globalTemplatePath)) {
        // Assume the file is encoded as UTF-8 (open in binary mode to avoid surprises)
        std::ifstream is(settings.globalTemplatePath, std::ios_base::binary);
        if (!is.is_open()) {
            msg = FS(_F("Unable to open global template file at {1}. Does it exist?") %
                     settings.globalTemplatePath.u8string().c_str());
        } else {
            std::string templ(std::istreambuf_iterator<char>(is), {});
            std::string sample = LatexGenerator::templateSub("x^2", templ, Color(0x000000U));
            auto const& tmpDir = Util::getTmpDirSubfolder("tex");
            auto result = LatexGenerator(settings).asyncRun(tmpDir, sample);
            if (auto* proc = std::get_if<GSubprocess*>(&result)) {
                GError* err = nullptr;
                if (g_subprocess_wait_check(*proc, nullptr, &err)) {
                    msg = _("Sample LaTeX file generated successfully.");
                } else {
                    msg = FS(_F("Error: {1}. Please check the contents of {2}") % err->message %
                             tmpDir.u8string().c_str());
                    g_error_free(err);
                }
                g_object_unref(*proc);
            } else if (auto* err = std::get_if<LatexGenerator::GenError>(&result)) {
                msg = err->message;
            }
        }
    } else {
        msg = FS(_F("Error: {1} is not a regular file. Please check your LaTeX template file settings. ") %
                 settings.globalTemplatePath.u8string().c_str());
    }
    GtkWidget* dialog =
            gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg.c_str());
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void LatexSettingsPanel::updateWidgetSensitivity() {
    bool useSystemFont = gtk_toggle_button_get_active(this->cbUseSystemFont);

    // Only select a custom font if we're not using the system's.
    gtk_widget_set_sensitive(this->get("boxCustomFontOptions"), !useSystemFont);
}
