#include "LatexSettingsPanel.h"

#include <fstream>   // for ifstream, basic_istream
#include <iterator>  // for istreambuf_iterator, ope...
#include <string>    // for allocator, string
#include <variant>   // for get_if

#include <gio/gio.h>      // for GSubprocess, g_subproces...
#include <glib-object.h>  // for g_object_unref, g_object...
#include <glib.h>         // for g_error_free, GError

#include "config-features.h"  // for ENABLE_GTK_SOURCEVIEW
#ifdef ENABLE_GTK_SOURCEVIEW
#include <gtksourceview/gtksource.h>  // for gtk_source_style_scheme_...
#endif

#include "control/latex/LatexGenerator.h"    // for LatexGenerator::GenError
#include "control/settings/LatexSettings.h"  // for LatexSettings
#include "gui/Builder.h"
#include "model/Font.h"              // for XojFont
#include "util/Color.h"              // for Color
#include "util/PathUtil.h"           // for fromGFile, getTmpDir...
#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/XojMsgBox.h"
#include "util/gtk4_helper.h"
#include "util/i18n.h"  // for FS, _F, _
#include "util/raii/CStringWrapper.h"

#include "filesystem.h"  // for path, is_regular_file

class GladeSearchpath;

constexpr auto UI_FILE = "latexSettings.glade";
constexpr auto UI_PANEL_NAME = "latexSettingsPanel";

LatexSettingsPanel::LatexSettingsPanel(GladeSearchpath* gladeSearchPath):
        builder(gladeSearchPath, UI_FILE),
        panel(GTK_SCROLLED_WINDOW(builder.get(UI_PANEL_NAME))),
        cbAutoDepCheck(GTK_CHECK_BUTTON(builder.get("latexSettingsRunCheck"))),
        // Todo(gtk4): replace this GtkFileChooserButton (by what?)
        globalTemplateChooser(GTK_FILE_CHOOSER(builder.get("latexSettingsTemplateFile"))),
        cbUseSystemFont(GTK_CHECK_BUTTON(builder.get("cbUseSystemFont"))) {

    g_signal_connect_swapped(builder.get("latexSettingsTestBtn"), "clicked",
                             G_CALLBACK(+[](LatexSettingsPanel* self) { self->checkDeps(); }), this);
    g_signal_connect_swapped(this->cbUseSystemFont, "toggled",
                             G_CALLBACK(+[](LatexSettingsPanel* self) { self->updateWidgetSensitivity(); }), this);
    g_signal_connect_swapped(builder.get("cbUseExternalEditor"), "toggled",
                             G_CALLBACK(+[](LatexSettingsPanel* self) { self->updateWidgetSensitivity(); }), this);

#ifdef ENABLE_GTK_SOURCEVIEW
    GtkBox* themeSelectionBox = GTK_BOX(builder.get("bxThemeSelectionContainer"));
    this->sourceViewThemeSelector = gtk_source_style_scheme_chooser_button_new();
    gtk_box_append(themeSelectionBox, sourceViewThemeSelector);

    gtk_label_set_text(GTK_LABEL(builder.get("lbSourceviewSettingsDescription")), _("LaTeX editor theme:"));

#if GTK_MAJOR_VERSION == 3
    // Widgets are visible by default in gtk4
    gtk_widget_show(sourceViewThemeSelector);
#endif
#else
    this->sourceViewThemeSelector = nullptr;

    gtk_label_set_text(GTK_LABEL(builder.get("lbSourceviewSettingsDescription")),
                       _("GtkSourceView was disabled when building Xournal++! "
                         "Some options will not be available."));
#endif
}

void LatexSettingsPanel::load(const LatexSettings& settings) {
    gtk_check_button_set_active(this->cbAutoDepCheck, settings.autoCheckDependencies);
    gtk_editable_set_text(GTK_EDITABLE(builder.get("latexDefaultEntry")), settings.defaultText.c_str());
    if (!settings.globalTemplatePath.empty()) {
        gtk_file_chooser_set_file(this->globalTemplateChooser, Util::toGFile(settings.globalTemplatePath).get(),
                                  nullptr);
    }

    switch (settings.type) {
        case LatexSettings::type_t::pdflatex:
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(builder.get("latexSettingsTemplateLaTeX")), 1);
            break;

        case LatexSettings::type_t::typst:
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(builder.get("latexSettingsTemplateTypst")), 1);
            break;
        case LatexSettings::type_t::custom:
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(builder.get("latexSettingsTemplateCustom")), 1);
            break;
    }

    gtk_editable_set_text(GTK_EDITABLE(builder.get("latexSettingsGenCmdBin")), settings.genCmd.c_str());

    gtk_editable_set_text(GTK_EDITABLE(builder.get("latexSettingsGenCmdArgs")), settings.genArgs.c_str());


