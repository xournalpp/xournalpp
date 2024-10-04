#include "AboutDialog.h"

#include <gtk/gtk.h>

#include "gui/Builder.h"
#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/i18n.h"               // for _

#include "config-git.h"  // for GIT_COMMIT_ID
#include "config.h"      // for PROJECT_VERSION

class GladeSearchpath;

constexpr auto UI_FILE = "about.ui";
constexpr auto UI_DIALOG_NAME = "aboutDialog";
constexpr auto GIT_REPO = "https://github.com/xournalpp/xournalpp";
constexpr auto WEBSITE = "https://xournalpp.github.io";
constexpr auto AUTHORS_LINK = "https://raw.githubusercontent.com/xournalpp/xournalpp/master/AUTHORS";
constexpr auto LICENCE_LINK = "https://raw.githubusercontent.com/xournalpp/xournalpp/master/LICENSE";

static GtkWindow* constructWindow(GladeSearchpath* gladeSearchPath) {
    Builder builder(gladeSearchPath, UI_FILE);
    GtkWindow* window = GTK_WINDOW(builder.get(UI_DIALOG_NAME));

    auto insertPropertyKey = [](GtkGrid* grid, std::string&& str, gint top) {
        auto widget = gtk_label_new(nullptr);
        gtk_label_set_markup(GTK_LABEL(widget), str.insert(0, "<b>").append("</b>").c_str());
        gtk_widget_set_halign(widget, GtkAlign::GTK_ALIGN_START);
        gtk_grid_attach(grid, widget, 0, top, 1, 1);
    };

    auto insertPropertyValue = [](GtkGrid* grid, std::string const& str, gint top) {
        auto widget = gtk_label_new(str.c_str());
        gtk_widget_set_halign(widget, GtkAlign::GTK_ALIGN_START);
        gtk_grid_attach(grid, widget, 1, top, 1, 1);
    };

    auto infoGrid = GTK_GRID(builder.get("versionInfoGrid"));

    insertPropertyKey(infoGrid, _("Version"), 0);
    insertPropertyValue(infoGrid, PROJECT_VERSION, 0);

    insertPropertyKey(infoGrid, _("Built on"), 1);
    insertPropertyValue(infoGrid, __DATE__ ", " __TIME__, 1);

    insertPropertyKey(infoGrid, _("GTK Version"), 2);
    auto gtkVersion =
            FS(FORMAT_STR("{1}.{2}.{3}") % gtk_get_major_version() % gtk_get_minor_version() % gtk_get_micro_version());
    insertPropertyValue(infoGrid, gtkVersion, 2);

    auto const gitCommitId = std::string{GIT_COMMIT_ID};
    if (!gitCommitId.empty()) {
        insertPropertyKey(infoGrid, _("Git commit"), 3);
        insertPropertyValue(infoGrid, gitCommitId.c_str(), 3);
    }

    gtk_box_append(GTK_BOX(builder.get("vboxRepo")), gtk_link_button_new(GIT_REPO));
    gtk_box_append(GTK_BOX(builder.get("vboxWebsite")), gtk_link_button_new(WEBSITE));
    gtk_box_append(GTK_BOX(builder.get("vboxCommunity")),
                   gtk_link_button_new_with_label(AUTHORS_LINK, _("See the full list of contributors")));
    gtk_box_append(GTK_BOX(builder.get("vboxLicense")),
                   gtk_link_button_new_with_label(LICENCE_LINK, _("GNU GPLv2 or later")));

    g_signal_connect_swapped(GTK_BUTTON(builder.get("closebutton")), "clicked", G_CALLBACK(gtk_window_close), window);

    return window;
}

xoj::popup::AboutDialog::AboutDialog(GladeSearchpath* gladeSearchPath): window(constructWindow(gladeSearchPath)) {}

xoj::popup::AboutDialog::~AboutDialog() = default;
