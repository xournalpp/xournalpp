#include "LanguageConfigGui.h"
#include "i18n.h"
#include "control/settings/Settings.h"

LanguageConfigGui::LanguageConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings) :
        GladeGui(gladeSearchPath, "settingsLanguageConfig.glade", "offscreenwindow"), settings(settings) {
    GtkWidget* dropdown = get("languageSettingsDropdown");
    gtk_container_remove(GTK_CONTAINER(getWindow()), dropdown);
    gtk_container_add(GTK_CONTAINER(w), dropdown);
    gtk_widget_show_all(dropdown);
};

void LanguageConfigGui::saveSettings() {
    gint pos = gtk_combo_box_get_active(GTK_COMBO_BOX(get("languageSettingsDropdown")));

    switch (pos) {
        case 0:
            settings->setPreferredLocale("en_US.UTF-8");
            break;
        case 1:
            settings->setPreferredLocale("de_DE.UTF-8");
            break;
        default:
            throw "No language selected";
    }
    settings->customSettingsChanged();
}