#ifdef ENABLE_GTK_SOURCEVIEW
    std::string themeId = settings.sourceViewThemeId;
    GtkSourceStyleSchemeManager* themeManager = gtk_source_style_scheme_manager_get_default();
    GtkSourceStyleScheme* theme = gtk_source_style_scheme_manager_get_scheme(themeManager, themeId.c_str());

    if (theme) {
        gtk_source_style_scheme_chooser_set_style_scheme(GTK_SOURCE_STYLE_SCHEME_CHOOSER(this->sourceViewThemeSelector),
                                                         theme);
    }
#endif

    gtk_check_button_set_active(GTK_CHECK_BUTTON(builder.get("cbShowLineNumbers")), settings.sourceViewShowLineNumbers);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(builder.get("cbAutoIndent")), settings.sourceViewAutoIndent);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(builder.get("cbSyntaxHighlight")), settings.sourceViewSyntaxHighlight);

    // Editor font
    std::string editorFontDescription{settings.editorFont.asString()};
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(builder.get("selBtnEditorFont")), editorFontDescription.c_str());

    // Should we use the custom editor font?
    gtk_check_button_set_active(this->cbUseSystemFont, !settings.useCustomEditorFont);

    // Editor word wrap.
    gtk_check_button_set_active(GTK_CHECK_BUTTON(builder.get("cbWordWrap")), settings.editorWordWrap);

    // External editor
    gtk_check_button_set_active(GTK_CHECK_BUTTON(builder.get("cbUseExternalEditor")), settings.useExternalEditor);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(builder.get("cbExternalEditorAutoConfirm")),
                                settings.externalEditorAutoConfirm);
    gtk_editable_set_text(GTK_EDITABLE(builder.get("latexExternalEditorCmd")), settings.externalEditorCmd.c_str());
    gtk_editable_set_text(GTK_EDITABLE(builder.get("latexTemporaryFileExt")), settings.temporaryFileExt.c_str());

    this->updateWidgetSensitivity();

    const bool enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(builder.get("latexSettingsTemplateCustom")));
    gtk_widget_set_sensitive(builder.get("latexSettingsTemplateCustomGrid"), enabled);

    const auto box_refresh = +[](GtkToggleButton*, const gpointer user_data) {
        const auto* self = static_cast<LatexSettingsPanel*>(user_data);
        auto builder = self->builder;

        const bool enable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(builder.get("latexSettingsTemplateCustom")));

        gtk_widget_set_sensitive(builder.get("latexSettingsTemplateCustomGrid"), enable);
    };

    g_signal_connect(builder.get("latexSettingsTemplateLaTeX"), "toggled", G_CALLBACK(box_refresh), this);
    g_signal_connect(builder.get("latexSettingsTemplateTypst"), "toggled", G_CALLBACK(box_refresh), this);
    g_signal_connect(builder.get("latexSettingsTemplateCustom"), "toggled", G_CALLBACK(box_refresh), this);
}

void LatexSettingsPanel::save(LatexSettings& settings) {
    settings.autoCheckDependencies = gtk_check_button_get_active(this->cbAutoDepCheck);
    settings.defaultText = gtk_editable_get_text(GTK_EDITABLE(builder.get("latexDefaultEntry")));
    settings.globalTemplatePath = Util::fromGFile(
            xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(this->globalTemplateChooser), xoj::util::adopt)
                    .get());
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(builder.get("latexSettingsTemplateLaTeX")))) {
        settings.applyTemplate(LatexSettings::type_t::pdflatex);
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(builder.get("latexSettingsTemplateTypst")))) {
        settings.applyTemplate(LatexSettings::type_t::typst);
    } else {
        settings.applyTemplate(LatexSettings::type_t::custom);

        settings.genCmd = gtk_editable_get_text(GTK_EDITABLE(builder.get("latexSettingsGenCmdBin")));
        settings.genArgs = gtk_editable_get_text(GTK_EDITABLE(builder.get("latexSettingsGenCmdArgs")));
    }

