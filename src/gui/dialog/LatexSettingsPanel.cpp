#include "LatexSettingsPanel.h"

LatexSettingsPanel::LatexSettingsPanel(GladeSearchpath* gladeSearchPath):
        GladeGui(gladeSearchPath, "latexSettings.glade", "latexSettingsPanel"),
        cbAutoDepCheck(GTK_TOGGLE_BUTTON(this->get("latexSettingsRunCheck"))),
        globalTemplateChooser(GTK_FILE_CHOOSER(this->get("latexSettingsTemplateFile"))) {
    g_object_ref(this->cbAutoDepCheck);
    g_object_ref(this->globalTemplateChooser);
}

LatexSettingsPanel::~LatexSettingsPanel() {
    g_object_unref(this->cbAutoDepCheck);
    g_object_unref(this->globalTemplateChooser);
}

void LatexSettingsPanel::load(const LatexSettings& settings) {
    gtk_toggle_button_set_active(this->cbAutoDepCheck, settings.autoCheckDependencies);
    if (!settings.globalTemplatePath.empty()) {
        gtk_file_chooser_set_filename(this->globalTemplateChooser, settings.globalTemplatePath.u8string().c_str());
    }
    gtk_entry_set_text(GTK_ENTRY(this->get("latexSettingsKpsewhich")), settings.kpsewhichCmd.c_str());
    gtk_entry_set_text(GTK_ENTRY(this->get("latexSettingsGenCmd")), settings.genCmd.c_str());
}

void LatexSettingsPanel::save(LatexSettings& settings) {
    settings.autoCheckDependencies = gtk_toggle_button_get_active(this->cbAutoDepCheck);
    gchar* templPath = gtk_file_chooser_get_filename(this->globalTemplateChooser);
    if (templPath) {
        settings.globalTemplatePath = templPath;
        g_free(templPath);
    } else {
        settings.globalTemplatePath = "";
    }
    settings.kpsewhichCmd = gtk_entry_get_text(GTK_ENTRY(this->get("latexSettingsKpsewhich")));
    settings.genCmd = gtk_entry_get_text(GTK_ENTRY(this->get("latexSettingsGenCmd")));
}

void LatexSettingsPanel::show(GtkWindow* parent) {}
