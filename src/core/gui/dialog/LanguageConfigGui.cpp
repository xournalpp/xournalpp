#include "LanguageConfigGui.h"

#include <algorithm>  // for find, lower_bound, sort
#include <cstdlib>    // for setenv
#include <memory>     // for allocator, __alloc_traits<>::...

#include <glib-object.h>  // for G_TYPE_STRING
#include <glib.h>         // for g_warning, gint

#include "control/settings/Settings.h"  // for Settings
#include "util/PathUtil.h"              // for getGettextFilepath, getLocale...
#include "util/StringUtils.h"           // for StringUtils
#include "util/XojMsgBox.h"             // for XojMsgBox
#include "util/i18n.h"                  // for _
#include "util/safe_casts.h"            // for as_unsigned

#include "config.h"      // for GETTEXT_PACKAGE
#include "filesystem.h"  // for directory_iterator, operator/

class GladeSearchpath;

LanguageConfigGui::LanguageConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings):
        GladeGui(gladeSearchPath, "settingsLanguageConfig.glade", "languageSettingsDropdown"), settings(settings) {
    auto dropdown = get("languageSettingsDropdown");
    gtk_box_append(GTK_BOX(w), dropdown);

    // Fetch available locales
    try {
        fs::path baseLocaleDir = Util::getGettextFilepath(Util::getLocalePath().u8string().c_str());
        for (auto const& d: fs::directory_iterator(baseLocaleDir)) {
            if (fs::exists(d.path() / "LC_MESSAGES" / (std::string(GETTEXT_PACKAGE) + ".mo"))) {
                availableLocales.push_back(d.path().filename().u8string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        g_warning("%s", e.what());
    }
    std::sort(availableLocales.begin(), availableLocales.end());

    // No pot file for English
    if (auto enPos = std::lower_bound(availableLocales.begin(), availableLocales.end(), "en");
        enPos != availableLocales.end() && !StringUtils::startsWith(*enPos, "en")) {
        availableLocales.insert(enPos, "en");
    }

    // Default
    availableLocales.insert(availableLocales.begin(), _("System Default"));

    auto gtkAvailableLocales = gtk_list_store_new(1, G_TYPE_STRING);
    for (auto const& i: availableLocales) {
        GtkTreeIter iter;
        gtk_list_store_append(gtkAvailableLocales, &iter);
        gtk_list_store_set(gtkAvailableLocales, &iter, 0, i.c_str(), -1);
    }
    gtk_combo_box_set_model(GTK_COMBO_BOX(dropdown), GTK_TREE_MODEL(gtkAvailableLocales));


    // Set the current locale if previously selected
    auto prefPos = [&]() {
        if (auto preferred = settings->getPreferredLocale(); !preferred.empty()) {
            if (auto &&endi = availableLocales.end(), prefPos = std::find(availableLocales.begin(), endi, preferred);
                prefPos != endi) {
                return prefPos;
            }
            XojMsgBox::showErrorToUser(nullptr, _("Previously selected language not available anymore!"));
        }
        // Use system default
        return availableLocales.begin();
    }();
    gtk_combo_box_set_active(GTK_COMBO_BOX(dropdown),
                             static_cast<gint>(std::distance(availableLocales.begin(), prefPos)));
}


void LanguageConfigGui::saveSettings() {
    gint pos = gtk_combo_box_get_active(GTK_COMBO_BOX(get("languageSettingsDropdown")));
    auto pref = (pos == 0) ? "" : availableLocales[as_unsigned(pos)];

    settings->setPreferredLocale(pref);
    settings->customSettingsChanged();
#ifdef _WIN32
    _putenv_s("LANGUAGE", this->settings->getPreferredLocale().c_str());
#else
    setenv("LANGUAGE", this->settings->getPreferredLocale().c_str(), 1);
#endif
}
