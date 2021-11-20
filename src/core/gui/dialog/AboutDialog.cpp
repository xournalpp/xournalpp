#include "AboutDialog.h"

<<<<<<< HEAD:src/core/gui/dialog/AboutDialog.cpp
=======
#include <array>

#include <config.h>
>>>>>>> 7793d150 (WIP: temporary gtk4 - to be splitted or cherry picked):src/gui/dialog/AboutDialog.cpp
#include <gtk/gtk.h>

#include "util/StringUtils.h"
#include "util/i18n.h"

#include "config-git.h"
#include "config.h"

constexpr auto UI_FILE = "about.glade";
constexpr auto UI_DIALOG_NAME = "aboutDialog";
constexpr auto GIT_REPO = "https://github.com/xournalpp/xournalpp";
constexpr auto WEBSITE = "https://xournalpp.github.io";

auto gtk_version() -> std::string_view {
    static std::array<char, 10> version = [] {
        std::array<char, 10> version_int{};
        sprintf(version_int.data(), "%d.%d.%d", gtk_get_major_version(), gtk_get_minor_version(),
                gtk_get_micro_version());
        return version_int;
    }();
    return {version.data(), version.size()};
}

void show_about_dialog(GtkWindow* parent) {
    GtkWidget* dialog;  // Todo (fabian): init


    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    //
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_widget_show(dialog);
}

AboutDialog::AboutDialog(GladeSearchpath* gladeSearchPath): GladeGui(gladeSearchPath, UI_FILE, UI_DIALOG_NAME) {
    gtk_label_set_markup(GTK_LABEL(get("lbBuildDate")), __DATE__ ", " __TIME__);
    gtk_label_set_markup(GTK_LABEL(get("lbVersion")), PROJECT_VERSION);
    gtk_label_set_markup(GTK_LABEL(get("lbRevId")), GIT_COMMIT_ID);
    gtk_label_set_markup(GTK_LABEL(get("lbGtkVersion")), gtk_version().data());
    gtk_link_button_set_uri(GTK_LINK_BUTTON(get("linkRepo")), GIT_REPO);    // Todo (gtk4): append to "vboxRepo"
    gtk_link_button_set_uri(GTK_LINK_BUTTON(get("linkWebsite")), WEBSITE);  // Todo (gtk4): append to "vboxWebsite"

<<<<<<< HEAD:src/core/gui/dialog/AboutDialog.cpp
    char gtkVersion[10];
    sprintf(gtkVersion, "%u.%u.%u", gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version());
=======
    auto* lnkbtn1 = GTK_LINK_BUTTON(get("linkAuthors"));
    gtk_link_button_set_uri(lnkbtn1, "https://raw.githubusercontent.com/xournalpp/xournalpp/master/AUTHORS");
    gtk_button_set_label(GTK_BUTTON(lnkbtn1), _("See the full list of contributors"));
>>>>>>> 7793d150 (WIP: temporary gtk4 - to be splitted or cherry picked):src/gui/dialog/AboutDialog.cpp

    auto* lnkbtn2 = GTK_LINK_BUTTON(get("linkAuthors"));
    gtk_link_button_set_uri(lnkbtn2, "https://raw.githubusercontent.com/xournalpp/xournalpp/master/LICENSE");
    gtk_button_set_label(GTK_BUTTON(lnkbtn2), _("GNU GPLv2 or later"));
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    gtk_widget_show(this->window);
}