#ifdef ENABLE_GTK_SOURCEVIEW
    GtkSourceStyleScheme* theme = gtk_source_style_scheme_chooser_get_style_scheme(
            GTK_SOURCE_STYLE_SCHEME_CHOOSER(this->sourceViewThemeSelector));
    settings.sourceViewThemeId = gtk_source_style_scheme_get_id(theme);
#endif

    settings.sourceViewShowLineNumbers =
            gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get("cbShowLineNumbers")));
    settings.sourceViewAutoIndent = gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get("cbAutoIndent")));
    settings.sourceViewSyntaxHighlight =
            gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get("cbSyntaxHighlight")));

    GtkFontChooser* fontSelector = GTK_FONT_CHOOSER(builder.get("selBtnEditorFont"));
    settings.editorFont = xoj::util::OwnedCString::assumeOwnership(gtk_font_chooser_get_font(fontSelector)).get();
    settings.useCustomEditorFont = !gtk_check_button_get_active(this->cbUseSystemFont);
    settings.editorWordWrap = gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get("cbWordWrap")));

    settings.useExternalEditor = gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get("cbUseExternalEditor")));
    settings.externalEditorAutoConfirm =
            gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get("cbExternalEditorAutoConfirm")));
    settings.externalEditorCmd = gtk_editable_get_text(GTK_EDITABLE(builder.get("latexExternalEditorCmd")));
    settings.temporaryFileExt = gtk_editable_get_text(GTK_EDITABLE(builder.get("latexTemporaryFileExt")));
}

void LatexSettingsPanel::checkDeps() {
    LatexSettings settings;
    this->save(settings);
    std::string msg;
    bool fail = false;

    if (fs::is_regular_file(settings.globalTemplatePath)) {
        // Assume the file is encoded as UTF-8 (open in binary mode to avoid surprises)
        std::ifstream is(settings.globalTemplatePath, std::ios_base::binary);
        if (!is.is_open()) {
            msg = FS(_F("Unable to open global template file at {1}. Does it exist?") %
                     settings.globalTemplatePath.u8string());
            fail = true;
        } else {
            std::string templ(std::istreambuf_iterator<char>(is), {});
            std::string sample = LatexGenerator::templateSub(settings.defaultText, templ, Colors::black);
            auto const& tmpDir = Util::getTmpDirSubfolder("tex");
            auto result = LatexGenerator(settings).asyncRun(tmpDir, sample);
            if (auto* proc = std::get_if<GSubprocess*>(&result)) {
                GError* err = nullptr;
                if (g_subprocess_wait_check(*proc, nullptr, &err)) {
                    msg = _("Sample LaTeX file generated successfully.");
                } else {
                    msg = FS(_F("Error: {1}. Please check the contents of {2}") % err->message % tmpDir.u8string());
                    g_error_free(err);
                    fail = true;
                }
                g_object_unref(*proc);
            } else if (auto* err = std::get_if<LatexGenerator::GenError>(&result)) {
                msg = err->message;
                fail = true;
            }
        }
    } else {
        msg = FS(_F("Error: {1} is not a regular file. Please check your LaTeX template file settings. ") %
                 settings.globalTemplatePath.u8string());
        fail = true;
    }

    GtkWindow* win = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(panel)));
    XojMsgBox::showMessageToUser(win, msg, fail ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO);
}

void LatexSettingsPanel::updateWidgetSensitivity() {
    bool useSystemFont = gtk_check_button_get_active(this->cbUseSystemFont);

    // Only select a custom font if we're not using the system's.
    gtk_widget_set_sensitive(builder.get("boxCustomFontOptions"), !useSystemFont);

#ifndef ENABLE_GTK_SOURCEVIEW
    gtk_widget_set_sensitive(builder.get("bxGtkSourceviewSettings"), false);
#endif

    auto useExternalEditor = gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get("cbUseExternalEditor")));
    gtk_widget_set_sensitive(builder.get("cbExternalEditorAutoConfirm"), useExternalEditor);
    gtk_widget_set_sensitive(builder.get("latexExternalEditorCmd"), useExternalEditor);
    gtk_widget_set_sensitive(builder.get("latexTemporaryFileExt"), useExternalEditor);
}
